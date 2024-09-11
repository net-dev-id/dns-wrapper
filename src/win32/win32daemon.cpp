/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "win32/win32daemon.hpp"
#include "args.hpp"
#include "common.h"
#include "log.hpp"
#include "win32/win32config.hpp"
#include <boost/asio/windows/object_handle.hpp>
#include <boost/system/error_code.hpp>
#include <signal.h>

Win32Daemon::Win32Daemon(HANDLE stopEvent) : stopEvent(stopEvent) {
  if (Args::Get()->configFile == CONFIG_FILE_PATH) {
    configReader = new Win32ConfigReader();
  } else {
    configReader = new IniConfigReader(Args::Get()->configFile);
  }
}

Win32Daemon::~Win32Daemon() {
  if (Args::Get()->configFile == CONFIG_FILE_PATH) {
    delete (Win32ConfigReader *)configReader;
  } else {
    delete (IniConfigReader *)configReader;
  }
}

void Win32Daemon::platformInit() {
  if (!Args::Get()->daemonMode || stopEvent == INVALID_HANDLE_VALUE) {
    return;
  }

  using namespace boost::asio;
  using namespace boost::system;

  windows::object_handle obj_handle{ioContext, stopEvent};
  obj_handle.async_wait([&](const error_code &ec) {
    if (ec) {
      LERROR << "Failed waiting for stop event" << std::endl;
      return;
    }

    LERROR << "Received stop event. Exiting." << std::endl;
    LDEBUG << "Stopping ioContext" << std::endl;
    ioContext.stop();
  });
}

void Win32Daemon::forkAndSetupDaemon() {}

void Win32Daemon::signalHandler(boost::system::error_code ec, int signalNo) {
  if (ec) {
    LERROR << "Error during signal handling: " << ec << " [" << signalNo << "]"
           << std::endl;
    exit(EC_SIGNAL);
  }

  if (signalNo == SIGTERM || signalNo == SIGINT) {
    LINFO << "Received signal: " << signalNo << " exiting." << std::endl;
    if (!Args::Get()->daemonMode || stopEvent == INVALID_HANDLE_VALUE) {
      LDEBUG << "Stopping ioContext" << std::endl;
      ioContext.stop();
    } else {
      LDEBUG << "Signal handling skipped as running in daemon mode. Stop "
                "service instead."
             << std::endl;
    }
  } else {
    LERROR << "Received signal: " << signalNo << " ignoring." << std::endl;
  }
}
