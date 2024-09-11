/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "common.h"
#include "win32/win32config.hpp"

std::string Win32ConfigReader::baseKey = "\\SOFTWARE\\NetDevId\\DnsWrapper\\main";

std::string Win32ConfigReader::getStringValue(const std::string& key, const std::string& defValue) {
    return m.RegGetString(HKEY_LOCAL_MACHINE, baseKey, key, defValue);
}

long Win32ConfigReader::getLongValue(const std::string& key, const long& defValue) {
    return m.RegGetDword(HKEY_LOCAL_MACHINE, baseKey, key, defValue);
}

bool Win32ConfigReader::getBoolValue(const std::string& key, const bool& defValue) {
    return m.RegGetBoolean(HKEY_LOCAL_MACHINE, baseKey, key, defValue);
}
