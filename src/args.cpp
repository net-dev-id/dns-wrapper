/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "args.hpp"
#include "common.h"
#include "rule/parse.hpp"
#include "tp/optionprinter.hpp"

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>
#include <string>

#define OPTION_HELP "help"
#define OPTION_CONFIG_FILE "config-file"
#define OPTION_USE_RAW_SOCKETS "use-raw-sockets"
#define OPTION_COMMAND "command"
#define OPTION_SUBARGS "subargs"
#define OPTION_START "start"
#define OPTION_UNLOCK "unlock"
#define OPTION_DAEMON "daemon"
#define OPTION_RULES "rules"

Args *Args::_args = nullptr;

void Args::PrintOptions(
    const std::string &appName, po::options_description &desc,
    std::string subCommand, std::map<std::string, std::string> subCommands,
    boost::program_options::positional_options_description *positionalDesc) {
  OptionPrinter::printStandardAppDesc(appName, std::cout, desc, subCommand,
                                      subCommands, positionalDesc);
}

void Args::PrintGlobalOptions(
    po::options_description &desc,
    boost::program_options::positional_options_description *positionalDesc) {
  PrintOptions(
      DAEMON_NAME, desc, OPTION_COMMAND,
      std::map<std::string, std::string>{
          {OPTION_START, "Starts Service (default)."},
          {OPTION_UNLOCK,
           "Unlock Service. Useful if service stopped unexpectedly."},
          {OPTION_RULES, "Add, delete, insert, clear, load and save rules."}},
      positionalDesc);
}

void Args::PrintSubCommandOptions(const std::string &subCommand,
                                  po::options_description &desc) {
  PrintOptions(DAEMON_NAME " " + subCommand, desc, "",
               std::map<std::string, std::string>(), nullptr);
}

Args::ExitCode Args::Init(int argc, char *argv[]) {
  _args = new Args();

  po::options_description global("Allowed options for " DAEMON_NAME);
  global.add_options()(OPTION_HELP ",h", "produce help message")(
      OPTION_CONFIG_FILE ",c", po::value<std::string>(),
      "set configuration file")(OPTION_COMMAND, po::value<std::string>(),
                                "command to execute")(
      OPTION_SUBARGS, po::value<std::vector<std::string>>(),
      "Arguments for command");

  po::positional_options_description pos;
  pos.add(OPTION_COMMAND, 1).add(OPTION_SUBARGS, -1);

  try {
    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(global)
                                    .positional(pos)
                                    .allow_unregistered()
                                    .run();

    po::store(parsed, vm);

    if (vm.count(OPTION_CONFIG_FILE)) {
      _args->configFile = vm[OPTION_CONFIG_FILE].as<std::string>();
    } else {
      _args->configFile = CONFIG_FILE_PATH;
    }

    if (vm.contains(OPTION_COMMAND)) {
      std::string cmd = vm[OPTION_COMMAND].as<std::string>();
      if (cmd == OPTION_START) {
        po::options_description start("start options");
        start.add_options()(OPTION_HELP ",h", "produce help message")(
            OPTION_DAEMON ",-d", "run in daemon mode (as background service)")(
            OPTION_UNLOCK, "Unlock service before running. Useful if prior run "
                           "of service crashed.")(
            OPTION_USE_RAW_SOCKETS, po::value<bool>(),
            "Enables/disables use of raw sockets. If disable mac address based "
            "rules will not function.");
        std::vector<std::string> opts =
            po::collect_unrecognized(parsed.options, po::include_positional);

        opts.erase(opts.begin());
        po::store(po::command_line_parser(opts).options(start).run(), vm);

        if (vm.count(OPTION_HELP)) {
          PrintSubCommandOptions(OPTION_START, start);
          return Args::ExitWithNoError;
        }

        if (vm.count(OPTION_DAEMON)) {
          _args->daemonMode = true;
        } else {
          _args->daemonMode = false;
        }

        if (vm.count(OPTION_USE_RAW_SOCKETS)) {
          _args->useRawSockets = vm[OPTION_USE_RAW_SOCKETS].as<bool>();
        } else {
#ifdef __linux
          _args->useRawSockets = true;
#else  /* WIN32 */
          _args->useRawSockets = false;
#endif /* __linux */
        }

        if (vm.count(OPTION_UNLOCK)) {
          boost::interprocess::named_mutex::remove(DAEMON_NAME);
        }

      } else if (cmd == OPTION_UNLOCK) {
        po::options_description unlock("unlock options");
        if (vm.count(OPTION_HELP)) {
          PrintSubCommandOptions(OPTION_UNLOCK, unlock);
          return Args::ExitWithNoError;
        }

        std::cout << "Unlocking Process" << std::endl;
        return Args::ExitWithUnlock;
      } else if (cmd == OPTION_RULES) {
        RuleParser ruleParser;
        return ruleParser.Parse(parsed, vm);
      } else {
        std::cerr << "Error unsupported command: " << cmd << std::endl;
        PrintGlobalOptions(global, &pos);
        return Args::ExitWithError;
      }
    }

    if (vm.count(OPTION_HELP)) {
      PrintGlobalOptions(global, &pos);
      return Args::ExitWithNoError;
    }
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    PrintGlobalOptions(global, &pos);
    return Args::ExitWithError;
  }

  return Args::NoExit;
}
