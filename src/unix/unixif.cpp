/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "net/if.hpp"
#include "unix/unixutil.hpp"

#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
struct ifaddrs {
    struct ifaddrs  *ifa_next;    / * Next item in list * /
    char            *ifa_name;    / * Name of interface * /
    unsigned int     ifa_flags;   / * Flags from SIOCGIFFLAGS * /
    struct sockaddr *ifa_addr;    / * Address of interface * /
    struct sockaddr *ifa_netmask; / * Netmask of interface * /
    union {
        struct sockaddr *ifu_broadaddr;
                        / * Broadcast address of interface * /
        struct sockaddr *ifu_dstaddr;
                        / * Point-to-point destination address * /
    } ifa_ifu;
#define              ifa_broadaddr ifa_ifu.ifu_broadaddr
#define              ifa_dstaddr   ifa_ifu.ifu_dstaddr
    void            *ifa_data;    / * Address-specific data * /
};
*/

static ifaddrs *getIfAddrs() {
  ifaddrs *addrs;
  int res = getifaddrs(&addrs);
  if (res < 0) {
    UnixUtil::Die("Failed to get interface list", EC_MISC);
  }

  return addrs;
}

int Interface::Index() const { return if_nametoindex(ptr->ifa_name); }

std::string Interface::Name() const { return ptr->ifa_name; }

std::string Interface::Type() const {
  switch (ptr->ifa_addr->sa_family) {
  case AF_INET:
    return "AF_INET";
  case AF_INET6:
    return "AF_INET6";
  case AF_PACKET:
    return "AF_PACKET";
  }

  return "UNKNOWN";
}

bool Interface::HasIpv4() const { return ptr->ifa_addr->sa_family == AF_INET; }

bool Interface::HasIpv6() const { return ptr->ifa_addr->sa_family == AF_INET6; }

inline static bool skippableInterface(ifaddrs *ptr) {
  return (0 == std::strcmp(ptr->ifa_name, "lo") ||
          ptr->ifa_addr->sa_family == AF_PACKET);
}

inline static ifaddrs *thisOrNextUseful(ifaddrs *ptr) {
  do {
    if (!skippableInterface(ptr)) {
      return ptr;
    }
  } while ((ptr = ptr->ifa_next));

  return nullptr;
}

Interface &Interface::operator++() {
  while ((ptr = ptr->ifa_next)) {
    if (!skippableInterface(ptr)) {
      break;
    }
  }

  return *this;
}

NetInterface::NetInterface()
    : addrs(getIfAddrs()), start(thisOrNextUseful(addrs)), finish(nullptr) {}

NetInterface::~NetInterface() {
  if (addrs) {
    freeifaddrs(addrs);
  }
}
