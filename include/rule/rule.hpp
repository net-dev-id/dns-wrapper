/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "rule/action.hpp"
#include "rule/expression.hpp"
#include <boost/asio/ip/udp.hpp>

class Rule {
public:
  enum Result { Accept, Drop, Consumed };

  Result Evaluate(const Input &input) const;
  Rule *Next() { return next; }

private:
  Expression *expression;
  Action *action;
  Rule *next;
};
