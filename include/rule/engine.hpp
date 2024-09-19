/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "rule/input.hpp"
#include "rule/rule.hpp"

class RuleEngine {
public:
  bool Evaluate(Input &input) const;

private:
  Rule *rules;
  Action *policyAction;
};
