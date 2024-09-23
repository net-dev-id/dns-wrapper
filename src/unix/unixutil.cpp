/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "unix/unixutil.hpp"
#include "log.hpp"
#include "net/netcommon.h"
#include <arpa/inet.h>
#include <pwd.h>
#include <sys/socket.h>

void UnixUtil::Die(const std::string &baseMessage, const int exitCode) {
  char *errMesssage = strerror(errno);

  LFATAL << baseMessage << ": " << errMesssage << std::endl;
  exit(exitCode);
}

char *GetUserName(void) {
  char *name;
  const uid_t euid = geteuid();
  const struct passwd *pw = getpwuid(euid);
  if (pw) {
    name = strdup(pw->pw_name);
  } else {
    if (asprintf(&name, "%u", euid) < 0)
      return NULL;
  }

  return name;
}

int ToIpAddress(const std::string &ipaddr, const bool &ipv4,
                union IpAddress *address) {
  int af;
  if (ipv4) {
    af = AF_INET;
  } else {
    af = AF_INET6;
  }

  int r = inet_pton(af, ipaddr.c_str(), address);
  if (r == 0) {
    LERROR << "ipaddr does not contain a character string representing a valid "
              "network address"
           << std::endl;
  } else if (r < 1) {
    LERROR << "Error converting ip address to string: " << strerror(errno)
           << std::endl;
  }

  return r;
}

int IpAddressToString(const union IpAddress &ipaddr, const bool &ipv4,
                      std::string &address) {
  static char buf[INET6_ADDRSTRLEN + 1];
  int af;
  if (ipv4) {
    af = AF_INET;
  } else {
    af = AF_INET6;
  }

  const char *t = inet_ntop(af, &ipaddr, buf, INET6_ADDRSTRLEN + 1);
  if (t == nullptr) {
    LERROR << "Error converting ip address to string: " << strerror(errno)
           << std::endl;
    return 0;
  }

  address = buf;
  return strlen(t);
}
