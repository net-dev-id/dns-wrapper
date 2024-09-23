/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"
#include "rule/input.hpp"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <cstddef>
#include <cstdint>
#include <ostream>

#define DATA_SIZE 8192

enum ActionType { Dns, Redirect };

enum RuleType { IpAddress, EthAddress, IpAndEthAddress };

struct RuleHeader {
  RuleType ruleType;
  ActionType actionType;
};

struct IpAddressData {
  bool ipv4;
  union IpAddress ipaddr;
};

struct Rule {
  RuleHeader header;
  const IpAddressData *ipd;
  const union EthAddress *eth;
  const IpAddressData *target;
};

struct RuleData {
  boost::interprocess::interprocess_mutex mutex;
  struct {
    ActionType action;
    union IpAddress targetIpv4;
    union IpAddress targetIpv6;
  } policy;
  std::size_t size;
  uint8_t data[DATA_SIZE];
};

class ShmRuleEngine {
public:
  ShmRuleEngine(bool mainProcess);
  ~ShmRuleEngine();

  bool AppendRule(const RuleType &ruleType, const ActionType &actionType,
                  const IpAddressData *ipd, const union EthAddress *eth,
                  const IpAddressData *target);
  bool InsertRule(const RuleType &ruleType, const ActionType &actionType,
                  const IpAddressData *ipd, const union EthAddress *eth,
                  const IpAddressData *target, const std::size_t &index);
  bool DeleteRule(const std::size_t &index);
  bool ClearRules();
  bool SetPolicy(const ActionType &actionType,
                 const union IpAddress &targetIpv4,
                 const union IpAddress &targetIpv6);

  bool Evaluate(Input &input) const;

  friend std::ostream &operator<<(std::ostream &ostream,
                                  const ShmRuleEngine &engine);

private:
  Rule nextRule(uint8_t **loc) const;
  std::size_t insertRules(const Rule *rules, const std::size_t &size,
                          const std::size_t &index);
  bool deleteRule(const std::size_t &index);
  bool clearRules();

  bool mainProcess;
  RuleData *ruleData;

  boost::interprocess::shared_memory_object shm;
  boost::interprocess::mapped_region region;
};
