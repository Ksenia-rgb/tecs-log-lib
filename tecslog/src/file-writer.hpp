#include <fstream>
#include <string>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <tecslog/types.hpp>

namespace tecslog
{
  namespace detail
  {
    class FileOutInfo
    {
    public:
      FileOutInfo(const std::string& filename, bool open_now);
      ~FileOutInfo();
      
      std::ofstream& getFout();
      std::string getFile();
      std::error_code getState();

      std::error_code changeFile(const std::string& filename);
      void open();
      void close();
      void clear();
    private:
      std::ofstream fout_;
      std::string filename_;
      std::error_code state_;
    };

    class LogWriter
    {
    public:
      explicit LogWriter(const std::string& filename, bool fallback);
      ~LogWriter();

      std::string getFile();
      bool getFallback();
      std::error_code getState();

      std::error_code setFile(const std::string& filename);
      std::error_code setFallback(bool fallback);

      std::error_code push(std::string message);
      void reset();
    private:
      std::deque< std::string > log_queue_;
      std::mutex mutex_queue_;
      std::condition_variable cond_queue_;

      std::string filename_;
      FileOutInfo fout_default_;
      FileOutInfo fout_;
      std::mutex mutex_file_;

      bool fallback_;

      std::atomic< bool > stopped_;
      std::thread writer_;

      void write();
      FileOutInfo& chooseFout();
    };
  }
}