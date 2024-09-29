/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <boost/program_options.hpp>
#include <stdexcept>
#include <string>

namespace po = boost::program_options;

class RuleParser;

class Args {
public:
  enum ExitCode { NoExit, ExitWithNoError, ExitWithError, ExitWithUnlock };

private:
  Args() = default;
  ~Args() {};

  static Args *_args;

public:
  static ExitCode Init(int argc, char *argv[]);
  static Args *Get() {
    if (_args == nullptr) {
      throw std::runtime_error("Arguments not initialized");
    }

    return _args;
  }

  static void PrintOptions(
      const std::string &appName, po::options_description &desc,
      std::string subCommand, std::map<std::string, std::string> subCommands,
      boost::program_options::positional_options_description *positionalDesc);

  static void PrintGlobalOptions(
      po::options_description &desc,
      boost::program_options::positional_options_description *positionalDesc);

  static void PrintSubCommandOptions(const std::string &subCommand,
                                     po::options_description &desc);

  std::string configFile;
  bool daemonMode;
  bool useRawSockets;
};
