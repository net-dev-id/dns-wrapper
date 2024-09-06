/*
 * Created Date: Monday, August 26th 2024, 5:16:12 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 26th August 2024 10:27:04 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "rule/rule.hpp"

bool RuleMatcher::IsMatch(const DnsPacket &, const udp::endpoint &) {
    return false;
}