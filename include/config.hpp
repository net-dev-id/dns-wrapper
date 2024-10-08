/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "log.hpp"
#include "util.hpp"
#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <string>
#include <vector>

enum Protocol {
  Tcp,
  Udp,
};

struct UpstreamServer {
  std::string hostIp;
  uint16_t port;
  Protocol protocol;
  IpProtocolVersion protocolVersion;
};

class ConfigReader {
public:
  ConfigReader() = default;
  virtual ~ConfigReader() {}

  bool logToConsoleAlso;
  std::string logFile;
  LogLevel logLevel;

#ifdef __unix__
  std::string pidFile;
#endif /* __unix__ */

  std::string ruleFile;
  uint16_t dnsPort;
  uint16_t tcpPort;

  std::vector<UpstreamServer> servers;

  virtual void LoadConfiguration();

protected:
  void addServer(const std::string &host, uint16_t port,
                 const std::string &protocol);

  virtual std::string getStringValue(const std::string &key,
                                     const std::string &defValue) = 0;
  virtual long getLongValue(const std::string &key, const long &defValue) = 0;
  virtual bool getBoolValue(const std::string &key, const bool &defValue) = 0;
};

class IniConfigReader : public ConfigReader {
public:
  IniConfigReader(const std::string &filePath) : filePath(filePath) {}
  virtual ~IniConfigReader() {}
  void LoadConfiguration();

protected:
  virtual std::string getStringValue(const std::string &key,
                                     const std::string &defValue);
  virtual long getLongValue(const std::string &key, const long &defValue);
  virtual bool getBoolValue(const std::string &key, const bool &defValue);

private:
  const std::string &filePath;
  boost::property_tree::ptree tree;
};
