/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "rule/condition.hpp"

class BaseExpression {
public:
  enum Operator { AND, OR };
  
  virtual bool Evaluate(const Input &input) const = 0;
};

class SingleExpression : public BaseExpression {
public:
  virtual bool Evaluate(const Input &input) const;

private:
  Condition *condition;
};

class Expression : public BaseExpression {
public:
  virtual bool Evaluate(const Input &input) const;

private:
  SingleExpression *expression1;
  Operator op;
  SingleExpression *expression2;
};
