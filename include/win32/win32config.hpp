/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "config.hpp"
#include "win32/registry.hpp"

class Win32ConfigReader : public ConfigReader {
public:
  Win32ConfigReader() {}
  virtual ~Win32ConfigReader() {}

protected:
  virtual std::string getStringValue(const std::string &key,
                                     const std::string &defValue);
  virtual long getLongValue(const std::string &key, const long &defValue);
  virtual bool getBoolValue(const std::string &key, const bool &defValue);

private:
  static std::string baseKey;
  RegistryManager m;
};
