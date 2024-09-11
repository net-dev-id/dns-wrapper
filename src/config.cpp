/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <boost/property_tree/ini_parser.hpp>
#include <cstdint>
#include <iostream>
#include <string>

#include "common.h"
#include "config.hpp"

#define IPV4 "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define IPV6 "[0-9a-eA-E][0-9a-eA-E]?"

namespace pt = boost::property_tree;

std::regex ConfigReader::ipRegex4(IPV4 "." IPV4 "." IPV4 "." IPV4);
std::regex ConfigReader::ipRegex6(IPV6 "(:" IPV6 "){15}");

std::string IniConfigReader::getStringValue(const std::string &key,
                                            const std::string &defValue) {
  return tree.get<std::string>("main." + key, defValue);
}

long IniConfigReader::getLongValue(const std::string &key,
                                   const long &defValue) {
  return tree.get<long>("main." + key, defValue);
}

bool IniConfigReader::getBoolValue(const std::string &key,
                                   const bool &defValue) {
  return tree.get<bool>("main." + key, defValue);
}

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
    std::cerr << "Expected value for server is not a valid ip address: " << host
              << std::endl;
    throw new std::invalid_argument("Invalid server value");
  }

  std::cout << "Upstream server identified as: " << protocol << "://" << host
            << ":" << port << std::endl;
  servers.push_back(
      {host, port, protocol == "udp" ? Protocol::Udp : Protocol::Tcp, ipv});
}

void ConfigReader::LoadConfiguration() {
  logToConsoleAlso = getBoolValue("logToConsoleAlso", true);
  logFile = getStringValue("logFile", LOG_FILE);
  logLevel = Log::ToLogLevel(
      getStringValue("logLevel", Log::FromLogLevel(LogLevel::info)));
  dnsPort = (uint16_t)getLongValue("dnsPort", DNS_PORT);
#ifdef __unix__
  pidFile = getStringValue("pidFile", PID_FILE);
#endif /* __unix__ */

  std::string host = getStringValue("serverIp1", SERVER_IP_1);
  uint16_t port = (uint16_t)getLongValue("serverPort1", DNS_PORT);
  std::string protocol = getStringValue("protocol1", "udp");
  addServer(host, port, protocol);

  host = getStringValue("serverIp2", "");
  if (!host.empty()) {
    port = (uint16_t)getLongValue("serverPort2", DNS_PORT);
    protocol = getStringValue("protocol2", "udp");
    addServer(host, port, protocol);
  }

  host = getStringValue("serverIp3", "");
  if (!host.empty()) {
    port = (uint16_t)getLongValue("serverPort3", DNS_PORT);
    protocol = getStringValue("protocol3", "udp");
    addServer(host, port, protocol);
  }
}

void IniConfigReader::LoadConfiguration() {
  try {
    pt::read_ini(filePath, tree);
  } catch (boost::property_tree::ini_parser_error &e) {
    std::cerr << "Failure parsing configuration file: " << e.what()
              << ". Will continue with defaults." << std::endl;
  }
  ConfigReader::LoadConfiguration();
}
