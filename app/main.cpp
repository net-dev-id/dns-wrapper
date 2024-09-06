// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#include "daemon.hpp"
#include <ostream>

#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#endif

#include <iostream>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Daemon daemon;
  daemon.ParseArgs(argc, argv);

  if (daemon.IsAlreadyRunning()) {
    std::cerr << "An instance of Dns Wrapper is already executing. Exiting!!!"
              << std::endl;
    exit(1);
  }

  return daemon.Run();

  /*
    eUsername = getUserName();
    configReader.LoadConfiguration(CONFIG_FILE_PATH);

      LTRACE << "Dns Wrapper v" << PROJECT_VERSION_MAJOR << "."
             << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << "."
             << PROJECT_VERSION_TWEAK << std::endl;
      LTRACE << "Git branch: " << GIT_BRANCH << ", Version: " << GIT_VERSION
             << ", Commit: " << GIT_HASH << ", Date: " << GIT_DATE << std::endl;
      LTRACE << "Current User: " << eUsername << std::endl;
      LTRACE << "Compiled for " << COMPILATION_ARCH << " using " <<
    COMPILATION_CC
             << std::endl;

      BytePacketBuffer bpb = {{0x86, 0x2a, 0x01, 0x20, 0x00, 0x01, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x67,
                               0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x03, 0x63,
                               0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01},
                              0};
      DnsPacket dp;
      int code = dp.Read(&bpb);
      std::cout << "Packet read status: " << code << std::endl;
      std::cout << dp << std::endl;

      BytePacketBuffer bpb2;
      bpb2.pos = 0;
      code = dp.Write(&bpb2);
      std::cout << "Packet write status: " << code << std::endl;

      DnsPacket dp2;
      code = dp2.Read(&bpb2);
      std::cout << "Packet read status: " << code << std::endl;
      std::cout << dp << std::endl;

      // Bring in the dummy class from the example source,
      // just to show that it is accessible from main.cpp.
      Dummy d = Dummy();
      return d.doSomething() ? 0 : -1;
      */
}
