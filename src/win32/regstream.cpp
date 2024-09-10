/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "win32/regstream.hpp"
#include <winerror.h>
#include <winreg.h>
#include <memory>

std::streamsize RegStream::read(char* s, std::streamsize n) {

}

std::vector<std::pair<std::string, DWORD>> RegStream::regEnumValues(HKEY hKey) {
	DWORD valueCount{};
	DWORD maxValueNameLen{};
	LONG retCode = ::RegQueryInfoKey(
		hKey,
		nullptr,    // No user-defined class
		nullptr,    // No user-defined class size
		nullptr,    // Reserved
		nullptr,    // No subkey count
		nullptr,    // No subkey max length
		nullptr,    // No subkey class length
		&valueCount,
		&maxValueNameLen,
		nullptr,    // No max value length
		nullptr,    // No security descriptor
		nullptr     // No last write time
	);

	if (retCode != ERROR_SUCCESS)
	{
		throw RegistryError{ "Cannot query key info from the registry", retCode};
	}

	maxValueNameLen++;

	std::vector<std::pair<std::string, DWORD>> values;
	for (DWORD index = 0; index < valueCount; index++)
	{
		values.push_back(regEnumType(hKey, maxValueNameLen, index));
	}

	return values;
}

std::pair<std::string, DWORD> RegStream::regEnumType(const HKEY hKey, const DWORD len, const DWORD index) {
	auto nameBuffer = std::make_unique<char[]>(len);

	DWORD valueNameLen = len;
	DWORD valueType{};
	int retCode = ::RegEnumValue(
		hKey,
		index,
		nameBuffer.get(),
		&valueNameLen,
		nullptr,
		&valueType,
		nullptr,
		nullptr
	);

	if (retCode != ERROR_SUCCESS)
	{
		throw RegistryError{ "Cannot get value info from the registry", retCode };
	}

	return std::make_pair(
		std::string{ nameBuffer.get(), valueNameLen },
		valueType
	);
}

DWORD RegStream::regGetDword(const HKEY hKey, const std::string& subKey, const std::string& value) {
	DWORD data{};
	DWORD dataSize = sizeof(data);

	LONG retCode = ::RegGetValue(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_DWORD,
		nullptr,
		&data,
		&dataSize
	);

	if (retCode != ERROR_SUCCESS)
	{
		throw RegistryError{ "Cannot read DWORD from registry.", retCode };
	}

	return data;
}

std::string RegStream::regGetString(
	HKEY hKey,
	const std::string& subKey,
	const std::string& value
) {
	DWORD dataSize{};
	LONG retCode = ::RegGetValue(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		nullptr,
		&dataSize
	);

	if (retCode != ERROR_SUCCESS)
	{
		throw RegistryError{ "Cannot read string from registry", retCode };
	}

	std::string data;
	data.resize(dataSize / sizeof(char));

	retCode = ::RegGetValue(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		&data[0],
		&dataSize
	);

	if (retCode != ERROR_SUCCESS)
	{
		throw RegistryError{ "Cannot read string from registry", retCode };
	}

	DWORD stringLengthInWchars = dataSize / sizeof(char);

	stringLengthInWchars--; // Exclude the NUL written by the Win32 API
	data.resize(stringLengthInWchars);

	return data;
}
