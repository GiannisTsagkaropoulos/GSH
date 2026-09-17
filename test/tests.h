#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

#define RUN_TEST(test_func)               \
  do                                      \
  {                                       \
    printf("Running %s... ", #test_func); \
    if (test_func() == 0)                 \
    {                                     \
      printf("\033[32mPASSED\033[0m\n");  \
    }                                     \
    else                                  \
    {                                     \
      printf("\033[31mFAILED\033[0m\n");  \
      tests_failed++;                     \
    }                                     \
    tests_run++;                          \
  } while (0)

#define ASSERT(test, message)                                             \
  do                                                                      \
  {                                                                       \
    if (!(test))                                                          \
    {                                                                     \
      fprintf(stderr, "[FAIL] %s:%d: %s\n", __FILE__, __LINE__, message); \
      return 1;                                                           \
    }                                                                     \
  } while (0)

#define ASSERT_STR_EQ(expected, actual, message)             \
  do                                                         \
  {                                                          \
    const char *test_e = (expected);                         \
    const char *test_a = (actual);                           \
    int test_ok = (test_e == NULL && test_a == NULL) ||      \
                  (test_e != NULL && test_a != NULL &&       \
                   strcmp(test_e, test_a) == 0);             \
    if (!test_ok)                                            \
    {                                                        \
      printf("[FAIL] %s:%d: %s (expected '%s', got '%s')\n", \
             __FILE__, __LINE__, message, expected, actual); \
      return 1;                                              \
    }                                                        \
  } while (0)

#endif /* TEST_H */