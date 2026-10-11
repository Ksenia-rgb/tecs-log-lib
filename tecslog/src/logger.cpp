#include "logger.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <optional>
#include <iterator>
#include <thread>
#include <iostream>

namespace
{
  std::string strNowTime();
  std::string strToUpper(const std::string& str);

  std::string strNowTime()
  {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm{};
    #ifdef _WIN32
      localtime_s(&tm, &now_c);
    #else
      localtime_r(&now_c, &tm);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d.%m.%Y %H:%M:%S");
    return oss.str();
  }
  std::string strToUpper(const std::string& str)
  {
    std::string upper;
    std::transform(str.begin(), str.end(), std::back_inserter(upper),
      [](unsigned char c){ return std::toupper(c); }
    );
    return upper;
  }
}

std::string tecslog::detail::levelToStr(Level level)
{
  return str_level[level];
}

std::optional< tecslog::Level > tecslog::detail::strToLevel(const std::string& level)
{
  std::string level_up = strToUpper(level);

  auto level_iter = std::find(str_level.begin(), str_level.end(), level_up);
  if (level_iter != str_level.end())
  {
    size_t index = std::distance(str_level.begin(), level_iter);
    return static_cast< Level >(index);
  }
  return {};
}

tecslog::Logger::Logger():
  writer_(default_filename, default_fallback),
  level_(default_level)
{}

tecslog::Logger& tecslog::Logger::instance()
{
  static Logger logger_instance;
  return logger_instance;
}

tecslog::Level tecslog::Logger::getLevel()
{
  return level_;
}

std::string tecslog::Logger::getFile()
{
  return writer_.getFile();
}

bool tecslog::Logger::getFallback()
{
  return writer_.getFallback();
}

std::ostream& tecslog::Logger::printLevels(std::ostream& out)
{
  out << 0 << ' ' << detail::levelToStr(Level::MIN);
  for (int i = static_cast< int >(Level::MIN) + 1; i <= static_cast< int >(Level::MAX); i++)
  {
    out << '\n' << i << ' ' << detail::levelToStr(static_cast< Level >(i));
  }
  return out;
}

std::error_code tecslog::Logger::setLevel(Level level)
{
  if (level_ != level)
  {
    level_.store(level);
  }
  return {};
}

std::error_code tecslog::Logger::setLevel(const std::string& str_level)
{
  std::optional< Level > level = detail::strToLevel(str_level);
  if (level.has_value())
  {
    return setLevel(level.value());
  }
  setLevel(default_level);
  return std::make_error_code(std::errc::invalid_argument);
}

std::error_code tecslog::Logger::setFile(const std::string& filename)
{
  return writer_.setFile(filename);
}

std::error_code tecslog::Logger::setFallback(bool fallback)
{
  return writer_.setFallback(fallback);
}

void tecslog::Logger::reset()
{
  writer_.reset();
  level_.store(default_level);
}

std::error_code tecslog::Logger::info(const std::string& message)
{
  return log(Level::INFO, message);
}

std::error_code tecslog::Logger::warning(const std::string& message)
{
  return log(Level::WARNING, message);
}

std::error_code tecslog::Logger::error(const std::string& message)
{
  return log(Level::ERROR, message);
}

std::error_code tecslog::Logger::log(Level level, const std::string& message)
{
  if (level >= level_)
  {
    return sendLog(detail::levelToStr(level), message);
  }
  return {};
}

std::error_code tecslog::Logger::log(const std::string& str_level, const std::string& message)
{
  std::optional< Level > level = detail::strToLevel(str_level);
  if (!level.has_value())
  {
    return std::make_error_code(std::errc::invalid_argument);
  }

  if (level.value() >= level_)
  {
    return sendLog(str_level, message);
  }
  return {};
}

std::error_code tecslog::Logger::sendLog(const std::string& str_level, const std::string& message)
{
  std::string formatted = strNowTime() + " [" + str_level + "] " + message;
  return writer_.push(std::move(formatted));
}
