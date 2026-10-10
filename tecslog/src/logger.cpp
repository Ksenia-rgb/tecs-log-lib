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

tecslog::detail::LogWriter::LogWriter(const std::string& filename):
  out_default_(default_filename),
  stopped_(false)
{
  setFile(filename);
  writer_ = std::thread(&LogWriter::write, this);
}

tecslog::detail::LogWriter::~LogWriter()
{
  stopped_.store(true);
  cond_.notify_all();

  if (writer_.joinable())
  {
    writer_.join();
  }
  closeFiles();
}

std::error_code tecslog::detail::LogWriter::setFile(const std::string& filename)
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  filename_ = filename;
  tryFileOpen();
  return file_state_;
}

std::string tecslog::detail::LogWriter::getFile()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  return filename_;
}

std::error_code tecslog::detail::LogWriter::push(std::string message)
{
  {
    std::lock_guard< std::mutex > lock(mutex_queue_);
    log_queue_.push_back(std::move(message));
  }
  cond_.notify_one();

  std::lock_guard< std::mutex > lock(mutex_file_);
  return file_state_;
}

void tecslog::detail::LogWriter::reset()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  closeFiles();
  out_default_.open(default_filename);
  filename_ = default_filename;
  file_state_ = {};
}

void tecslog::detail::LogWriter::write()
{
  while (!stopped_)
  {
    std::unique_lock< std::mutex > lock(mutex_queue_);
    cond_.wait(lock, [this]{ return stopped_.load() || !log_queue_.empty(); });
    if (log_queue_.empty())
    {
      break;
    }

    std::deque< std::string > copy = std::move(log_queue_);
    lock.unlock();

    std::lock_guard< std::mutex > lock_file(mutex_file_);
    std::ostream* fout = chooseOut();
    std::copy(
      copy.begin(),
      copy.end(),
      std::ostream_iterator< std::string >(*fout, "\n")
    );
    fout->flush();
  }

  std::lock_guard< std::mutex > lock_file(mutex_file_);
  std::ostream* fout = chooseOut();
  std::copy(
    log_queue_.begin(),
    log_queue_.end(),
    std::ostream_iterator< std::string >(*fout, "\n")
  );
  fout->flush();
}

void tecslog::detail::LogWriter::tryFileOpen()
{
  if (out_.is_open() && filename_ != default_filename)
  {
    out_.flush();
    out_.close();
  }
  if (filename_ == default_filename)
  {
    file_state_ = {};
    return;
  }
  out_.open(filename_, std::ios::app);
  file_state_ = !out_.is_open() ? std::make_error_code(std::errc::no_such_file_or_directory) : std::error_code{};
  return;
}

std::ofstream* tecslog::detail::LogWriter::chooseOut()
{
  if (file_state_ || filename_ == default_filename)
  {
    return &out_default_;
  }
  return &out_;
}

void tecslog::detail::LogWriter::closeFiles()
{
  if (out_.is_open())
  {
    out_.flush();
    out_.close();
  }
  if (out_default_.is_open())
  {
    out_default_.flush();
    out_default_.close();
  }
}

tecslog::Logger::Logger():
  writer_(default_filename),
  level_(default_level)
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
  setLevel(default_level);
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
  std::string formatted = strNowTime() + " [" + str_level + "] " + message;
  return writer_.push(std::move(formatted));
}

std::string tecslog::Logger::levelToStr(Level level)
{
  return str_level[level];
}

std::optional< tecslog::Level > tecslog::Logger::strToLevel(const std::string& level)
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
