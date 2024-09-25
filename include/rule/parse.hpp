/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "args.hpp"

#include <boost/program_options.hpp>
#include <string>
#include <vector>

namespace po = boost::program_options;

class RuleParser {
public:
  Args::ExitCode Parse(const po::parsed_options &parsed, po::variables_map &vm);
  bool Parse(const std::vector<std::string> &ruleStrings);
};
