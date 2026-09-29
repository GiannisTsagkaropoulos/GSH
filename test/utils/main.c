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

int test_create_fullpath()
{
  char target[PATH_MAX];
  char cwd[PATH_MAX];
  char arg[PATH_MAX];
  strncpy(target, "\0", sizeof(target));
  strncpy(cwd, "\0", sizeof(cwd));
  strncpy(arg, "\0", sizeof(arg));

  // 1. Home Directory path
  const char *home_env = getenv("HOME");
  strcpy(cwd, "/asdfasdfas");
  strcpy(arg, "~");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ(home_env, target, "Handles home directory correctly");
  strncpy(target, "\0", sizeof(target));

  // 2. Absolute path
  strcpy(cwd, "/home/user");
  strcpy(arg, "/usr/lib");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ("/usr/lib", target, "Handles absolute paths correctly");
  strncpy(target, "\0", sizeof(target));

  // 3. Relative path starting with alnum
  strcpy(cwd, "/home/user");
  strcpy(arg, "docs");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ("/home/user/docs", target, "alnum should append to cwd");
  strncpy(target, "\0", sizeof(target));

  // 4. Relative path starting with hidden directory
  strcpy(cwd, "/home/user");
  strcpy(arg, ".git");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ("/home/user/.git", target, ".git should append to cwd");
  strncpy(target, "\0", sizeof(target));

  // 5. Relative path starting with ./
  strcpy(cwd, "/home/user");
  strcpy(arg, "./git");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ("/home/user/git", target, "handles correctly path starting with ./");
  strncpy(target, "\0", sizeof(target));

  // 6. Relative path with multiple './'
  strcpy(cwd, "/home/user");
  strcpy(arg, "./././././same-level");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ("/home/user/same-level", target, "handles correctly path with consecutive './' ");
  strncpy(target, "\0", sizeof(target));

  // 7. Going back with ..
  strcpy(cwd, "/home/user/docs");
  strcpy(arg, "../downloads");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ(target, "/home/user/downloads", ".. should go up one directory");
  strncpy(target, "\0", sizeof(target));

  // 8. Mix of '..' and './'
  strcpy(cwd, "/var/log/log2/nginx");
  strcpy(arg, "../../././../www/html");
  create_fullpath(target, cwd, arg);
  ASSERT_STR_EQ(target, "/var/www/html", "Multiple .. should work");
  strncpy(target, "\0", sizeof(target));

  return 0;
}

int main()
{
  printf("--- Running Utils Tests ---\n");

  RUN_TEST(test_dir_exists);
  RUN_TEST(test_is_exec);
  RUN_TEST(test_handle_home_dir);
  RUN_TEST(test_create_fullpath);

  printf("---------------------------\n");
  printf("Tests run: %d\n", tests_run);
  printf("Failures:  %d\n", tests_failed);

  return tests_failed > 0 ? 1 : 0;
}