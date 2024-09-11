/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

 // Adapted from:
 // https://learn.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry

class RegistryError : public std::runtime_error {
public:
    RegistryError(const char* message, long errorCode)
        : std::runtime_error{ message }, m_errorCode{ errorCode } {}

    long ErrorCode() const noexcept { return m_errorCode; }

private:
    long m_errorCode;
};

class RegistryManager {
public:
public:
    RegistryManager() {}
    ~RegistryManager() {}

    HKEY OpenKey(const HKEY parentHKey, const std::string& subKey,
        const REGSAM samDesired, const bool create = false);
    HKEY CreateKey(const HKEY parentHKey, const std::string& subKey,
        const REGSAM samDesired);
    std::vector<std::string> RegEnumerateSubKeys(const HKEY hKey);
    std::string RegSubKey(const HKEY hKey, const DWORD len, const DWORD index);
    std::vector<std::pair<std::string, DWORD>>
        RegEnumerateValues(const HKEY hKey);
    std::pair<std::string, DWORD> RegValueType(const HKEY hKey, const DWORD len,
        const DWORD index);
    bool RegGetBoolean(const HKEY hKey, const std::string& subKey,
        const std::string& value, const bool& defValue);
    DWORD RegGetDword(const HKEY hKey, const std::string& subKey,
        const std::string& value, const DWORD& defValue);
    std::string RegGetString(HKEY hKey, const std::string& subKey,
        const std::string& value, const std::string& defValue);
};
