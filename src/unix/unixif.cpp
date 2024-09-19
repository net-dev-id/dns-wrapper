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

#include <ifaddrs.h>
#include <net/if.h>
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

template <> Interface<ifaddrs>::~Interface() {
  if (start) {
    freeifaddrs(start);
  }
}

template <> int Interface<ifaddrs>::Index() const {
  return if_nametoindex(ptr->ifa_name);
}

template <> std::string Interface<ifaddrs>::Name() const {
  return ptr->ifa_name;
}

template <> Interface<ifaddrs> &Interface<ifaddrs>::operator++() {
  if (ptr) {
    ptr = ptr->ifa_next;
  }
  return *this;
}

template <> Interface<ifaddrs> Interface<ifaddrs>::operator++(int) {
  if (ptr) {
    ptr = ptr->ifa_next;
  }
  return *this;
}

template <>
NetInterface<ifaddrs>::NetInterface() : start(getIfAddrs()), finish(nullptr) {}
