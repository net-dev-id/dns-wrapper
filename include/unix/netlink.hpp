#pragma once

#include <bits/types/struct_iovec.h>
#include <cstdint>

class NetLink {
public:
  NetLink();
  ~NetLink();

private:
  int netlinkfd;
  uint32_t netlink_pid;
  struct iovec iov;
};
