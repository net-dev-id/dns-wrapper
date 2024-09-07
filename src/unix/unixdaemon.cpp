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
#include <unistd.h>

void UnixDaemon::platformInit() {}

void UnixDaemon::forkAndSetupDaemon() {
  if (!Args::Get()->daemonMode) {
    LDEBUG << "Skipping entering into daemon mode as per user request" << std::endl;
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

  closeFds(sysconf(_SC_OPEN_MAX), -1, -1, -1);
}

void UnixDaemon::closeFds(long max_fd, int spare1, int spare2, int spare3) {
  /* On Linux, use the /proc/ filesystem to find which files
     are actually open, rather than iterate over the whole space,
     for efficiency reasons. If this fails we drop back to the dumb code. */
#if defined(__linux__)
  DIR *d;

  if ((d = opendir("/proc/self/fd"))) {
    struct dirent *de;

    while ((de = readdir(d))) {
      long fd;
      char *e = NULL;

      errno = 0;
      fd = strtol(de->d_name, &e, 10);

      if (errno != 0 || !e || *e || fd == dirfd(d) || fd == STDOUT_FILENO ||
          fd == STDERR_FILENO || fd == STDIN_FILENO || fd == spare1 ||
          fd == spare2 || fd == spare3)
        continue;

      close(fd);
    }

    closedir(d);
    return;
  }
#endif

  /* fallback, dumb code. */
  for (max_fd--; max_fd >= 0; max_fd--) {
    if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO &&
        max_fd != STDIN_FILENO && max_fd != spare1 && max_fd != spare2 &&
        max_fd != spare3) {
      close(max_fd);
    }
  }
}

void UnixDaemon::signalHandler(boost::system::error_code ec, int signalNo) {
  if (ec) {
    LERROR << "Error during signal handling: " << ec << std::endl;
    exit(EC_SIGNAL);
  }

  if (signalNo == SIGTERM || signalNo == SIGINT) {
    LERROR << "Received signal: " << signalNo << " exiting." << std::endl;
    LDEBUG << "Stopping ioContext" << std::endl;
    ioContext.stop();
  } else {
    LERROR << "Received signal: " << signalNo << " ignoring." << std::endl;
  }
}
