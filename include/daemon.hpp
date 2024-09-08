/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "config.hpp"
#include "log.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#define DAEMON_NAME "dns-wrapper"

class Daemon {
public:
  Daemon();
  virtual ~Daemon() {
    if (lockOwned) {
      LTRACE << "Removing owning process lock" << std::endl;
      boost::interprocess::named_mutex::remove(DAEMON_NAME);
    }
  }

  bool IsAlreadyRunning();
  int Run();
  void Initialize();
  int Start();

  virtual void platformInit() = 0;
  virtual void forkAndSetupDaemon(void) = 0;

private:
  bool lockOwned = false;
  boost::interprocess::named_mutex executionLock;

  int childPid = -1;

  std::string userName;

protected:
  ConfigReader configReader;
  boost::asio::io_context ioContext;
};
