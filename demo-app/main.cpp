#include <iomanip>
#include <iostream>
#include <functional>
#include <system_error>
#include <thread>
#include <unordered_map>

#include <tecslog/tecslog.hpp>

#include "io-commands.hpp"

int main(int argc, char** argv)
{
  using namespace std::placeholders;

  if (argc != 3)
  {
    std::cerr << "Incorrect programm argument count\n";
    return 1;
  }
  if (std::error_code code = tecslog::init(argv[1], argv[2], true); code)
  {
    std::cerr << "Incorrect argument for lib initialization: " << code << '\n';
  }

  std::unordered_map< std::string, std::function< std::error_code(std::istream&, std::ostream&) > > command_map;
  command_map["default"] = demo::defaultCommand;
  command_map["log"] = demo::logCommand;
  command_map["help"] = demo::helpCommand;

  std::istream& in = std::cin;
  std::ostream& out = std::cout;

  std::string command;
  while (in >> command)
  {
    auto command_iter = command_map.find(command);
    if (command_iter != command_map.end())
    {
      std::error_code code = command_iter->second(in, out);
      if (code)
      {
        std::cerr << code.message();
      }
      out << '\n';
    }
    else
    {
      std::cerr << "Invalid command\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    }
  }
}
