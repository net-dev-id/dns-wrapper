/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "win32/win32daemon.hpp"
#include <boost/asio/windows/object_handle.hpp>
#include <boost/system/error_code.hpp>

void Win32Daemon::platformInit() {
  using namespace boost::asio;
  using namespace boost::system;

  windows::object_handle obj_handle{ ioContext, stopEvent };
  obj_handle.async_wait([&](const error_code& ec) {
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
