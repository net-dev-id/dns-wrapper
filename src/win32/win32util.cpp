/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "log.hpp"
#include <windows.h>
#include <Lmcons.h>

char* GetUserName(void) {
	static char userName[UNLEN + 1];
	DWORD len = UNLEN + 1;
	if (!GetUserName(userName, &len)) {
		return nullptr;
	}

	return userName;
}

[[noreturn]] 
void Die(const std::string& baseMessage, const int exitCode) {
	char* errMesssage = strerror(errno);

	LFATAL << baseMessage << ": " << errMesssage << std::endl;
	exit(exitCode);
}

[[noreturn]]
void WinDie(const std::string& baseMessage, const DWORD res, const int exitCode) {
	char* buf = nullptr;

	LERROR << baseMessage << std::endl;

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&buf, 0, NULL)) {
		LERROR << "Error: " << buf << std::endl;
		LocalFree(buf);
	}
	exit(exitCode);
}
