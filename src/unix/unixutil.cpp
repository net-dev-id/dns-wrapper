/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "unix/unixutil.hpp"
#include "log.hpp"
#include <pwd.h>

void UnixUtil::Die(const std::string &baseMessage, const int exitCode) {
  char *errMesssage = strerror(errno);

  LFATAL << baseMessage << ": " << errMesssage << std::endl;
  exit(exitCode);
}

char *GetUserName(void) {
  char *name;
  const uid_t euid = geteuid();
  const struct passwd *pw = getpwuid(euid);
  if (pw) {
    name = strdup(pw->pw_name);
  } else {
    if (asprintf(&name, "%u", euid) < 0)
      return NULL;
  }

  return name;
}

