/*
 * Created Date: Wednesday, August 21st 2024, 6:34:44 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 26th August 2024 11:41:15 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <inotify-cpp/NotifierBuilder.h>
#include <iostream>
#include <string>

#include "common.h"
#include "config.hpp"

#define IPV4 "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define IPV6 "[0-9a-eA-E][0-9a-eA-E]?"

namespace pt = boost::property_tree;

std::regex ConfigReader::ipRegex4(IPV4 "." IPV4 "." IPV4 "." IPV4);
std::regex ConfigReader::ipRegex6(IPV6 "(:" IPV6 "){15}");

void ConfigReader::addServer(const std::string &host, uint16_t port,
                             const std::string &protocol) {
  if (protocol != "udp" && protocol != "tcp") {
    std::cerr << "Expected value for protocol is udp or tcp, provided value: "
              << protocol << std::endl;
    throw new std::invalid_argument("Invalid protocol value");
  }

  IpProtocolVersion ipv;
  if (std::regex_match(host, ipRegex4)) {
    ipv = IpProtocolVersion::Ipv4;
  } else if (std::regex_match(host, ipRegex6)) {
    ipv = IpProtocolVersion::Ipv6;
  } else {
    std::cerr << "Expected value for server is not a valid ip address: "
              << host << std::endl;
    throw new std::invalid_argument("Invalid server value");
  }

  std::cout << "Upstream server identified as: " << protocol << "://" << host << ":" << port << std::endl;
  servers.push_back(
      {host, port, protocol == "udp" ? Protocol::Udp : Protocol::Tcp, ipv});
}

void ConfigReader::LoadConfiguration(const std::string &filePath) {
  pt::ptree tree;
  try {
    pt::read_ini(filePath, tree);
    logToConsoleAlso = tree.get<bool>("main.logToConsoleAlso", false);
    logFile = tree.get<std::string>("main.logFile", LOG_FILE);
    logLevel = Log::ToLogLevel(tree.get<std::string>(
        "main.logLevel", Log::FromLogLevel(LogLevel::info)));
    dnsPort = tree.get<uint16_t>("main.dnsPort", DNS_PORT);
    pidFile = tree.get<std::string>("main.pidFile", PID_FILE);

    std::string host = tree.get<std::string>("main.serverIp1", SERVER_IP_1);
    uint16_t port = tree.get<uint16_t>("main.serverPort1", DNS_PORT);
    std::string protocol = tree.get<std::string>("main.protocol1", "udp");
    addServer(host, port, protocol);

    host = tree.get<std::string>("main.serverIp2", "");
    if (!host.empty()) {
      port = tree.get<uint16_t>("main.serverPort2", DNS_PORT);
      protocol = tree.get<std::string>("main.protocol2", "udp");
      addServer(host, port, protocol);
    }

    host = tree.get<std::string>("main.serverIp3", "");
    if (!host.empty()) {
      port = tree.get<uint16_t>("main.serverPort3", DNS_PORT);
      protocol = tree.get<std::string>("main.protocol3", "udp");
      addServer(host, port, protocol);
    }

  } catch (boost::property_tree::ini_parser_error &e) {
    std::cerr << "Failure parsing configuration file: " << e.what()
              << std::endl;
  }
}

void ConfigReader::SaveConfiguration(const std::string &filePath) {
  pt::ptree tree;
  tree.put("main.logToConsoleAlso", logToConsoleAlso);
  tree.put("main.logFile", logFile);
  tree.put("main.logLevel", Log::FromLogLevel(logLevel));
  tree.put("main.pidFile", pidFile);
  tree.put("main.dnsPort", dnsPort);
  pt::write_ini(filePath, tree);
}
