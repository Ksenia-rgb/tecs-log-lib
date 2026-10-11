#ifndef TECSLOG_HPP
#define TECSLOG_HPP

#include <optional>
#include <string>
#include <system_error>

#include <tecslog/types.hpp>

namespace tecslog
{
  std::error_code init();

  Level getLevel();
  std::string getFile();
  bool getFallback();

  Level getDefaultLevel();
  std::string getDefaultFile();
  bool getDefaultFallback();

  std::ostream& printLevels(std::ostream& out);

  std::error_code setLevel(Level level);
  std::error_code setLevel(const std::string& level);
  std::error_code setFile(const std::string& filename);
  std::error_code setFallback(bool fallback);

  void reset();

  std::error_code info(const std::string& message);
  std::error_code warning(const std::string& message);
  std::error_code error(const std::string& message);

  std::error_code log(Level level, const std::string& message);
  std::error_code log(const std::string& level, const std::string& message);
}

#endif
