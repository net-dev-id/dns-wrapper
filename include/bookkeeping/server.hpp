/*
 * Created Date: Sunday, September 1st 2024, 4:35:15 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Friday, 6th September 2024 5:46:13 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#pragma once

#include <boost/asio/ip/udp.hpp>
#include <cstdint>

using boost::asio::ip::udp;

struct UpstreamServerInfo {
  udp::endpoint endpoint;
  bool ipv4;
  struct {
    uint32_t queries;
    uint32_t replies;
    uint32_t invalids;
    uint32_t nosources;
    uint32_t timedouts;
    uint32_t failed;
    uint32_t Latency;
    uint32_t MMA;
  } stats;
  time_t forwardtime;
  int forwardcount;

  UpstreamServerInfo *next;
};
