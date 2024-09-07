/*
 * Created Date: Friday, September 6th 2024, 6:54:04 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Su/09/2024 12:nn:19
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#include "daemon.hpp"
#include "log.hpp"
#include <ostream>

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#endif

#include <iostream>
#include <stdlib.h>

#ifndef WIN32
#include "unix/unixdaemon.hpp"
#endif

int main(int argc, char *argv[]) {
#ifndef WIN32
  UnixDaemon daemon;
#endif
  daemon.ParseArgs(argc, argv);

  if (daemon.IsAlreadyRunning()) {
    std::cerr << "An instance of Dns Wrapper is already executing. Exiting!!!"
              << std::endl;
    exit(1);
  }

  int exitCode = daemon.Run();

  LTRACE << "Exiting with exit code: " << exitCode << std::endl;
  return exitCode;
}
