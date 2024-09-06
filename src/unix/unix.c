#include "unix/unix.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void closeFds(long max_fd, int spare1, int spare2, int spare3);

// https://thekelleys.org.uk/dnsmasq/docs/dnsmasq-man.html
// Conn tracking
// DNSSEC
// auth zones
// auth server
// HAVE_IPSET define this to include the ability to selectively add resolved ip
//            addresses to given ipsets.
// HAVE_NFTSET define this to include the ability to selectively add resolved ip
//             addresses to given nftables sets

void ForkDaemon(void) {
  umask(022);
  closeFds(sysconf(_SC_OPEN_MAX), -1, -1, -1);
}

void closeFds(long max_fd, int spare1, int spare2, int spare3) {
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
  for (max_fd--; max_fd >= 0; max_fd--)
    if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO &&
        max_fd != STDIN_FILENO && max_fd != spare1 && max_fd != spare2 &&
        max_fd != spare3)
      close(max_fd);
}
