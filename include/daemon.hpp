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
    LTRACE << "Removing pid file" << std::endl;
    removePid();

    if (lockOwned) {
      LTRACE << "Removing owning process lock" << std::endl;
      boost::interprocess::named_mutex::remove(DAEMON_NAME);
    }
  }

  void ParseArgs(int argc, char *argv[]);
  bool IsAlreadyRunning();
  int Run();

protected:
  virtual void platformInit(void) = 0;
  void savePid(void);
  void removePid(void);
  virtual void forkAndSetupDaemon(void) = 0;
  virtual void signalHandler(boost::system::error_code, int) = 0;

private:
  bool lockOwned = false;
  boost::interprocess::named_mutex executionLock;

  bool pidFileCreated = false;
  int childPid = -1;

  std::string userName;
  ConfigReader configReader;

protected:
  boost::asio::io_context ioContext;
};
