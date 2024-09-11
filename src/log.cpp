/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "log.hpp"
#include "config.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#define LOG_FORMAT                                                             \
  "[%TimeStamp%] [%ThreadID%] [%Severity%] [%ProcessID%] [%LineID%] %Message%"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;

void Log::Init(const ConfigReader *configReader) {
  logging::register_simple_formatter_factory<logging::trivial::severity_level,
                                             char>("Severity");
  if (configReader->logToConsoleAlso) {
    logging::add_console_log(std::cout, keywords::format = LOG_FORMAT);
  }

  logging::add_file_log(keywords::file_name = configReader->logFile,
                        keywords::format = LOG_FORMAT);

  logging::core::get()->set_filter(logging::trivial::severity >=
                                   configReader->logLevel);

  logging::add_common_attributes();
}

LogLevel Log::ToLogLevel(const std::string &level) {
  LogLevel logLevel;
  if (boost::log::trivial::from_string(level.c_str(), level.length(),
                                       logLevel)) {
    return logLevel;
  }

  return LogLevel::info;
}

std::string Log::FromLogLevel(const LogLevel &level) {
  const char *type = boost::log::trivial::to_string(level);
  if (type == nullptr) {
    return boost::log::trivial::to_string(
        boost::log::trivial::severity_level::info);
  }

  return type;
}
