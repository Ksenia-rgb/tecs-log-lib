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

  if (std::error_code code = tecslog::init(); code)
  {
    std::cerr << "Incorrect argument for lib initialization: " << code << '\n';
  }

  std::unordered_map< std::string, std::function< std::error_code(std::istream&, std::ostream&) > > command_map;
  command_map["get-level"] = demo::getLevelCommand;
  command_map["get-file"] = demo::getFileCommand;
  command_map["get-fallback"] = demo::getFallbackCommand;
  command_map["set-level"] = demo::setLevelCommand;
  command_map["set-file"] = demo::setFileCommand;
  command_map["set-fallback"] = demo::setFallbackCommand;
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
