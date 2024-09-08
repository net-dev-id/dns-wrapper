/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "daemon.hpp"
#include "args.hpp"
#include "common.h"
#include "dns/server.hpp"
#include "log.hpp"
#include "util.hpp"
#include "version.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <csignal>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>

Daemon::Daemon()
    : lockOwned(false),
      executionLock(boost::interprocess::open_or_create, DAEMON_NAME) {}

int Daemon::Run() {
  Initialize();
  return Start();
}

bool Daemon::IsAlreadyRunning() {
  lockOwned = executionLock.try_lock();
  return !lockOwned;
}

static void logBasics(const std::string &userName) {
  LTRACE << "Dns Wrapper v" << PROJECT_VERSION_MAJOR << "."
         << PROJECT_VERSION_MINOR << "." << PROJECT_VERSION_PATCH << "."
         << PROJECT_VERSION_TWEAK << std::endl;
  LTRACE << "Git branch: " << GIT_BRANCH << ", Version: " << GIT_VERSION
         << ", Commit: " << GIT_HASH << ", Date: " << GIT_DATE << std::endl;
  LTRACE << "Current User: " << userName << std::endl;
  LTRACE << "Compiled for " << COMPILATION_ARCH << " using " << COMPILATION_CC
         << std::endl;
}

void Daemon::Initialize() {
  userName = GetUserName();
  configReader.LoadConfiguration(Args::Get()->configFile);
  Log::Init(configReader);
  logBasics(userName);
  platformInit();
}

int Daemon::Start() {
  try {
    DnsServer dnsServer(ioContext, configReader.dnsPort, &configReader);
    ioContext.notify_fork(boost::asio::io_context::fork_prepare);
    forkAndSetupDaemon();
    ioContext.notify_fork(boost::asio::io_context::fork_child);

    LTRACE << "Daemon started on port: " << configReader.dnsPort << std::endl;
    ioContext.run();
    LTRACE << "Daemon stopped on port: " << configReader.dnsPort << std::endl;

    return EC_GOOD;
  } catch (boost::property_tree::ini_parser_error &) {
    return EC_BADCONF;
  } catch (std::runtime_error &e) {
    std::cerr << "Runtime error occurred during execution: " << e.what()
              << std::endl;
    return EC_MISC;
  } catch (std::exception &e) {
    std::cerr << "Exception occurred during execution: " << e.what()
              << std::endl;
    return EC_MISC;
  } catch (...) {
    std::cerr << "Exception caught. Exiting!!" << std::endl;
    return EC_MISC;
  }
}
