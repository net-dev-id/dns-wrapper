/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <boost/iostreams/categories.hpp> // source_tag
#include <iosfwd>                         // streamsize
#include <string>
#include  <stdexcept>
#include <vector>
#include <utility>
#include <windows.h>
	
// Adapted from: 
// https://learn.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry

namespace io = boost::iostreams;

class RegistryError: public std::runtime_error
{
public:
    RegistryError(const char* message, long errorCode)
        : std::runtime_error{ message }
        , m_errorCode{ errorCode }
    {}

    long ErrorCode() const noexcept
    {
        return m_errorCode;
    }

private:
    long m_errorCode;
};

class RegStream {
public:
    typedef char char_type;
    typedef boost::iostreams::source_tag category;

    std::streamsize read(char* s, std::streamsize n);

private:
    std::vector<std::pair<std::string, DWORD>> regEnumValues(HKEY hKey);
    std::pair<std::string, DWORD> regEnumType(const HKEY hKey, const DWORD len, const DWORD index);
    DWORD regGetDword(const HKEY hKey, const std::string &subKey, const std::string &value);
    std::string regGetString(
        HKEY hKey,
        const std::string& subKey,
        const std::string& value
    );

};
