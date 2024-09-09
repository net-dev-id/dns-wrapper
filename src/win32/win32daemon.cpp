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
#include <boost/asio/windows/object_handle.hpp>
#include <boost/system/error_code.hpp>
#include <signal.h>

void Win32Daemon::platformInit() {
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
    if (Args::Get()->daemonMode && stopEvent == INVALID_HANDLE_VALUE) {
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
