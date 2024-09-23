/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/parse.hpp"
#include "args.hpp"
#include "common.h"
#include "log.hpp"
#include "net/netcommon.h"
#include "rule/common.h"
#include "rule/shm.hpp"
#include "util.hpp"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <climits>
#include <cstddef>
#include <iostream>
#include <string>

#define OPTION_HELP "help"
#define OPTION_COMMAND "command"
#define OPTION_RULES "rules"
#define OPTION_SUBARGS "subargs"

static void printSubCommandOptions(
    po::options_description &desc,
    boost::program_options::positional_options_description *positionalDesc) {
  Args::PrintOptions(
      DAEMON_NAME " " OPTION_RULES, desc, OPTION_COMMAND,
      std::map<std::string, std::string>{
          {OPTION_ADD, "Adds a rule towards the end or at specific position"},
          {OPTION_DELETE, "Deletes a rule"},
          {OPTION_CLEAR, "Clears all rules"},
          {OPTION_LIST, "Lists all rules (default)"},
          {OPTION_POLICY, "Sets the policy to use"},
          {OPTION_LOAD, "Loads rule from a file"},
          {OPTION_SAVE, "Saves rules from a file"}},
      positionalDesc);
}

static bool parseIpAddress(const std::string &ipa, const std::string &optName,
                           union IpAddress &ipaddr) {
  IpProtocolVersion ipv = GetIpAddressType(ipa);
  bool ipv4;

  switch (ipv) {
  case IpProtocolVersion::Ipv4:
    ipv4 = true;
    break;
  case IpProtocolVersion::Ipv6:
    ipv4 = false;
    break;
  default:
    throw po::validation_error(
        po::validation_error::kind_t::invalid_option_value, optName, ipa);
  }

  if (ToIpAddress(ipa, ipv4, &ipaddr) != 1) {
    throw po::validation_error(
        po::validation_error::kind_t::invalid_option_value, optName, ipa);
  }

  return ipv4;
}

static ActionType getActionType(const std::string &action,
                                bool hasTarget = false) {
  if (action == OPTION_DNS) {
    if (hasTarget) {
      LWARNING << "Ignoring target value as action is defined as dns."
               << std::endl;
    }
    return ActionType::Dns;
  } else if (action == OPTION_REDIRECT) {
    return ActionType::Redirect;
  } else {
    throw po::validation_error(
        po::validation_error::kind_t::invalid_option_value, OPTION_ACTION,
        action);
  }
}

Args::ExitCode RuleParser::Parse(const po::parsed_options &parsed,
                                 [[maybe_unused]] po::variables_map &vm) {
  po::options_description rules("rules options");
  rules.add_options()(OPTION_HELP ",h", "produce help message")(
      OPTION_COMMAND, po::value<std::string>(), "command to execute")(
      OPTION_SUBARGS, po::value<std::vector<std::string>>(),
      "Arguments for command");

  po::positional_options_description pos;
  pos.add(OPTION_COMMAND, 1).add(OPTION_SUBARGS, -1);

  std::vector<std::string> opts =
      po::collect_unrecognized(parsed.options, po::include_positional);
  opts.erase(opts.begin());

  po::parsed_options ruleParsed = po::command_line_parser(opts)
                                      .options(rules)
                                      .positional(pos)
                                      .allow_unregistered()
                                      .run();
  po::variables_map vm1;
  po::store(ruleParsed, vm1);

  if (vm1.contains(OPTION_COMMAND)) {
    ShmRuleEngine engine(false);

    std::string cmd = vm1[OPTION_COMMAND].as<std::string>();
    opts = po::collect_unrecognized(ruleParsed.options, po::include_positional);
    opts.erase(opts.begin());
    if (cmd == OPTION_ADD) {
      po::options_description desc("rules add options");
      desc.add_options()(OPTION_HELP ",h", "produce help message")(
          OPTION_INDEX ",n", po::value<std::size_t>(),
          "index of entry to be added (by default add at the end)")(
          OPTION_IP ",i", po::value<std::string>(), "ip address to match")(
          OPTION_MAC ",m", po::value<std::string>(), "mac address to match")(
          OPTION_ACTION ",a", po::value<std::string>(),
          "action to be taken. should be either dns/redirect")(
          OPTION_TARGET ",t", po::value<std::string>(),
          "target IP address for redirect");

      po::parsed_options addParsed = po::command_line_parser(opts)
                                         .options(desc)
                                         .allow_unregistered()
                                         .run();
      po::variables_map vm2;
      po::store(addParsed, vm2);

      if (vm.count(OPTION_HELP)) {
        Args::PrintSubCommandOptions("rules add", desc);
        return Args::ExitWithNoError;
      }

      RuleType ruleType;
      ActionType actionType;
      IpAddressData ipd, target;
      union EthAddress eth;
      bool found = false;
      bool hasTarget = false;
      std::size_t index = SIZE_MAX;

      if (vm2.count(OPTION_INDEX)) {
        index = vm2[OPTION_INDEX].as<std::size_t>();
      }

      if (vm2.count(OPTION_IP)) {
        found = true;
        ruleType = RuleType::IpAddress;
        std::string ipa = vm2[OPTION_IP].as<std::string>();
        ipd.ipv4 = parseIpAddress(ipa, OPTION_IP, ipd.ipaddr);
      }

      if (vm2.count(OPTION_MAC)) {
        if (found) {
          ruleType = RuleType::IpAndEthAddress;
        } else {
          ruleType = RuleType::EthAddress;
        }
        found = true;
        std::string etha = vm2[OPTION_MAC].as<std::string>();
        if (ToEthAddress(etha, &eth) != 1) {
          throw po::validation_error(
              po::validation_error::kind_t::invalid_option_value, OPTION_MAC,
              etha);
        }
      }

      if (!found) {
        throw po::validation_error(
            po::validation_error::kind_t::at_least_one_value_required,
            OPTION_IP " or " OPTION_MAC, "");
      }

      if (vm2.count(OPTION_TARGET)) {
        hasTarget = true;
        std::string ipt = vm2[OPTION_TARGET].as<std::string>();
        target.ipv4 = parseIpAddress(ipt, OPTION_TARGET, target.ipaddr);
      }

      if (vm2.count(OPTION_ACTION)) {
        std::string a = vm2[OPTION_ACTION].as<std::string>();
        actionType = getActionType(a, hasTarget);
      }

      bool res;
      if (index == SIZE_MAX) {
        res = engine.AppendRule(ruleType, actionType, &ipd, &eth, &target);
      } else {
        res =
            engine.InsertRule(ruleType, actionType, &ipd, &eth, &target, index);
      }

      if (!res) {
        LERROR << "Failed to add rule" << std::endl;
        return Args::ExitWithError;
      }

      return Args::ExitWithNoError;
    } else if (cmd == OPTION_DELETE) {
      po::options_description desc("rules delete options");
      desc.add_options()(OPTION_HELP ",h", "produce help message")(
          OPTION_INDEX ",n", po::value<std::size_t>(),
          "index of entry to be added");

      po::parsed_options addParsed = po::command_line_parser(opts)
                                         .options(desc)
                                         .allow_unregistered()
                                         .run();
      po::variables_map vm2;
      po::store(addParsed, vm2);

      if (vm.count(OPTION_HELP)) {
        Args::PrintSubCommandOptions("rules delete", desc);
        return Args::ExitWithNoError;
      }

      if (vm2.count(OPTION_INDEX)) {
        std::size_t index = vm2[OPTION_INDEX].as<std::size_t>();
        if (!engine.DeleteRule(index)) {
          LERROR << "Error deleting rule" << std::endl;
          return Args::ExitWithError;
        }
      } else {
        throw po::validation_error(
            po::validation_error::kind_t::at_least_one_value_required,
            OPTION_INDEX, "");
      }

      return Args::ExitWithNoError;
    } else if (cmd == OPTION_CLEAR || cmd == OPTION_LIST) {
      if (vm.count(OPTION_HELP)) {
        std::cout << "Usage: " << DAEMON_NAME << " rules " << cmd
                  << " -h [--help]" << std::endl;
        return Args::ExitWithNoError;
      }

      if (cmd == OPTION_CLEAR) {
        engine.ClearRules();
      } else {
        std::cout << engine << std::endl;
      }

      return Args::ExitWithNoError;
    } else if (cmd == OPTION_POLICY) {
      po::options_description desc("rules add options");
      desc.add_options()(OPTION_HELP ",h", "produce help message")(
          OPTION_ACTION ",a", po::value<std::string>(),
          "policy action to be taken. should be either dns/redirect")(
          OPTION_TARGET_4 ",4", po::value<std::string>(),
          "target IPv4 address for redirect")(
          OPTION_TARGET_6 ",6", po::value<std::string>(),
          "target IPv6 address for redirect");

      po::parsed_options policyParsed = po::command_line_parser(opts)
                                      .options(desc)
                                      .allow_unregistered()
                                      .run();
      po::variables_map vm2;
      po::store(policyParsed, vm2);

      if (vm.count(OPTION_HELP)) {
        Args::PrintSubCommandOptions("rules add", desc);
        return Args::ExitWithNoError;
      }

      ActionType actionType;
      if (vm2.count(OPTION_ACTION)) {
        std::string a = vm2[OPTION_ACTION].as<std::string>();
        actionType = getActionType(a);
      } else {
        throw po::validation_error(
            po::validation_error::kind_t::at_least_one_value_required,
            OPTION_ACTION, "");
      }

      union IpAddress ipaddr4, ipaddr6;
      if (vm2.count(OPTION_TARGET_4)) {
        std::string ipt = vm2[OPTION_TARGET_4].as<std::string>();
        parseIpAddress(ipt, OPTION_TARGET_4, ipaddr4);
      } else {
        throw po::validation_error(
            po::validation_error::kind_t::at_least_one_value_required,
            OPTION_TARGET_4, "");
      }
      if (vm2.count(OPTION_TARGET_6)) {
        std::string ipt = vm2[OPTION_TARGET_6].as<std::string>();
        parseIpAddress(ipt, OPTION_TARGET_6, ipaddr6);
      } else {
        throw po::validation_error(
            po::validation_error::kind_t::at_least_one_value_required,
            OPTION_TARGET_6, "");
      }

      if (!engine.SetPolicy(actionType, ipaddr4, ipaddr6)) {
        LERROR << "Failed to set policy" << std::endl;
        return Args::ExitWithError;
      }

      return Args::ExitWithNoError;
    } else if (cmd == OPTION_LOAD || cmd == OPTION_SAVE) {
      po::options_description desc("rules " + cmd + " options");
      desc.add_options()(OPTION_HELP ",h", "produce help message")(
          OPTION_FILE ",f", po::value<std::size_t>(),
          "file to load/save rules from");

      po::parsed_options addParsed = po::command_line_parser(opts)
                                         .options(desc)
                                         .allow_unregistered()
                                         .run();
      po::variables_map vm2;
      po::store(ruleParsed, vm2);

      if (vm.count(OPTION_HELP)) {
        Args::PrintSubCommandOptions("rules " + cmd, desc);
        return Args::ExitWithNoError;
      }

      return Args::ExitWithNoError;
    } else {
      std::cerr << "Error unsupported command: " << cmd << std::endl;
      printSubCommandOptions(rules, &pos);
      return Args::ExitWithError;
    }
  }

  if (vm.count(OPTION_HELP)) {
    printSubCommandOptions(rules, &pos);
    return Args::ExitWithNoError;
  }

  // List rules

  return Args::ExitWithNoError;
}

bool RuleParser::Parse(
    [[maybe_unused]] const std::vector<std::string> &ruleStrings) {
  return true;
}
