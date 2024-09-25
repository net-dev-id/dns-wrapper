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

using boost::asio::ip::udp;

class DnsPacket;
class DnsServer;

struct Input {
  DnsPacket *packet;
  DnsServer *server;
  const udp::endpoint *endpoint;
  const bool ipv4;
  const EthAddress &ethaddr;
  const IpAddress &ipaddr;
};
