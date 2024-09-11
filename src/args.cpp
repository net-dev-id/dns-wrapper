/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "args.hpp"
#include "common.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#define OPTION_HELP "help"
#define OPTION_CONFIG_FILE "config-file"
#define OPTION_UNLOCK "unlock"
#define OPTION_DAEMON "daemon"

namespace po = boost::program_options;

Args *Args::_args = nullptr;

Args::ExitCode Args::Init(int argc, char *argv[]) {
  _args = new Args();

  po::options_description desc("Allowed options");
  desc.add_options()(OPTION_HELP, "produce help message")(
      OPTION_CONFIG_FILE, po::value<std::string>(),
      "set configuration file")(OPTION_UNLOCK, "unlock process")(
      OPTION_DAEMON, "run in daemon mode (as background service)");

  try {
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count(OPTION_HELP)) {
      std::cout << desc << std::endl;
      return Args::ExitWithNoError;
    }

    if (vm.count(OPTION_CONFIG_FILE)) {
      _args->configFile = vm[OPTION_CONFIG_FILE].as<std::string>();
    } else {
      _args->configFile = CONFIG_FILE_PATH;
    }

    if (vm.count(OPTION_DAEMON)) {
      _args->daemonMode = true;
    } else {
      _args->daemonMode = false;
    }

    if (vm.count(OPTION_UNLOCK)) {
      std::cout << "Unlocking Process" << std::endl;
      return Args::ExitWithUnlock;
    }
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cout << desc << std::endl;
    return Args::ExitWithError;
  }

  return Args::NoExit;
}
