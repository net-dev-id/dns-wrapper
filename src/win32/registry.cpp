/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "win32/registry.hpp"
#include "log.hpp"
#include <memory>
#include <winerror.h>
#include <winreg.h>

HKEY RegistryManager::OpenKey(const HKEY parentHKey, const std::string &subKey,
                              const REGSAM samDesired, const bool create) {
  HKEY hKey;
  LONG retCode = RegOpenKeyEx(parentHKey, subKey.c_str(), 0, samDesired, &hKey);
  if (retCode != ERROR_SUCCESS) {
    if (create && retCode == ERROR_FILE_NOT_FOUND) {
      return CreateKey(parentHKey, subKey, samDesired);
    }

    throw RegistryError{"Cannot open subkey from registry", retCode};
  }

  return hKey;
}

HKEY RegistryManager::CreateKey(const HKEY parentHKey,
                                const std::string &subKey,
                                const REGSAM samDesired) {
  HKEY hKey = nullptr;

  LONG retCode = RegCreateKeyEx(parentHKey, subKey.c_str(), 0, nullptr, 0,
                                samDesired, nullptr, &hKey, nullptr);

  if (retCode != ERROR_SUCCESS) {
    throw RegistryError{"Unable to create registry key", retCode};
  }

  return hKey;
}

std::vector<std::string> RegistryManager::RegEnumerateSubKeys(const HKEY hKey) {
  DWORD valueCount{};
  DWORD maxLen{};
  LONG retCode =
      RegQueryInfoKey(hKey, nullptr, nullptr, nullptr, &valueCount, &maxLen,
                      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

  if (retCode != ERROR_SUCCESS) {
    throw RegistryError{"Cannot query key info from the registry", retCode};
  }

  maxLen++;

  std::vector<std::string> values;
  for (DWORD index = 0; index < valueCount; index++) {
    values.push_back(RegSubKey(hKey, maxLen, index));
  }

  return values;
}

std::string RegistryManager::RegSubKey(const HKEY hKey, const DWORD len,
                                       const DWORD index) {
  auto nameBuffer = std::make_unique<char[]>(len);

  DWORD nameLen = len;
  LONG retCode = retCode = RegEnumKeyEx(hKey, index, nameBuffer.get(), &nameLen,
                                        nullptr, nullptr, nullptr, nullptr);

  if (retCode != ERROR_SUCCESS) {
    throw RegistryError{"Cannot get value info from the registry", retCode};
  }

  return std::string{nameBuffer.get(), nameLen};
}

std::vector<std::pair<std::string, DWORD>>
RegistryManager::RegEnumerateValues(const HKEY hKey) {
  DWORD valueCount{};
  DWORD maxValueNameLen{};
  LONG retCode = RegQueryInfoKey(hKey, nullptr, nullptr, nullptr, nullptr,
                                 nullptr, nullptr, &valueCount,
                                 &maxValueNameLen, nullptr, nullptr, nullptr);

  if (retCode != ERROR_SUCCESS) {
    throw RegistryError{"Cannot query key info from the registry", retCode};
  }

  maxValueNameLen++;

  std::vector<std::pair<std::string, DWORD>> values;
  for (DWORD index = 0; index < valueCount; index++) {
    values.push_back(RegValueType(hKey, maxValueNameLen, index));
  }

  return values;
}

std::pair<std::string, DWORD> RegistryManager::RegValueType(const HKEY hKey,
                                                            const DWORD len,
                                                            const DWORD index) {
  auto nameBuffer = std::make_unique<char[]>(len);

  DWORD valueNameLen = len;
  DWORD valueType{};
  int retCode = RegEnumValue(hKey, index, nameBuffer.get(), &valueNameLen,
                             nullptr, &valueType, nullptr, nullptr);

  if (retCode != ERROR_SUCCESS) {
    throw RegistryError{"Cannot get value info from the registry", retCode};
  }

  return std::make_pair(std::string{nameBuffer.get(), valueNameLen}, valueType);
}

#define HANDLE_ERROR \
  if (retCode == ERROR_FILE_NOT_FOUND || retCode == ERROR_BAD_PATHNAME) { \
    return defValue; \
  } \
\
if (retCode != ERROR_SUCCESS) { \
  LERROR_X << "Unable to read " << subKey << "'s value from registry" << std::endl; \
  throw RegistryError{ "Cannot read from registry.", retCode }; \
}


bool RegistryManager::RegGetBoolean(const HKEY hKey, const std::string &subKey,
                                    const std::string &value,
                                    const bool &defValue) {
  bool data{};
  DWORD dataSize = sizeof(data);

  LONG retCode = RegGetValue(hKey, subKey.c_str(), value.c_str(),
                             RRF_RT_REG_BINARY, nullptr, &data, &dataSize);
  HANDLE_ERROR
  return data;
}

DWORD RegistryManager::RegGetDword(const HKEY hKey, const std::string &subKey,
                                   const std::string &value,
                                   const DWORD &defValue) {
  DWORD data{};
  DWORD dataSize = sizeof(data);

  LONG retCode = RegGetValue(hKey, subKey.c_str(), value.c_str(),
                             RRF_RT_REG_DWORD, nullptr, &data, &dataSize);

  HANDLE_ERROR
  return data;
}

std::string RegistryManager::RegGetString(HKEY hKey, const std::string &subKey,
                                          const std::string &value,
                                          const std::string &defValue) {
  DWORD dataSize{};
  LONG retCode = RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ,
                             nullptr, nullptr, &dataSize);

  HANDLE_ERROR
  std::string data;
  data.resize(dataSize / sizeof(char));

  retCode = RegGetValue(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ,
                        nullptr, &data[0], &dataSize);

  HANDLE_ERROR
  DWORD stringLengthInchars = dataSize / sizeof(char);

  stringLengthInchars--; // Exclude the NUL written by the Win32 API
  data.resize(stringLengthInchars);

  return data;
}
