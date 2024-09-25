/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"
#include <array>
#include <cstddef>
#include <cstdint>

enum IpProtocolVersion { Ipv4, Ipv6, none };

template <class T> struct RegexParameter {
public:
  RegexParameter(T v) : v(v) {}
  T v;
};

char *GetCurrentUserName(void);
void DumpHex(std::array<uint8_t, MAX_PACKET_SZ>, const std::size_t n);
std::time_t GetNow();
IpProtocolVersion GetIpAddressType(const std::string &host);
int ToEthAddress(const std::string &ethaddr, union EthAddress *address);
int ToIpAddress(const std::string &ipaddr, const bool &ipv4,
                union IpAddress *address);
int IpAddressToString(const union IpAddress &ipaddr, const bool &ipv4,
                      std::string &address);
std::string EthAddressToString(const union EthAddress &ethaddr);
