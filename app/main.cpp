/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#include "args.hpp"
#include "daemon.hpp"
#include "log.hpp"
#include <ostream>

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#endif

#include "common.h"
#include <iostream>
#include <stdlib.h>

#ifdef WIN32
#include "win32/service.hpp"
#include "win32/win32daemon.hpp"
#else
#include "unix/unixdaemon.hpp"
#endif

int main(int argc, char *argv[]) {
  Args::ExitCode code = Args::Init(argc, argv);
  switch (code) {
  case Args::ExitWithUnlock:
    boost::interprocess::named_mutex::remove(DAEMON_NAME);
    [[fallthrough]];
  case Args::ExitWithNoError:
    exit(EC_GOOD);
  case Args::ExitWithError:
    exit(EC_BADCOMMANDLINE);
  case Args::NoExit:
    break;
  }

#ifdef WIN32
  if (Args::Get()->daemonMode) {
    return ServiceMain(argc, argv);
  }

  Win32Daemon daemon(INVALID_HANDLE_VALUE);
#else
  UnixDaemon daemon;
#endif /* WIN32 */

  if (daemon.IsAlreadyRunning()) {
    std::cerr << "An instance of Dns Wrapper is already executing. Exiting!!!"
              << std::endl;
    exit(1);
  }

  int exitCode = daemon.Run();

  LTRACE << "Exiting with exit code: " << exitCode << std::endl;
  return exitCode;
}
