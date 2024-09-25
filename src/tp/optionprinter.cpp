/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "tp/optionprinter.hpp"

#include <boost/algorithm/string/regex.hpp>

void OptionPrinter::addOption(const CustomOptionDescription &optionDesc) {
  optionDesc.isPositional ? positionalOptions.push_back(optionDesc)
                          : options.push_back(optionDesc);
}

std::string OptionPrinter::usage() {
  std::stringstream usageDesc;
  bool firstShortOption = true;
  usageDesc << "[";
  for (std::vector<CustomOptionDescription>::iterator it = options.begin();
       it != options.end(); ++it) {
    if (it->hasShort && !it->hasArgument && !it->required) {
      if (firstShortOption) {
        usageDesc << "-";
        firstShortOption = false;
      }

      usageDesc << it->optionDisplayName[1];
    }
  }

  usageDesc << "] ";

  for (std::vector<CustomOptionDescription>::iterator it = options.begin();
       it != options.end(); ++it) {
    if (!it->hasShort && !it->hasArgument && !it->required) {
      usageDesc << "[" << it->optionDisplayName << "] ";
    }
  }

  for (std::vector<CustomOptionDescription>::iterator it = options.begin();
       it != options.end(); ++it) {
    if (it->hasArgument && !it->required) {
      usageDesc << "[" << it->optionDisplayName << " ARG] ";
    }
  }

  for (std::vector<CustomOptionDescription>::iterator it = options.begin();
       it != options.end(); ++it) {
    if (it->hasArgument && it->required) {
      usageDesc << it->optionDisplayName << " ARG ";
    }
  }

  for (std::vector<CustomOptionDescription>::iterator it =
           positionalOptions.begin();
       it != positionalOptions.end(); ++it) {
    usageDesc << it->optionDisplayName << " ";
  }

  return usageDesc.str();
}

std::string OptionPrinter::positionalOptionDetails(
    const std::string &subCommand,
    std::map<std::string, std::string> &subCommands) {
  std::stringstream output;
  for (std::vector<CustomOptionDescription>::iterator it =
           positionalOptions.begin();
       it != positionalOptions.end(); ++it) {
    output << it->getPositionalOptionUsageString(subCommand, subCommands);
  }

  return output.str();
}

std::string OptionPrinter::optionDetails() {
  std::stringstream output;
  for (std::vector<CustomOptionDescription>::iterator it = options.begin();
       it != options.end(); ++it) {
    output << it->getOptionUsageString() << std::endl;
  }

  return output.str();
}

void OptionPrinter::printStandardAppDesc(
    const std::string &appName, std::ostream &out,
    boost::program_options::options_description desc,
    const std::string &subCommand,
    std::map<std::string, std::string> &subCommands,
    boost::program_options::positional_options_description *positionalDesc) {
  OptionPrinter optionPrinter;

  typedef std::vector<
      boost::shared_ptr<boost::program_options::option_description>>
      Options;
  Options allOptions = desc.options();
  for (Options::iterator it = allOptions.begin(); it != allOptions.end();
       ++it) {
    CustomOptionDescription currOption(*it);
    if (positionalDesc) {
      currOption.checkIfPositional(*positionalDesc);
    }

    optionPrinter.addOption(currOption);
  }

  out << "Usage: " << appName << " " << optionPrinter.usage() << std::endl
      << std::endl
      << "-- Option Descriptions --" << std::endl
      << std::endl;

  if (optionPrinter.hasPositionalOptions()) {
    out << "Commands:" << std::endl
        << optionPrinter.positionalOptionDetails(subCommand, subCommands)
        << std::endl;
  }

  if (optionPrinter.hasOptions()) {
    out << "Option Arguments:" << std::endl
        << optionPrinter.optionDetails() << std::endl;
  }
}

void OptionPrinter::formatRequiredOptionError(
    boost::program_options::required_option &error) {
  std::string currOptionName = error.get_option_name();
  boost::algorithm::erase_regex(currOptionName, boost::regex("^-+"));
  error.set_option_name(currOptionName);
}
