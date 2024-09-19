/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"
#include "rule/input.hpp"

class Condition {
public:
  virtual bool IsMatch(const Input &input) const = 0;
};

class IpCondition : public Condition {
public:
  virtual bool IsMatch(const Input &input) const;

private:
  bool ipv4;
  IpAddress ipaddr;
};

class MacCondition : public Condition {
public:
  virtual bool IsMatch(const Input &input) const;

private:
  EthAddress b;
};
