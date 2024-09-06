#pragma once

#include <stdexcept>
#include <string>

class Args {
public:
  enum ExitCode { NoExit, ExitWithNoError, ExitWithError, ExitWithUnlock };

private:
  Args() = default;
  ~Args() {};

  static Args *_args;

public:
  static ExitCode Init(int argc, char *argv[]);
  static Args *Get() {
    if (_args == nullptr) {
      throw new std::runtime_error("Arguments not initialized");
    }

    return _args;
  }

  std::string configFile;
};
