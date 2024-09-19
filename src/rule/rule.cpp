/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/rule.hpp"

Rule::Result Rule::Evaluate(const Input &input) const {
  if (expression->Evaluate(input)) {
    action->Run(input);
    return Consumed;
  }

  return Accept;
}
