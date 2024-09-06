/*
 * Created Date: Wednesday, August 21st 2024, 5:00:04 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Sunday, 1st September 2024 10:02:41 pm
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "util.hpp"
#include "dns/dnscommon.hpp"
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ROW_SIZE 16

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

void DumpHex(std::array<uint8_t, MAX_PACKET_SZ> data, const std::size_t n) {
  using namespace std;

  for (size_t j = 0; j < (n / ROW_SIZE) + 1; j++) {
    cout << "cc>> ";
    for (size_t i = 0; i < ROW_SIZE && j * ROW_SIZE + i < n; ++i)
      cout << setfill(' ') << setw(2)
           << (isprint(data[j * ROW_SIZE + i]) ? char(data[j * ROW_SIZE + i]) : '.') << " ";
    cout << endl;

    cout << "0x>> ";
    for (size_t i = 0; i < ROW_SIZE && j * ROW_SIZE + i < n; ++i)
      cout << hex << setfill('0') << setw(2) << int(data[j * ROW_SIZE + i])
           << " ";
    cout << endl;
  }
}
