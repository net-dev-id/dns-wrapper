/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "net/netcommon.h"
#include "util.hpp"
#include <cstddef>

#define MAX_MAC_IP_MAPPINGS 100

class EthMappings {
public:
  struct MacMappingRecord {
    union EthAddress ethaddress;
    bool ipv4;
    union IpAddress ipaddress;
    std::time_t time = GetNow();
  };

  bool Add(const union EthAddress &ethaddress, const bool &ipv4,
           const union IpAddress &ipaddress);
  const MacMappingRecord *LookUp(const union IpAddress &ipaddress,
                                 const bool &ipv4) const;

private:
  std::size_t count = 0;
  MacMappingRecord records[MAX_MAC_IP_MAPPINGS];
};
