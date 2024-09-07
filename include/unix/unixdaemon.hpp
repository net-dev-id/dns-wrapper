/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "daemon.hpp"

class UnixDaemon : public Daemon {
public:
  UnixDaemon() {}
  virtual ~UnixDaemon() {}

protected:
  virtual void platformInit(void);
  virtual void forkAndSetupDaemon(void);

private:
  void signalHandler(boost::system::error_code, int);
  void closeFds();
};
