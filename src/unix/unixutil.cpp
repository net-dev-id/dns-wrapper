#include "unix/unixutil.hpp"
#include "log.hpp"

void UnixUtil::Die(const std::string &baseMessage, const int exitCode) {
  char *errMesssage = strerror(errno);

  LFATAL << baseMessage << ": " << errMesssage << std::endl;
  exit(exitCode);
}
