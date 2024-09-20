/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "net/netcommon.h"
#include "util.hpp"

#define MAX_MAC_IP_MAPPINGS 30

class EthMappings {
public:
  struct MacMappingRecord {
    EthAddress ethaddress;
    bool ipv4;
    IpAddress ipaddress;
    std::time_t time = GetNow();
  };

  bool Add(const EthAddress &ethaddress, const bool &ipv4,
           const IpAddress &ipaddress);
  const MacMappingRecord *LookUp(const IpAddress &ipaddress, const bool &ipv4) const;

private:
  MacMappingRecord records[MAX_MAC_IP_MAPPINGS];
};
