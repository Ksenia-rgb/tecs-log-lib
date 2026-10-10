#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

namespace tecslog
{
  enum Level
  {
    INFO = 0,
    WARNING,
    ERROR,
    MIN = INFO,
    MAX = ERROR
  };

  const std::string default_filename = "tecslog.log";
  const tecslog::Level default_level = tecslog::Level::MIN;
  const std::vector< std::string > str_level = {"INFO", "WARNING", "ERROR"};
}

#endif
