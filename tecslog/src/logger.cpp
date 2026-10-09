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
  std::string str_now_time()
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
  std::string strToupper(const std::string& str)
  {
    std::string upper;
    std::transform(str.begin(), str.end(), std::back_inserter(upper),
      [](unsigned char c){ return std::toupper(c); }
    );
    return upper;
  }
}

const std::string tecslog::detail::default_filename_ = "tecslog.log";
const tecslog::Level tecslog::detail::default_level_ = tecslog::Level::MIN;
const std::unordered_map< tecslog::Level, std::string > tecslog::detail::level_map = {
  {tecslog::Level::INFO, "INFO"},
  {tecslog::Level::WARNING, "WARNING"},
  {tecslog::Level::ERROR, "ERROR"}
};
const std::unordered_map< std::string, tecslog::Level > tecslog::detail::str_level_map = {
  {"INFO", tecslog::Level::INFO},
  {"WARNING", tecslog::Level::WARNING},
  {"ERROR", tecslog::Level::ERROR}
};

tecslog::detail::LogWriter::LogWriter(const std::string& filename):
  stopped_(false),
  writer_(std::thread(&LogWriter::write, this))
{
  setFile(filename);
}

tecslog::detail::LogWriter::~LogWriter()
{
  stopped_.store(true);
  cond_.notify_all();

  if (writer_.joinable())
  {
    writer_.join();
  }
  if (out_.is_open())
  {
    out_.flush();
    out_.close();
  }
}

std::error_code tecslog::detail::LogWriter::setFile(const std::string& filename)
{
  {
    std::lock_guard< std::mutex > lock(mutex_file_);
    config_filename_ = filename;
  }
  tryFileOpen();
  return file_state_;
}

std::string tecslog::detail::LogWriter::getFile()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  return config_filename_;
}

std::error_code tecslog::detail::LogWriter::push(std::string message)
{
  {
    std::lock_guard< std::mutex > lock(mutex_queue_);
    log_queue_.push_back(std::move(message));
  }
  cond_.notify_one();
  return file_state_;
}

void tecslog::detail::LogWriter::reset()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  config_filename_ = default_filename_;
  open_filename_ = "";
  if (out_.is_open())
  {
    out_.flush();
    out_.close();
  }
}

void tecslog::detail::LogWriter::tryFileOpen()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  if (out_.is_open() && config_filename_ == open_filename_)
  {
    file_state_ = {};
    return;
  }
  if (!out_.is_open() && config_filename_ == open_filename_)
  {
    file_state_ = std::make_error_code(std::errc::no_such_file_or_directory);
    return;
  }
  out_.close();
  out_.open(config_filename_, std::ios::app);
  open_filename_ = config_filename_;
  file_state_ = !out_.is_open() ? std::make_error_code(std::errc::no_such_file_or_directory) : std::error_code{};

  return;
}

void tecslog::detail::LogWriter::write()
{
  while (!stopped_)
  {
    std::unique_lock< std::mutex > lock(mutex_queue_);
    cond_.wait(lock, [this]{ return stopped_.load() || !log_queue_.empty(); });
    if (log_queue_.empty())
    {
      out_.flush();
      break;
    }

    std::deque< std::string > copy = std::move(log_queue_);
    lock.unlock();

    tryFileOpen();
    if (!file_state_)
    {
      std::copy(
        copy.begin(),
        copy.end(),
        std::ostream_iterator< std::string >(out_, "\n")
      );
    }
  }

  std::copy(
    log_queue_.begin(),
    log_queue_.end(),
    std::ostream_iterator< std::string >(out_, "\n")
  );
  out_.flush();
}

tecslog::Logger::Logger():
  writer_(detail::default_filename_),
  level_(detail::default_level_)
{}

tecslog::Logger& tecslog::Logger::instance()
{
  static Logger logger_instance;
  return logger_instance;
}

std::error_code tecslog::Logger::setFile(const std::string& filename)
{
  return writer_.setFile(filename);
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
  std::optional< Level > level = strToLevel(str_level);
  if (level.has_value())
  {
    return setLevel(level.value());
  }
  return std::make_error_code(std::errc::invalid_argument);
}

tecslog::Level tecslog::Logger::getLevel()
{
  return level_;
}

std::string tecslog::Logger::getLogFile()
{
  return writer_.getFile();
}

void tecslog::Logger::reset()
{
  writer_.reset();
  level_.store(detail::default_level_);
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
    return sendLog(levelToStr(level), message);
  }
  return {};
}

std::error_code tecslog::Logger::log(const std::string& str_level, const std::string& message)
{
  std::optional< Level > level = strToLevel(str_level);
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
  std::string formatted = str_now_time() + " [" + str_level + "] " + message;
  return writer_.push(std::move(formatted));
}

std::string tecslog::Logger::levelToStr(Level level)
{
  return detail::level_map.find(level)->second;
}

std::optional< tecslog::Level > tecslog::Logger::strToLevel(const std::string& level)
{
  std::string level_up = strToupper(level);

  auto level_iter = detail::str_level_map.find(level_up);
  if (level_iter != detail::str_level_map.end())
  {
    return level_iter->second;
  }
  return {};
}
