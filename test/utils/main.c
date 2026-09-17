#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "../../utils.h"
#include "../tests.h"

int tests_run = 0;
int tests_failed = 0;

int test_dir_exists()
{
  ASSERT(dir_exists(".") == 1, "Current directory should exist");
  ASSERT(dir_exists("..") == 1, "Parent directory should exist");
  ASSERT(dir_exists("/this_dir_does_not_exist_12345") == 0, "Fake directory should not exist");

  return 0;
}

int test_is_exec()
{
  char fullpath[PATH_MAX];

  // Assumes 'ls' is in path
  ASSERT(is_exec(fullpath, "ls") == 1, "'ls' should be found in PATH");
  ASSERT(strlen(fullpath) > 0, "fullpath should be populated");

  ASSERT(is_exec(fullpath, "random_cmd") == 0, "Random command should not be executable");
  return 0;
}

int test_handle_home_dir()
{
  // Mock the HOME environment variable
  const char *home_env = getenv("HOME");
  setenv("HOME", "/mock/home", 1);

  char target[PATH_MAX];
  ASSERT(handle_home_dir(target, "~") == 0, "Should handle ~ successfully");
  ASSERT_STR_EQ(target, "/mock/home", "~ should expand to $HOME");

  ASSERT(handle_home_dir(target, "~/") == 0, "Should handle ~/ successfully");
  ASSERT_STR_EQ(target, "/mock/home", "~/ should expand to $HOME/");

  setenv("HOME", home_env, 1);
  return 0;
}

int main()
{
  printf("--- Running Utils Tests ---\n");

  RUN_TEST(test_dir_exists);
  RUN_TEST(test_is_exec);
  RUN_TEST(test_handle_home_dir);

  printf("---------------------------\n");
  printf("Tests run: %d\n", tests_run);
  printf("Failures:  %d\n", tests_failed);

  return tests_failed > 0 ? 1 : 0;
}