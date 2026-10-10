#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <deque>
#include <fstream>
#include <optional>
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <tecslog/types.hpp>

namespace tecslog
{
  namespace detail
  {
    class LogWriter
    {
    public:
      explicit LogWriter(const std::string& filename);
      ~LogWriter();

      std::error_code setFile(const std::string& filename);
      std::string getFile();
      std::error_code push(std::string message);
      void reset();
    private:
      std::deque< std::string > log_queue_;
      std::mutex mutex_queue_;
      std::condition_variable cond_;

      std::string filename_;
      std::ofstream out_;
      std::ofstream out_default_;
      std::mutex mutex_file_;
      std::error_code file_state_;

      std::atomic< bool > stopped_;
      std::thread writer_;

      void write();

      void tryFileOpen();
      std::ofstream* chooseOut();
      void closeFiles();
    };
  }
  class Logger
  {
  public:
    static Logger& instance();

    std::error_code setFile(const std::string& filename);
    std::error_code setLevel(Level level);
    std::error_code setLevel(const std::string& str_level);
    Level getLevel();
    std::string getLogFile();
    void reset();

    std::error_code info(const std::string& message);
    std::error_code warning(const std::string& message);
    std::error_code error(const std::string& message);
    std::error_code log(Level level, const std::string& message);
    std::error_code log(const std::string& str_level, const std::string& message);

    static std::string levelToStr(Level level);
    static std::optional< Level > strToLevel(const std::string& level);
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
