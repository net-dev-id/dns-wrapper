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
#include <boost/property_tree/ptree.hpp>

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

class ConfigReader {
public:
  bool logToConsoleAlso;
  std::string logFile;
  LogLevel logLevel;

#ifdef UNIX
  std::string pidFile;
#endif /* UNIX */

  uint16_t dnsPort;
  uint16_t tcpPort;

  std::vector<UpstreamServer> servers;

  virtual void LoadConfiguration();

private:
  static std::regex ipRegex4;
  static std::regex ipRegex6;

protected:
  void addServer(const std::string &host, uint16_t port,
                 const std::string &protocol);

  virtual std::string getStringValue(const std::string& key, const std::string& defValue) = 0;
  virtual long getLongValue(const std::string& key, const long& defValue) = 0;
  virtual bool getBoolValue(const std::string& key, const bool& defValue) = 0;
};

class IniConfigReader : public ConfigReader {
public:
  IniConfigReader(const std::string& filePath) : filePath(filePath) {}
  void LoadConfiguration();

protected:
  virtual std::string getStringValue(const std::string& key, const std::string& defValue);
  virtual long getLongValue(const std::string& key, const long& defValue);
  virtual bool getBoolValue(const std::string& key, const bool& defValue);

private:
  const std::string& filePath;
  boost::property_tree::ptree tree;
};
