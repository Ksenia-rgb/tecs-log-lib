#include "file-writer.hpp"

#include <iterator>

tecslog::detail::FileOutInfo::FileOutInfo(const std::string& filename, bool open_now):
  filename_(filename),
  state_()
{
  if (open_now)
  {
    open();
  }
}

tecslog::detail::FileOutInfo::~FileOutInfo()
{
  clearState();
  close();
}

std::ofstream& tecslog::detail::FileOutInfo::getFout()
{
  return fout_;
}

std::string tecslog::detail::FileOutInfo::getFile()
{
  return filename_;
}

std::error_code tecslog::detail::FileOutInfo::getState()
{
  return state_;
}

std::error_code tecslog::detail::FileOutInfo::changeFile(const std::string& filename)
{
  filename_ = filename;
  close();
  open();
  return state_;
}

void tecslog::detail::FileOutInfo::open()
{
  if (!fout_.is_open())
  {
    fout_.clear();
    fout_.open(filename_, std::ios::app);
    state_ = !fout_.is_open() ? std::make_error_code(std::errc::no_such_file_or_directory) : std::error_code{};
  }
}

void tecslog::detail::FileOutInfo::close()
{
  if (fout_.is_open())
  {
    fout_.flush();
    fout_.close();
  }
}

void tecslog::detail::FileOutInfo::flush()
{
  if (fout_.is_open())
  {
    fout_.flush();
  }
}

void tecslog::detail::FileOutInfo::clearState()
{
  state_ = std::error_code{};
}

tecslog::detail::LogWriter::LogWriter(const std::string& filename, bool fallback):
  filename_(filename),
  fout_default_(default_filename, fallback),
  fout_(filename, false),
  fallback_(fallback),
  stopped_(false)
{
  if (filename == default_filename)
  {
    fout_default_.open();
  }
  else
  {
    fout_.open();
  }
  writer_ = std::thread(&LogWriter::write, this);
}

tecslog::detail::LogWriter::~LogWriter()
{
  stopped_.store(true);
  cond_queue_.notify_all();

  if (writer_.joinable())
  {
    writer_.join();
  }
}

std::string tecslog::detail::LogWriter::getFile()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  return chooseFout().getFile();
}

bool tecslog::detail::LogWriter::getFallback()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  return fallback_;
}

std::error_code tecslog::detail::LogWriter::getState()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  return chooseFout().getState();
}

std::error_code tecslog::detail::LogWriter::setFile(const std::string& filename)
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  std::error_code return_code;
  if (filename != default_filename)
  {
    fout_.changeFile(filename);
    return_code = fout_.getState();
  }
  else
  {
    fout_.clearState();
    fout_.close();
    return_code = fout_default_.getState();
  }
  filename_ = filename;
  return return_code;
}

std::error_code tecslog::detail::LogWriter::setFallback(bool fallback)
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  fallback_ = fallback;
  if (fallback)
  {
    fout_default_.open();
  }
  return {};
}

std::error_code tecslog::detail::LogWriter::push(std::string message)
{
  FileOutInfo* fout = nullptr;
  {
    std::lock_guard< std::mutex > lock(mutex_file_);
    fout = &chooseFout();
  }
  {
    std::lock_guard< std::mutex > lock(mutex_queue_);
    log_queue_.push_back({message, fout});
  }
  cond_queue_.notify_one();
  return fout->getState();
}

void tecslog::detail::LogWriter::reset()
{
  std::lock_guard< std::mutex > lock(mutex_file_);
  fout_.clearState();
  fout_.close();
  fout_default_.changeFile(default_filename);
  filename_ = default_filename;
  fallback_ = default_fallback;
}

void tecslog::detail::LogWriter::write()
{
  while (!stopped_)
  {
    std::unique_lock< std::mutex > lock(mutex_queue_);
    cond_queue_.wait(lock, [this]{ return stopped_.load() || !log_queue_.empty(); });
    if (log_queue_.empty())
    {
      break;
    }

    std::deque< LogEntry > copy = std::move(log_queue_);
    lock.unlock();

    std::lock_guard< std::mutex > lock_file(mutex_file_);
    for (LogEntry& entry: copy)
    {
      if (!entry.fout->getState())
      {
        entry.fout->getFout() << entry.message << '\n';
      }
    }
    fout_.flush();
    fout_default_.flush();
  }

  std::deque< LogEntry > tail;
  {
    std::lock_guard< std::mutex > lock(mutex_queue_);
    std::swap(log_queue_, tail);
  }

  std::lock_guard< std::mutex > lock_file(mutex_file_);
  for (LogEntry& entry: tail)
  {
    entry.fout->getFout() << entry.message << '\n';
  }
  fout_.flush();
  fout_default_.flush();
}

tecslog::detail::FileOutInfo& tecslog::detail::LogWriter::chooseFout()
{
  if (fallback_ && fout_.getState())
  {
    return fout_default_;
  }
  if (filename_ == default_filename)
  {
    return fout_default_;
  }
  return fout_;
}
