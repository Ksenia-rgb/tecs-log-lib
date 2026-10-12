#include <tecslog/tecslog.hpp>

#include "test-runner.hpp"

TEST_CASE(TecslogDefaultInitTest)
{
  tecslog::reset();

  std::error_code code = tecslog::init();
  bool level = tecslog::getLevel() == tecslog::getDefaultLevel();
  bool file = tecslog::getFile() == tecslog::getDefaultFile();
  bool fallback = tecslog::getFallback() == tecslog::getDefaultFallback();

  return !code && file && level && fallback;
}

TEST_CASE(TecslogSetFileLevelFallbackTest)
{
  tecslog::reset();

  std::string filename = "test.log";
  test::TestFileGuard guard(filename);

  tecslog::init();
  std::error_code code_level = tecslog::setLevel(tecslog::WARNING);
  std::error_code code_file = tecslog::setFile(filename);
  std::error_code code_fallback = tecslog::setFallback(true);
  bool level = tecslog::getLevel() == tecslog::WARNING;
  bool file = tecslog::getFile() == filename;
  bool fallback = tecslog::getFallback() == true;

  return !code_level && !code_file && !code_fallback && file && level && fallback;
}

TEST_CASE(TecslogSetStrLevelTest)
{
  tecslog::reset();

  tecslog::init();
  std::error_code code1 = tecslog::setLevel("WARNING");
  bool level1 = tecslog::getLevel() == tecslog::WARNING;

  std::error_code code2 = tecslog::setLevel("INCORRECT");
  bool level2 = tecslog::getLevel() == tecslog::getDefaultLevel();

  return !code1 && level1 && code2 && level2;
}

TEST_CASE(TecslogSetIncorrectFileTest)
{
  tecslog::reset();

  std::string filename1 = "logs/unreal-directory/test.log";
  std::string filename2 = "logs/imagine-directory/test.log";

  tecslog::init();
  std::error_code code_file1 = tecslog::setFile(filename1);
  bool file1 = tecslog::getFile() == filename1;

  tecslog::setFallback(true);
  bool file2 = tecslog::getFile() == tecslog::getDefaultFile();

  std::error_code code_file2 = tecslog::setFile(filename2);
  bool file3 = tecslog::getFile() == tecslog::getDefaultFile();

  return code_file1 && file1 && file2 && code_file2 && file3;
}
