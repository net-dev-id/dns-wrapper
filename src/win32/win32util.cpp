/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

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

