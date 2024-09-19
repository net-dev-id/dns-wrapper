/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"
#include <boost/asio/ip/udp.hpp>
#include <cstdint>

using boost::asio::ip::udp;

struct UpstreamServerInfo {
  udp::endpoint endpoint;
  bool ipv4;
  IpAddress address;
  Port port;
  struct {
    uint32_t queries;
    uint32_t replies;
    uint32_t invalids;
    uint32_t nosources;
    uint32_t timedouts;
    uint32_t failed;
    std::time_t Latency;
    std::time_t MMA;
  } stats;
  time_t forwardtime;
  int forwardcount;

  UpstreamServerInfo *next;
};
