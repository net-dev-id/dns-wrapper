/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <winsock2.h>

#include <iphlpapi.h>

#include <Windows.h>

#include <string>

#include "common.h"
#include "log.hpp"
#include "net/if.hpp"

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

// https://learn.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh

void Die(const std::string &baseMessage, const int exitCode);
void WinDie(const std::string &baseMessage, const DWORD res,
            const int exitCode);

static IP_ADAPTER_ADDRESSES *getIfAddrs() {
  PIP_ADAPTER_ADDRESSES addrs = nullptr;

  DWORD res = 0;
  unsigned long outBufLen = WORKING_BUFFER_SIZE;
  unsigned long flags = GAA_FLAG_INCLUDE_PREFIX;

  for (int i = 0; i < MAX_TRIES; i++) {
    addrs = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (addrs == nullptr) {
      LERROR << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct";
      exit(EC_NOMEM);
    }

    res = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, addrs, &outBufLen);

    if (res == ERROR_BUFFER_OVERFLOW) {
      free(addrs);
      addrs = nullptr;
    } else {
      break;
    }
  }

  if (res != NO_ERROR) {
    LERROR << "Call to GetAdaptersAddresses failed with error: " << res
           << std::endl;
    if (res == ERROR_NO_DATA) {
      Die("No addresses were found for the requested parameters", EC_MISC);
    } else {
      WinDie("Call to GetAdaptersAddresses failed", res, EC_MISC);
    }
  }

  return addrs;
}

int Interface::Index() const { return ptr->IfIndex; }

std::string Interface::Name() const { return ptr->AdapterName; }

std::string Interface::Type() const {
  if (ptr->Ipv4Enabled) {
    return "AF_INET";
  }
  if (ptr->Ipv6Enabled) {
    return "AF_INET6";
  }

  return "UNKNOWN";
}

bool Interface::HasIpv4() const { return ptr->Ipv4Enabled; }

bool Interface::HasIpv6() const { return ptr->Ipv6Enabled; }

Interface &Interface::operator++() {
  if (ptr) {
    ptr = ptr->Next;
  }
  return *this;
}

NetInterface::NetInterface()
    : addrs(getIfAddrs()), start(addrs), finish(nullptr) {}

NetInterface::~NetInterface() {
  if (addrs) {
    free(addrs);
  }
}
