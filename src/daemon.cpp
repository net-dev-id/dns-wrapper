/*
 * Created Date: Thursday, August 22nd 2024, 8:48:01 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 26th August 2024 11:35:42 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
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
#include <unistd.h>

Daemon::Daemon()
    : lockOwned(false),
      executionLock(boost::interprocess::open_or_create, DAEMON_NAME),
      pidFileCreated(false), forkDone(false) {}

void Daemon::ParseArgs(int argc, char *argv[]) {
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

int Daemon::Run() {
  try {
    userName = GetUserName();
    configReader.LoadConfiguration(Args::Get()->configFile);
    Log::Init(configReader);
    logBasics(userName);
    savePid();

    DnsServer dnsServer(io_context, configReader.dnsPort, &configReader);
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code /*ec*/, int /*signo*/) {
      io_context.stop();
    });

    io_context.notify_fork(boost::asio::io_context::fork_prepare);
    forkAndSetupDaemon();
    forkDone = true;
    io_context.notify_fork(boost::asio::io_context::fork_child);

    LTRACE << "Daemon started on port: " << configReader.dnsPort << std::endl;
    io_context.run();
    LTRACE << "Daemon stopped on port: " << configReader.dnsPort << std::endl;

    return 0;
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

void Daemon::savePid(void) {
  const pid_t pid = getpid();
  std::ofstream pidFile(configReader.pidFile);
  if (!pidFile) {
    LWARNING << "Unable to write PID to file. PID: " << pid << std::endl;
    return;
  }

  pidFile << getpid();
  pidFile.close();
  LINFO << "PID file: " << configReader.pidFile << " written.";
  pidFileCreated = true;
}

void Daemon::removePid(void) {
  if (!pidFileCreated) {
    return;
  }

  if (std::filesystem::remove(configReader.pidFile)) {
    LINFO << "PID file: " << configReader.pidFile << " removed." << std::endl;
  } else {
    LWARNING << "Deleting of PID file: " << configReader.pidFile
             << " not successful." << std::endl;
  }
}

void Daemon::signalHandler(boost::system::error_code ec, int signalNo) {
  if (ec) {
    LERROR << "Error during signal handling: " << ec << std::endl;
    exit(EC_SIGNAL);
  }

  io_context.stop();

  if (!forkDone) {
    if (signalNo == SIGTERM || signalNo == SIGINT) {
      exit(EC_SIGNAL);
    }
  } else if (childPid != getpid()) {
    // Children of children
    // in ftl db is closed here in case SIGALRM
  } else {
    /*
    int event, errsave = errno;
    if (signalNo == SIGHUP)
      event = EVENT_RELOAD;
    else if (signalNo == SIGCHLD)
      event = EVENT_CHILD;
    else if (signalNo == SIGALRM)
      event = EVENT_ALARM;
    else if (signalNo == SIGUSR6) // Pi-hole modified
      event = EVENT_TERM;
    else if (signalNo == SIGUSR1)
      event = EVENT_DUMP;
    else if (signalNo == SIGUSR2)
      event = EVENT_REOPEN;
    else if (signalNo == SIGINT) {
      io_context.stop();
      // Handle SIGINT normally in debug mode, so ctrl-c continues to operate.
    if (option_bool(OPT_DEBUG))
      exit(EC_MISC);
    else
      event = EVENT_TIME;
    */
  }

  // send_event(pipewrite, event, 0, NULL);
  // errno = errsave;
}

void Daemon::forkAndSetupDaemon(void) {
  /*
  if (pid_t pid = fork()) {
    if (pid > 0) {
      // Parent process
    } else {
      LERROR << "Fork failed: "
    }
  }
  */
}
