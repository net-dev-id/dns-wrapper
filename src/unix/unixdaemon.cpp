/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "unix/unixdaemon.hpp"
#include "args.hpp"
#include "common.h"
#include "log.hpp"
#include "unix/unixutil.hpp"
#include <boost/asio/signal_set.hpp>
#include <filesystem>
#include <fstream>
#include <unistd.h>

void UnixDaemon::platformInit() {}

void UnixDaemon::savePid(void) {
  const pid_t pid = getpid();
  std::ofstream pidFile(configReader->pidFile);
  if (!pidFile) {
    LWARNING << "Unable to write PID to file. PID: " << pid << std::endl;
    return;
  }

  pidFile << getpid();
  pidFile.close();
  LINFO << "PID file: " << configReader->pidFile << " written.";
  pidFileCreated = true;
}

void UnixDaemon::removePid(void) {
  if (!pidFileCreated) {
    return;
  }

  if (std::filesystem::remove(configReader->pidFile)) {
    LINFO << "PID file: " << configReader->pidFile << " removed." << std::endl;
  } else {
    LWARNING << "Deleting of PID file: " << configReader->pidFile
             << " not successful." << std::endl;
  }
}

void UnixDaemon::forkAndSetupDaemon() {
  if (!Args::Get()->daemonMode) {
    LDEBUG << "Skipping entering into daemon mode as per user request"
           << std::endl;
    return;
  }

  pid_t pid;
  if ((pid = fork()) == -1) {
    UnixUtil::Die("Unable to fork", EC_FORK);
  }

  if (pid > 0) {
    // Parent process
    _exit(EC_GOOD);
  }

  setsid();
  if (chdir("/") != 0)
    UnixUtil::Die("Cannot chdir to filesystem root", EC_MISC);
  umask(022);

  if ((pid = fork()) == -1) {
    UnixUtil::Die("Unable to fork", EC_FORK);
  }

  if (pid > 0) {
    _exit(EC_GOOD);
  }

  savePid();
  closeFds();
}

void UnixDaemon::closeFds() {
  close(0);
  close(1);
  close(2);

  if (open("/dev/null", O_RDONLY) < 0) {
    UnixUtil::Die("Failed to open /dev/null", EC_MISC);
  }
}

void UnixDaemon::signalHandler(boost::system::error_code ec, int signalNo) {
  if (ec) {
    LERROR << "Error during signal handling: " << ec << " [" << signalNo << "]"
           << std::endl;
    exit(EC_SIGNAL);
  }

  if (signalNo == SIGTERM || signalNo == SIGINT) {
    LINFO << "Received signal: " << signalNo << " exiting." << std::endl;
    LDEBUG << "Stopping ioContext" << std::endl;
    ioContext.stop();
  } else {
    LERROR << "Received signal: " << signalNo << " ignoring." << std::endl;
  }
}
