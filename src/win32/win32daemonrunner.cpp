/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "win32/win32daemon.hpp"

static Win32Daemon *daemon;

void ServiceInit(const HANDLE stopEvent) {
  daemon = new Win32Daemon(stopEvent);
  daemon->Initialize();
}

void ServiceStart() { daemon->Start(); }

void ServiceStop() {
  daemon->Stop();
  delete daemon;
}
