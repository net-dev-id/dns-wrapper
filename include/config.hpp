/*
 * Created Date: Wednesday, August 21st 2024, 6:34:32 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 26th August 2024 11:27:27 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#pragma once

#include "log.hpp"
#include <cstdint>
#include <regex>
#include <string>
#include <vector>

/*
        // Initialize the lock attributes
        pthread_mutexattr_t lock_attr = {};
        pthread_mutexattr_init(&lock_attr);

        // Initialize the lock
        pthread_mutex_init(&lock, &lock_attr);

        // Destroy the lock attributes since we're done with it
        pthread_mutexattr_destroy(&lock_attr);
*/

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
  std::string pidFile;

  uint16_t dnsPort;
  uint16_t tcpPort;

  std::vector<UpstreamServer> servers;

  void LoadConfiguration(const std::string &filePath);
  void SaveConfiguration(const std::string &filePath);

  static std::regex ipRegex4;
  static std::regex ipRegex6;

private:
  void addServer(const std::string &host, uint16_t port,
                 const std::string &protocol);
};
