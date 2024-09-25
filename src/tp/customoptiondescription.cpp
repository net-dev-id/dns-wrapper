/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "tp/customoptiondescription.hpp"

#include <boost/algorithm/string/erase.hpp>
#include <iomanip>
#include <ios>
#include <string>

namespace {
const size_t LONG_NON_PREPENDED_IF_EXIST_ELSE_PREPENDED_SHORT = 0;
const size_t LONG_PREPENDED_IF_EXIST_ELSE_PREPENDED_SHORT = 1;
const size_t SHORT_PREPENDED_IF_EXIST_ELSE_LONG = 4;

const size_t SHORT_OPTION_STRING_LENGTH = 2; // -x
const size_t ADEQUATE_WIDTH_FOR_OPTION_NAME = 20;

const bool HAS_ARGUMENT = true;
const bool DOES_NOT_HAVE_ARGUMENT = false;
} // namespace

CustomOptionDescription::CustomOptionDescription(
    boost::shared_ptr<boost::program_options::option_description> option)
    : required(false), hasShort(false), hasArgument(false),
      isPositional(false) {
  if ((option->canonical_display_name(SHORT_PREPENDED_IF_EXIST_ELSE_LONG)
           .size() == SHORT_OPTION_STRING_LENGTH)) {
    hasShort = true;
    optionID =
        option->canonical_display_name(SHORT_PREPENDED_IF_EXIST_ELSE_LONG);
    optionDisplayName =
        option->canonical_display_name(SHORT_PREPENDED_IF_EXIST_ELSE_LONG);
  } else {
    hasShort = false;
    optionID = option->canonical_display_name(
        LONG_NON_PREPENDED_IF_EXIST_ELSE_PREPENDED_SHORT);
    optionDisplayName = option->canonical_display_name(
        LONG_PREPENDED_IF_EXIST_ELSE_PREPENDED_SHORT);
  }

  boost::shared_ptr<const boost::program_options::value_semantic> semantic =
      option->semantic();
  required = semantic->is_required();
  hasArgument =
      semantic->max_tokens() > 0 ? HAS_ARGUMENT : DOES_NOT_HAVE_ARGUMENT;

  optionDescription = option->description();
  optionFormatName = option->format_name();
}

#include <iostream>

void CustomOptionDescription::checkIfPositional(
    const boost::program_options::positional_options_description
        &positionalDesc) {
  std::string last = "";
  for (unsigned i = 0; i < positionalDesc.max_total_count(); ++i) {
    std::string current = positionalDesc.name_for_position(i);
    if (optionID == current) {
      boost::algorithm::erase_all(optionDisplayName, "-");
      isPositional = true;
      break;
    }

    if (positionalDesc.max_total_count() > 1000 && current == last) {
      return;
    }

    last = current;
  }
}

std::string CustomOptionDescription::getOptionUsageString() {
  std::stringstream usageString;
  if (isPositional) {
    usageString << "\t" << std::setw(ADEQUATE_WIDTH_FOR_OPTION_NAME)
                << std::left << optionDisplayName << "\t" << optionDescription;
  } else {
    usageString << "\t" << std::setw(ADEQUATE_WIDTH_FOR_OPTION_NAME)
                << std::left << optionFormatName << "\t" << optionDescription;
  }

  return usageString.str();
}

std::string CustomOptionDescription::getPositionalOptionUsageString(
    const std::string &subCommand,
    std::map<std::string, std::string> &subCommands) {
  std::stringstream usageString;

  if (optionID == subCommand) {
    usageString << "\t" << "A command can be one of the following (each with its own set of options):" << std::endl;
    for (auto it = subCommands.begin(); it != subCommands.end(); it++) {
      usageString << "\t" << std::setw(ADEQUATE_WIDTH_FOR_OPTION_NAME)
                  << std::left << it->first << "\t" << it->second << std::endl;
    }
  } else if (optionID != "subargs") {
    usageString << "\t" << std::setw(ADEQUATE_WIDTH_FOR_OPTION_NAME)
                << std::left << optionDisplayName << "\t" << optionDescription
                << std::endl;
  }

  return usageString.str();
}
