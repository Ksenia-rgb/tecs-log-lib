#ifndef IO_COMMANDS_HPP
#define IO_COMMANDS_HPP

#include <iostream>

namespace demo
{
  std::error_code getLevelCommand(std::istream& in, std::ostream& out);
  std::error_code getFileCommand(std::istream& in, std::ostream& out);
  std::error_code getFallbackCommand(std::istream& in, std::ostream& out);

  std::error_code setLevelCommand(std::istream& in, std::ostream& out);
  std::error_code setFileCommand(std::istream& in, std::ostream& out);
  std::error_code setFallbackCommand(std::istream& in, std::ostream& out);

  std::error_code logCommand(std::istream& in, std::ostream& out);
  std::error_code helpCommand(std::istream& in, std::ostream& out);
}

#endif
