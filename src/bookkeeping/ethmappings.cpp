/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bookkeeping/ethmappings.hpp"
#include "net/netcommon.h"
#include "util.hpp"
#include <algorithm>
#include <cstddef>
#include <ctime>

#define TIMEOUT 5

bool EthMappings::Add(const union EthAddress &ethaddress, const bool &ipv4,
                      const union IpAddress &ipaddress) {
  MacMappingRecord *oldest = nullptr;
  MacMappingRecord *target = nullptr;
  auto now = GetNow();

  for (std::size_t i = 0; i < count; i++) {
    auto &r = records[i];
    if (std::ranges::equal(r.ethaddress.v, ethaddress.v)) {
      target = &r;
      break;
    }

    if (!oldest || oldest->time < r.time) {
      oldest = &r;
    }
  }

  if (!target && count < MAX_MAC_IP_MAPPINGS) {
    target = &records[count++];
  }

  if (!target && oldest && std::difftime(now, oldest->time) > TIMEOUT) {
    target = oldest;
  }

  if (target) {
    target->ipv4 = ipv4;
    std::ranges::copy(ethaddress.v, target->ethaddress.v);
    if (ipv4) {
      target->ipaddress.Ipv4 = ipaddress.Ipv4;
    } else {
      std::ranges::copy(ipaddress.Ipv6v, target->ipaddress.Ipv6v);
    }
    target->time = now;
    return true;
  }

  return false;
}

const EthMappings::MacMappingRecord *
EthMappings::LookUp(const union IpAddress &ipaddress, const bool &ipv4) const {
  for (std::size_t i = 0; i < count; i++) {
    auto &r = records[i];
    if (r.ipv4 == ipv4 &&
        ((ipv4 && ipaddress.Ipv4 == r.ipaddress.Ipv4) ||
         (!ipv4 && std::ranges::equal(ipaddress.Ipv6, r.ipaddress.Ipv6)))) {
      return &r;
    }
  }

  return nullptr;
}
