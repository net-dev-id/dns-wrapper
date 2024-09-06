#pragma once

#include <string>

class UnixUtil {
public:
  static void Die(const std::string &baseMessage, const int exitCode);
};
