/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/engine.hpp"

bool RuleEngine::Evaluate(Input &input) const {
  Rule *rule = rules;

  while (rule) {
    auto r = rule->Evaluate(input);
    if (r == Rule::Consumed) {
      return true;
    }

    if (r == Rule::Drop) {
      return false;
    }

    rule = rule->Next();
  }

  if (policyAction) {
    policyAction->Run(input);
    return true;
  }

  return false;
}
