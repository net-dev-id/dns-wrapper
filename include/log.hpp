/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#ifndef WIN32
// As per
// https://stackoverflow.com/questions/23137637/linker-error-while-linking-boost-log-tutorial-undefined-references
#define BOOST_LOG_DYN_LINK 1
#else
// As per
// https://github.com/microsoft/vcpkg/discussions/22762
#define _WIN32_WINNT 0x0601
#endif /* WIN32 */

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>

#define LTRACE BOOST_LOG_TRIVIAL(trace)
#define LDEBUG BOOST_LOG_TRIVIAL(debug)
#define LINFO BOOST_LOG_TRIVIAL(info)
#define LWARNING BOOST_LOG_TRIVIAL(warning)
#define LERROR BOOST_LOG_TRIVIAL(error)
#define LFATAL BOOST_LOG_TRIVIAL(fatal)

// https://www.sentinelone.com/blog/getting-started-quickly-cplusplus-logging/

#define LERROR_X LERROR << "[" << __FILE__ << ":" << __LINE__ << "] "

struct ConfigReader;

typedef boost::log::trivial::severity_level LogLevel;

class Log {
public:
  static void Init(const ConfigReader &configReader);
  static LogLevel ToLogLevel(const std::string &);
  static std::string FromLogLevel(const LogLevel &);
};