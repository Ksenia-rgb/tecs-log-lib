#include <tecslog/tecslog.hpp>

#include "test-runner.hpp"

int main()
{
  test::TestFileGuard guard(tecslog::getDefaultFile());
  return test::runAllTests();
}
