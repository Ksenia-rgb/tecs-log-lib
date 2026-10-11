#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <optional>
#include <string>
#include <vector>
#include <atomic>

#include <tecslog/types.hpp>
#include "file-writer.hpp"

namespace tecslog
{
  namespace detail
  {
    static const std::vector< std::string > str_level = {"INFO", "WARNING", "ERROR"};
    std::string levelToStr(Level level);
    std::optional< Level > strToLevel(const std::string& level);
  }
  class Logger
  {
  public:
    static Logger& instance();

    Level getLevel();
    std::string getFile();
    bool getFallback();
    std::error_code getState();
    std::ostream& printLevels(std::ostream& out);

    std::error_code setFile(const std::string& filename);
    std::error_code setLevel(Level level);
    std::error_code setLevel(const std::string& str_level);
    std::error_code setFallback(bool fallback);
    
    void reset();

    std::error_code info(const std::string& message);
    std::error_code warning(const std::string& message);
    std::error_code error(const std::string& message);
    std::error_code log(Level level, const std::string& message);
    std::error_code log(const std::string& str_level, const std::string& message);

  private:
    detail::LogWriter writer_;
    std::atomic< Level > level_;

    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::error_code sendLog(const std::string& str_level, const std::string& message);
  };
}

#endif
