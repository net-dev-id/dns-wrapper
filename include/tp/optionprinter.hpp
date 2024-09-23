/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "tp/customoptiondescription.hpp"
#include <map>

class OptionPrinter {
public:
  void addOption(const CustomOptionDescription &optionDesc);

  std::string usage();

  std::string
  positionalOptionDetails(const std::string &subCommand,
                          std::map<std::string, std::string> &subCommands);
  std::string optionDetails();
  bool hasOptions() { return !options.empty(); };
  bool hasPositionalOptions() { return !positionalOptions.empty(); }

public:
  static void printStandardAppDesc(
      const std::string &appName, std::ostream &out,
      boost::program_options::options_description desc,
      const std::string &subCommand,
      std::map<std::string, std::string> &subCommands,
      boost::program_options::positional_options_description *positionalDesc =
          0);
  static void
  formatRequiredOptionError(boost::program_options::required_option &error);

private:
  std::vector<CustomOptionDescription> options;
  std::vector<CustomOptionDescription> positionalOptions;
};
