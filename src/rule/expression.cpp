/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/expression.hpp"

bool SingleExpression::Evaluate(const Input &input) const {
  return condition->IsMatch(input);
}

bool Expression::Evaluate(const Input &input) const {
  bool r1 = expression1->Evaluate(input);
  if (op == AND && !r1) {
    return false;
  }

  if (op == OR && r1) {
    return true;
  }

  return expression2->Evaluate(input);
}
