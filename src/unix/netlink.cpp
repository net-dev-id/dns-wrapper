#include "unix/netlink.hpp"
#include "common.h"
#include "unix/unixutil.hpp"

#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/types.h>
#include <sys/socket.h>

NetLink::NetLink() {
  struct sockaddr_nl addr;
  socklen_t slen = sizeof(addr);

  addr.nl_family = AF_NETLINK;
  addr.nl_pad = 0;
  addr.nl_pid = 0; /* autobind */
  addr.nl_groups = RTMGRP_IPV4_ROUTE;
  addr.nl_groups |= RTMGRP_IPV4_IFADDR;
  addr.nl_groups |= RTMGRP_IPV6_ROUTE;
  addr.nl_groups |= RTMGRP_IPV6_IFADDR;

  /* May not be able to have permission to set multicast groups don't die in
   * that case */
  if ((netlinkfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) != -1) {
    if (bind(netlinkfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      addr.nl_groups = 0;
      if (errno != EPERM ||
          bind(netlinkfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        netlinkfd = -1;
    }
  }

  if (netlinkfd == -1 ||
      getsockname(netlinkfd, (struct sockaddr *)&addr, &slen) == -1)
    UnixUtil::Die("Cannot create netlink socket", EC_MISC);

  /* save pid assigned by bind() and retrieved by getsockname() */
  netlink_pid = addr.nl_pid;

  iov.iov_len = 100;
  iov.iov_base = new char[iov.iov_len]();
}

NetLink::~NetLink() {}
