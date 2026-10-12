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
  const Level default_level = Level::MIN;
  const bool default_fallback = false;
}

#endif
