/*
 * Created Date: Monday, August 26th 2024, 5:11:25 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 26th August 2024 6:06:14 pm
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

#include "dns/dnspacket.hpp"
#include <boost/asio/ip/udp.hpp>

using boost::asio::ip::udp;

class RuleMatcher {
public:
  bool IsMatch(const DnsPacket &, const udp::endpoint &);
};
