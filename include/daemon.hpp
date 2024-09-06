#pragma once

#include "config.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#define DAEMON_NAME "dns-wrapper"

class Daemon {
public:
  Daemon();
  ~Daemon() {
    removePid();

    if (lockOwned) {
      boost::interprocess::named_mutex::remove(DAEMON_NAME);
    }
  }

  void ParseArgs(int argc, char *argv[]);
  bool IsAlreadyRunning();
  int Run();

private:
  void savePid(void);
  void removePid(void);
  void forkAndSetupDaemon(void);
  void signalHandler(boost::system::error_code, int);

private:
  bool lockOwned = false;
  boost::interprocess::named_mutex executionLock;

  bool pidFileCreated = false;
  bool forkDone = false;
  int childPid = -1;

  std::string userName;
  ConfigReader configReader;
  boost::asio::io_context io_context;
};
