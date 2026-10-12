#include "io-commands.hpp"

#include <iomanip>
#include <sstream>

#include <tecslog/tecslog.hpp>

namespace
{
  std::error_code readWordTillEnd(std::istream& in, std::string& str)
  {
    std::string buffer;
    std::getline(in, buffer);
    std::stringstream sin(buffer);

    if (!(sin >> str))
    {
      return std::make_error_code(std::errc::io_error);
    }
    return {};
  }
}

std::error_code demo::getLevelCommand(std::istream& in, std::ostream& out)
{
  out << "OK: current level is specified on " << tecslog::getLevel();
  return {};
}

std::error_code demo::getFileCommand(std::istream& in, std::ostream& out)
{
  out << "OK: current file is specified on " << tecslog::getFile();
  return {};
}

std::error_code demo::getFallbackCommand(std::istream& in, std::ostream& out)
{
  out << "OK: current fallback is specified on " << std::boolalpha << tecslog::getFallback();
  return {};
}

std::error_code demo::setLevelCommand(std::istream& in, std::ostream& out)
{
  std::string str_level;
  std::error_code code = readWordTillEnd(in, str_level);
  if (code)
  {
    return code;
  }

  code = tecslog::setLevel(str_level);
  if (code)
  {
    return code;
  }
  out << "OK: default level is specified on " << str_level;
  return {};
}

std::error_code demo::setFileCommand(std::istream& in, std::ostream& out)
{
  std::string filename;
  std::error_code code = readWordTillEnd(in, filename);
  if (code)
  {
    return code;
  }

  code = tecslog::setFile(filename);
  if (code)
  {
    return code;
  }
  out << "OK: file is specified on " << filename;
  return {};
}

std::error_code demo::setFallbackCommand(std::istream& in, std::ostream& out)
{
  std::string fallback;
  std::error_code code = readWordTillEnd(in, fallback);
  if (code)
  {
    return code;
  }

  bool fallback_bool = false;
  if (fallback == "y" || fallback == "yes" || fallback == "Y" || fallback == "Yes")
  {
    fallback_bool = true;
  }
  code = tecslog::setFallback(fallback_bool);
  if (code)
  {
    return code;
  }
  out << "OK: fallback is specified on " << std::boolalpha << fallback_bool;
  return {};
}

std::error_code demo::logCommand(std::istream& in, std::ostream& out)
{
  std::string message;
  in >> std::quoted(message);
  if (!in)
  {
    return std::make_error_code(std::errc::io_error);
  }

  std::string str_level;
  std::error_code code = readWordTillEnd(in, str_level);
  if (code)
  {
    return code;
  }

  code = tecslog::log(str_level, message);
  if (code)
  {
    return code;
  }
  out << "OK: log with message: \"" << message << "\" and level: " << str_level;
  return {};
}

std::error_code demo::helpCommand(std::istream& in, std::ostream& out)
{
  out << "Commands:\n"
    << "1 default <level>\t= Specify default log level, logs with a lower level will not be recorded\n"
    << "2 log <message> <level>\t= Log message with level\n";

  out << "Log levels:\n";
  tecslog::printLevels(out);

  return {};
}
