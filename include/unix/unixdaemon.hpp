/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "args.hpp"
#include "config.hpp"
#include "daemon.hpp"
#include <memory>

class UnixDaemon : public Daemon {
public:
  UnixDaemon() : pidFileCreated(false) {
    configReader = std::make_unique<IniConfigReader>(Args::Get()->configFile);
  }
  virtual ~UnixDaemon() {
    LTRACE << "Removing pid file" << std::endl;
    removePid();
  }

private:
  void savePid(void);
  void removePid(void);
  virtual void platformInit();
  virtual void forkAndSetupDaemon(void);

private:
  void signalHandler(boost::system::error_code, int);
  void closeFds();

private:
  bool pidFileCreated;
};
