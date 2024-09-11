/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "args.hpp"
#include "daemon.hpp"
#include "win32/win32config.hpp"
#include <winsvc.h>

class Win32Daemon : public Daemon {
public:
  Win32Daemon(HANDLE stopEvent) : stopEvent(stopEvent) {
	if (Args::Get()->configFile == CONFIG_FILE_PATH) {
	  configReader = new Win32ConfigReader();
	} else {
	  configReader = new IniConfigReader(Args::Get()->configFile);
	}
  }
  
  virtual ~Win32Daemon() {
    if (Args::Get()->configFile == CONFIG_FILE_PATH) {
	  delete (Win32ConfigReader*)configReader;
	} else {
	  delete (IniConfigReader*)configReader;
	}
  }

private:
  virtual void platformInit();
  virtual void forkAndSetupDaemon(void);
  virtual void signalHandler(boost::system::error_code, int);

  HANDLE stopEvent;
};
