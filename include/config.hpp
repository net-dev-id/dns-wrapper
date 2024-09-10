/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "log.hpp"
#include <cstdint>
#include <regex>
#include <string>
#include <vector>

enum Protocol {
  Tcp,
  Udp,
};

enum IpProtocolVersion { Ipv4, Ipv6 };

struct UpstreamServer {
  std::string hostIp;
  uint16_t port;
  Protocol protocol;
  IpProtocolVersion protocolVersion;
};

struct ConfigReader {
  bool logToConsoleAlso;
  std::string logFile;
  LogLevel logLevel;

#ifdef UNIX
  std::string pidFile;
#endif /* UNIX */

  uint16_t dnsPort;
  uint16_t tcpPort;

  std::vector<UpstreamServer> servers;

  virtual void LoadConfiguration(const std::string &filePath);
  virtual void SaveConfiguration(const std::string &filePath) const;

  static std::regex ipRegex4;
  static std::regex ipRegex6;

private:
  void addServer(const std::string &host, uint16_t port,
                 const std::string &protocol);
};
