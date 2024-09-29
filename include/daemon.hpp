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
#include "rule/shm.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <memory>

class Daemon {
public:
  Daemon();
  virtual ~Daemon();

  bool IsAlreadyRunning();
  int Run();
  void Initialize();
  int Start();
  void Stop();

  virtual void platformInit() = 0;
  virtual void forkAndSetupDaemon(void) = 0;

protected:
  virtual void signalHandler(boost::system::error_code, int) = 0;

private:
  bool lockOwned;
  boost::interprocess::named_mutex executionLock;

  int childPid = -1;

  std::string userName;

protected:
  std::unique_ptr<ConfigReader> configReader;
  ShmRuleEngine ruleEngine;
  boost::asio::io_context ioContext;
};
