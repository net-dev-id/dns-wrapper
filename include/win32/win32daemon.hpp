/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "daemon.hpp"
#include <winsvc.h>

class Win32Daemon : public Daemon {
public:
  Win32Daemon(HANDLE stopEvent) : stopEvent(stopEvent) {}
  virtual ~Win32Daemon() {}

private:
  virtual void platformInit();
  virtual void forkAndSetupDaemon(void);
  virtual void signalHandler(boost::system::error_code, int);

  HANDLE stopEvent;
};
