/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "dns/dnspacket.hpp"
#include <boost/asio/ip/udp.hpp>

using boost::asio::ip::udp;

class RuleMatcher {
public:
  bool IsMatch(const DnsPacket &, const udp::endpoint &);
};
