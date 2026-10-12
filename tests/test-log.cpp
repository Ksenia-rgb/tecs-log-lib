#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

#include <tecslog/tecslog.hpp>

#include "test-runner.hpp"

TEST_CASE(TecslogCorrectDefaultLogTest)
{
  tecslog::reset();

  tecslog::init();
  std::error_code code_info = tecslog::info("Info default mess");
  std::error_code code_warn = tecslog::warning("Warn default mess");
  std::error_code code_error = tecslog::error("Error default mess");

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool info_found = sout.str().find("[INFO] Info default mess") != std::string::npos;
  bool warn_found = sout.str().find("[WARNING] Warn default mess") != std::string::npos;
  bool error_found = sout.str().find("[ERROR] Error default mess") != std::string::npos;

  return !code_info && !code_warn && !code_error && info_found && warn_found && error_found;
}

TEST_CASE(TecslogCorrectFileLogTest)
{
  tecslog::reset();

  std::string filename = "test.log";
  test::TestFileGuard guard(filename);

  tecslog::init();
  std::error_code code_file = tecslog::setFile(filename);
  std::error_code code_info = tecslog::info("Info mess");
  std::error_code code_warn = tecslog::warning("Warn mess");
  std::error_code code_error = tecslog::error("Error mess");

  std::ifstream fout(filename);
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool info_found = sout.str().find("[INFO] Info mess") != std::string::npos;
  bool warn_found = sout.str().find("[WARNING] Warn mess") != std::string::npos;
  bool error_found = sout.str().find("[ERROR] Error mess") != std::string::npos;

  return !code_file && !code_info && !code_warn && !code_error && info_found && warn_found && error_found;
}

TEST_CASE(TecslogLogWithDifferentLevelsTest)
{
  tecslog::reset();

  std::string filename = "test.log";
  test::TestFileGuard guard(filename);

  tecslog::init();
  tecslog::setFile(filename);

  std::error_code code_info1 = tecslog::info("First info mess");

  tecslog::setLevel(tecslog::Level::WARNING);
  std::error_code code_info2 = tecslog::info("Second info mess");

  tecslog::setLevel(tecslog::Level::ERROR);
  std::error_code code_warn = tecslog::warning("First warn mess");
  std::error_code code_error = tecslog::error("First error mess");

  std::ifstream fout(filename);
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool info_found1 = sout.str().find("[INFO] First info mess") != std::string::npos;
  bool info_found2 = sout.str().find("[INFO] Second info mess") != std::string::npos;
  bool warn_found = sout.str().find("[WARNING] First warn mess") != std::string::npos;
  bool error_found = sout.str().find("[ERROR] First error mess") != std::string::npos;

  return !code_info1 && !code_info2 && !code_warn && !code_error &&
    info_found1 && !info_found2 && !warn_found && error_found;
}

TEST_CASE(TecslogLogWithCorrectStrLevelTest)
{
  tecslog::reset();

  std::error_code code_init = tecslog::init();
  std::error_code code = tecslog::log("INFO", "Info str mess");
  code = tecslog::log("WARNING", "Warn str mess");
  code = tecslog::log("ERROR", "Error str mess");

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool info_found = sout.str().find("[INFO] Info str mess") != std::string::npos;
  bool warn_found = sout.str().find("[WARNING] Warn str mess") != std::string::npos;
  bool error_found = sout.str().find("[ERROR] Error str mess") != std::string::npos;

  return code_init.value() == 0 && code.value() == 0 && info_found && warn_found && error_found;
}

TEST_CASE(TecslogLogWithInorrectStrLevelTest)
{
  tecslog::reset();

  std::string filename = "test.log";
  test::TestFileGuard guard(filename);

  tecslog::init();
  tecslog::setFile(filename);
  std::error_code code1 = tecslog::log("BOOL", "Info mess");
  std::error_code code2 = tecslog::log("INCORRECT", "Warn mess");

  std::ifstream fout(filename);
  std::ostringstream sout;
  sout << fout.rdbuf();

  return code1 && code2 && sout.str().empty();
}

TEST_CASE(TecslogLogWithIncorrectFileWithoutFallbackTest)
{
  tecslog::reset();

  std::string filename_unreal = "logs/unreal-directory/test.log";

  tecslog::init();
  std::error_code code_file = tecslog::setFile(filename_unreal);
  std::error_code code_log = tecslog::error("New error mess");
  bool file = tecslog::getFile() == filename_unreal;

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  return code_file && code_log && file && sout.str().find("[ERROR] New error mess") == std::string::npos;
}

TEST_CASE(TecslogLogWithIncorrectFileWithFallbackTest)
{
  tecslog::reset();

  std::string filename_unreal = "logs/unreal-directory/test.log";

  tecslog::init();
  tecslog::setFallback(true);
  std::error_code code_file = tecslog::setFile(filename_unreal);
  std::error_code code_log = tecslog::error("Error incorrect file fallback mess");
  bool file = tecslog::getFile() == tecslog::getDefaultFile();

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  return code_file && !code_log && file && sout.str().find("[ERROR] Error incorrect file fallback mess") != std::string::npos;
}

TEST_CASE(TecslogLogSetDefaultFileTest)
{
  tecslog::reset();

  tecslog::init();

  std::error_code code_file1 = tecslog::setFile(tecslog::getDefaultFile());
  std::error_code code_info = tecslog::info("Default file info mess");
  bool file1 = tecslog::getFile() == tecslog::getDefaultFile();

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool info_found = sout.str().find("[INFO] Default file info mess") != std::string::npos;

  return !code_file1 && !code_info && file1 && info_found;
}

TEST_CASE(TecslogLogSetDefaultFileWithFallbackTest)
{
  tecslog::reset();

  tecslog::init();
  std::error_code code_file1 = tecslog::setFile(tecslog::getDefaultFile());
  bool file1 = tecslog::getFile() == tecslog::getDefaultFile();

  tecslog::setFallback(true);
  std::error_code code_warn = tecslog::warning("Default file fallback warn mess");
  bool file2 = tecslog::getFile() == tecslog::getDefaultFile();

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool warn_found = sout.str().find("[WARNING] Default file fallback warn mess") != std::string::npos;

  return !code_file1 && file1 && !code_warn && file2 && warn_found;
}

TEST_CASE(TecslogLogSetDefaultAndIncorrectFileTest)
{
  tecslog::reset();

  std::string filename_unreal = "logs/unreal-directory/test.log";

  tecslog::init();
  std::error_code code_file1 = tecslog::setFile(tecslog::getDefaultFile());
  bool file1 = tecslog::getFile() == tecslog::getDefaultFile();

  tecslog::setFallback(true);
  tecslog::setFile(filename_unreal);
  std::error_code code_warn = tecslog::warning("Default file with fallback warn mess");
  bool file2 = tecslog::getFile() == tecslog::getDefaultFile();

  tecslog::setFallback(false);
  std::error_code code_error = tecslog::error("Default file without fallback error mess");
  bool file3 = tecslog::getFile() == filename_unreal;

  std::ifstream fout(tecslog::getDefaultFile());
  std::ostringstream sout;
  sout << fout.rdbuf();

  bool warn_found = sout.str().find("[WARNING] Default file with fallback warn mess") != std::string::npos;
  bool error_found = sout.str().find("[ERROR] Default file without fallback error mess") != std::string::npos;

  return !code_file1 && file1 && !code_warn && file2 && code_error && file3 && warn_found && !error_found;
}
