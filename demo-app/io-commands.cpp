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

std::error_code demo::defaultCommand(std::istream& in, std::ostream& out)
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
    << "\t1 default <level>\t= Specify default log level, logs with a lower level will not be recorded\n"
    << "\t3 log <message> <level>\t= Log message with level\n";

  out << "Log levels:\n";
  tecslog::printPossibleLevels(out);

  return {};
}
