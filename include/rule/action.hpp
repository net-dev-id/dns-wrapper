/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "rule/input.hpp"

class Action {
public:
  virtual void Run(const Input &input) const = 0;
};

class DNSAction : public Action {
public:
  virtual void Run(const Input &input) const;
};

class RedirectAction : public Action {
public:
  virtual void Run(const Input &input) const;
};
