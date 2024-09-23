/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

#include "dns/server.hpp"
#include "log.hpp"
#include "net/netcommon.h"
#include "rule/common.h"
#include "rule/input.hpp"
#include "rule/shm.hpp"

#include <algorithm>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/lock_types.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>

#define SHM_NAME "shm-" DAEMON_NAME

using namespace boost::interprocess;

ShmRuleEngine::ShmRuleEngine(bool mainProcess) : mainProcess(mainProcess) {
  if (mainProcess) {
    shared_memory_object::remove(SHM_NAME);

    shm = shared_memory_object(create_only, SHM_NAME, read_write);
    shm.truncate(sizeof(RuleData));
  } else {
    shm = shared_memory_object(open_only, SHM_NAME, read_write);
  }

  region = mapped_region(shm, read_write);
  void *addr = region.get_address();

  if (mainProcess) {
    // std::memset(addr, 0, sizeof(RuleData));
    ruleData = new (addr) RuleData;
  } else {
    ruleData = static_cast<RuleData *>(addr);
  }
}

ShmRuleEngine::~ShmRuleEngine() {
  if (mainProcess) {
    shared_memory_object::remove(SHM_NAME);
  }
}

bool ShmRuleEngine::Evaluate(Input &input) const {
  scoped_lock<interprocess_mutex> lock(ruleData->mutex);

  uint8_t *t = ruleData->data;
  for (std::size_t i = 0; i < ruleData->size; i++) {
    Rule r = nextRule(&t);
    if (r.header.ruleType == RuleType::IpAddress ||
        r.header.ruleType == RuleType::IpAndEthAddress) {
      if (r.ipd->ipv4 != input.ipv4) {
        continue;
      }

      if ((r.ipd->ipv4 && r.ipd->ipaddr.Ipv4 != input.ipaddr.Ipv4) ||
          (!r.ipd->ipv4 &&
           !std::ranges::equal(r.ipd->ipaddr.Ipv6, input.ipaddr.Ipv6))) {
        continue;
      }
    }

    if (r.header.ruleType == RuleType::EthAddress ||
        r.header.ruleType == RuleType::IpAndEthAddress) {
      if (!std::ranges::equal(r.eth->v, input.ethaddr.v)) {
        continue;
      }
    }

    // The target IP (v4 or v6) should match that required in query
    if (r.header.actionType == ActionType::Redirect &&
        input.ipv4 != r.target->ipv4) {
      continue;
    }

    // At this point we have matched rules. So we need to take action.

    if (r.header.actionType == ActionType::Dns) {
      input.server->Resolve(input.packet, input.endpoint, input.ipv4);
    } else {
      input.server->Redirect(input.packet, input.endpoint, input.ipv4,
                             r.target->ipaddr);
    }
  }

  // No match with any so we will next apply policy

  if (ruleData->policy.action == ActionType::Dns) {
    input.server->Resolve(input.packet, input.endpoint, input.ipv4);
  } else {
    if (input.ipv4) {
      input.server->Redirect(input.packet, input.endpoint, input.ipv4,
                             ruleData->policy.targetIpv4);
    } else {
      input.server->Redirect(input.packet, input.endpoint, input.ipv4,
                             ruleData->policy.targetIpv6);
    }
  }

  return true;
}

bool ShmRuleEngine::AppendRule(const RuleType &ruleType,
                               const ActionType &actionType,
                               const IpAddressData *ipd,
                               const union EthAddress *eth,
                               const IpAddressData *target) {
  // This function is not using InsertRule as mutex needs
  // to be locked before reading ruleData->size.

  scoped_lock<interprocess_mutex> lock(ruleData->mutex);
  Rule rule{{ruleType, actionType}, ipd, eth, target};
  std::size_t aIndex = ruleData->size;
  return insertRules(&rule, 1, aIndex);
}

bool ShmRuleEngine::InsertRule(const RuleType &ruleType,
                               const ActionType &actionType,
                               const IpAddressData *ipd,
                               const union EthAddress *eth,
                               const IpAddressData *target,
                               const std::size_t &index) {
  scoped_lock<interprocess_mutex> lock(ruleData->mutex);
  Rule rule{{ruleType, actionType}, ipd, eth, target};
  return insertRules(&rule, 1, index);
}

bool ShmRuleEngine::DeleteRule(const std::size_t &index) {
  scoped_lock<interprocess_mutex> lock(ruleData->mutex);
  return deleteRule(index);
}

bool ShmRuleEngine::ClearRules() {
  scoped_lock<interprocess_mutex> lock(ruleData->mutex);
  return clearRules();
}

bool ShmRuleEngine::SetPolicy(const ActionType &actionType,
                              const union IpAddress &targetIpv4,
                              const union IpAddress &targetIpv6) {
  scoped_lock<interprocess_mutex> lock(ruleData->mutex);
  ruleData->policy.action = actionType;
  ruleData->policy.targetIpv4 = targetIpv4;
  ruleData->policy.targetIpv6 = targetIpv6;
  return true;
}

Rule ShmRuleEngine::nextRule(uint8_t **loc) const {

  RuleHeader *rh = reinterpret_cast<RuleHeader *>(*loc);
  *loc += sizeof(RuleHeader);

  IpAddressData *ipd = nullptr;
  union EthAddress *eth = nullptr;

  switch (rh->ruleType) {
  case RuleType::IpAddress:
    ipd = reinterpret_cast<IpAddressData *>(*loc);
    *loc += sizeof(IpAddressData);
    break;
  case RuleType::EthAddress:
    eth = reinterpret_cast<union EthAddress *>(*loc);
    *loc += sizeof(EthAddress);
    break;
  case RuleType::IpAndEthAddress:
    ipd = reinterpret_cast<IpAddressData *>(*loc);
    *loc += sizeof(IpAddressData);
    eth = reinterpret_cast<union EthAddress *>(*loc);
    *loc += sizeof(EthAddress);
    break;
  }

  IpAddressData *target = nullptr;
  if (rh->actionType == ActionType::Redirect) {
    target = reinterpret_cast<IpAddressData *>(*loc);
    *loc += sizeof(IpAddressData);
  }

  return {{rh->ruleType, rh->actionType}, ipd, eth, target};
}

std::size_t ShmRuleEngine::insertRules(const Rule *rules,
                                       const std::size_t &size,
                                       const std::size_t &index) {
  if (index > ruleData->size) {
    LERROR << "Specified index: " << index
           << " is greater than list size: " << ruleData->size << std::endl;
    return 0;
  }

  // First calculate size of data to be inserted
  std::size_t i = 0;

  std::size_t bytes = 0;
  for (i = 0; i < size; i++) {
    bytes += sizeof(RuleHeader);
    if (rules[i].header.ruleType == RuleType::IpAddress ||
        rules[i].header.ruleType == RuleType::IpAndEthAddress) {
      bytes += sizeof(IpAddressData);
    }

    if (rules[i].header.ruleType == RuleType::EthAddress ||
        rules[i].header.ruleType == RuleType::IpAndEthAddress) {
      bytes += sizeof(EthAddress);
    }

    if (rules[i].header.actionType == ActionType::Redirect) {
      bytes += sizeof(IpAddressData);
    }
  }

  // Skip index items
  uint8_t *t = ruleData->data;
  for (i = 0; i < index; i++) {
    nextRule(&t);
  }

  uint8_t *insertionPoint = t;

  // Go to the end
  for (; i < ruleData->size; i++) {
    nextRule(&t);
  }

  uint8_t *end = t;

  // Do not write past available data
  if (end + bytes > ruleData->data + DATA_SIZE) {
    LERROR << "Shm data is full rejecting insertion request" << std::endl;
    return 0;
  }

  // Copy existing data to new location
  for (t = end + bytes; t > insertionPoint + bytes; t--) {
    *t = *(t - bytes);
  }

  // Now insert new rules
  t = insertionPoint;
  for (i = 0; i < size; i++) {
    std::memcpy(t, &rules[i].header, sizeof(RuleHeader));
    t += sizeof(RuleHeader);

    if (rules[i].header.ruleType == RuleType::IpAddress ||
        rules[i].header.ruleType == RuleType::IpAndEthAddress) {
      std::memcpy(t, rules[i].ipd, sizeof(IpAddressData));
      t += sizeof(IpAddressData);
    }

    if (rules[i].header.ruleType == RuleType::EthAddress ||
        rules[i].header.ruleType == RuleType::IpAndEthAddress) {
      std::memcpy(t, rules[i].eth, sizeof(EthAddress));
      t += sizeof(EthAddress);
    }

    if (rules[i].header.actionType == ActionType::Redirect) {
      std::memcpy(t, rules[i].target, sizeof(IpAddressData));
      t += sizeof(IpAddressData);
    }
  }

  ruleData->size += size;

  return size;
}

bool ShmRuleEngine::deleteRule(const std::size_t &index) {
  if (ruleData->size == 0 || index >= ruleData->size) {
    return false;
  }

  // If last item is to be deleted
  if (index == ruleData->size - 1) {
    ruleData->size--;
    return true;
  }

  std::size_t i = 0;

  // Skip index - 1 items
  uint8_t *t = ruleData->data;
  for (i = 0; i < index; i++) {
    nextRule(&t);
  }

  uint8_t *deletionPoint = t;
  nextRule(&t);
  i++;
  std::size_t bytes = t - deletionPoint;

  // Go to the end
  for (; i < ruleData->size; i++) {
    nextRule(&t);
  }

  uint8_t *end = t;

  for (t = deletionPoint; t + bytes < end; t++) {
    *t = *(t + bytes);
  }

  ruleData->size--;
  return true;
}

bool ShmRuleEngine::clearRules() {
  ruleData->size = 0;
  return true;
}

static inline std::ostream &showParam(std::ostream &ostream, const char *what,
                                      bool spaced = false) {
  if (spaced) {
    ostream << " ";
  }
  ostream << "--" << what << " ";
  return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const ShmRuleEngine &engine) {
  ostream << "start" << std::endl;

  uint8_t *t = engine.ruleData->data;
  for (std::size_t i = 0; i < engine.ruleData->size; i++) {
    Rule r = engine.nextRule(&t);

    ostream << OPTION_ADD << " ";

    bool space = false;
    if (r.header.ruleType == RuleType::IpAddress ||
        r.header.ruleType == RuleType::IpAndEthAddress) {
      std::string address;
      space = true;
      IpAddressToString(r.ipd->ipaddr, r.ipd->ipv4, address);
      showParam(ostream, OPTION_IP) << address;
    }

    if (r.header.ruleType == RuleType::EthAddress ||
        r.header.ruleType == RuleType::IpAndEthAddress) {

      showParam(ostream, OPTION_MAC, space)
          << r.eth->v[0] << ":" << r.eth->v[1] << ":" << r.eth->v[2] << ":"
          << r.eth->v[3] << ":" << r.eth->v[4] << ":" << r.eth->v[5];
    }

    showParam(ostream, OPTION_ACTION, true)
        << (r.header.actionType == ActionType::Dns ? "dns" : "redirect");

    if (r.header.actionType == ActionType::Redirect) {
      std::string address;
      IpAddressToString(r.target->ipaddr, r.target->ipv4, address);
      showParam(ostream, OPTION_TARGET, true) << address;
    }

    ostream << " # " << i << std::endl;
  }

  ostream << OPTION_POLICY << " "
          << (engine.ruleData->policy.action == ActionType::Dns ? "dns"
                                                                : "redirect");

  if (engine.ruleData->policy.action == ActionType::Redirect) {
    std::string address;
    IpAddressToString(engine.ruleData->policy.targetIpv4, true, address);
    showParam(ostream, OPTION_TARGET_4, true) << address;

    IpAddressToString(engine.ruleData->policy.targetIpv6, false, address);
    showParam(ostream, OPTION_TARGET_6, true) << address;
  }

  ostream << std::endl;
  ostream << "end" << std::endl;

  return ostream;
}
