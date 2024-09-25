/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <boost/program_options.hpp>

#include <string>

class CustomOptionDescription {
public:
  CustomOptionDescription(
      boost::shared_ptr<boost::program_options::option_description> option);

  void
  checkIfPositional(const boost::program_options::positional_options_description
                        &positionalDesc);

  std::string getOptionUsageString();
  std::string getPositionalOptionUsageString(
      const std::string &subCommand,
      std::map<std::string, std::string> &subCommands);

public:
  std::string optionID;
  std::string optionDisplayName;
  std::string optionDescription;
  std::string optionFormatName;

  bool required;
  bool hasShort;
  bool hasArgument;
  bool isPositional;
};