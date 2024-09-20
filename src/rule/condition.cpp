/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/condition.hpp"
#include "rule/input.hpp"
#include <algorithm>

bool IpCondition::IsMatch(const Input &input) const {
  return (ipv4 && input.ipaddr.Ipv4 == ipaddr.Ipv4) ||
         (!ipv4 &&
          std::ranges::equal(input.ipaddr.Ipv6, ipaddr.Ipv6));
}

bool MacCondition::IsMatch(const Input &input) const {
  return std::ranges::equal(input.ethaddr.v, b.v);
}
