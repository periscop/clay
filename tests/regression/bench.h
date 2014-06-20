#include <stdio.h>
#include <stdbool.h>

#define TEST(name) \
void test__ ## name ## __function()

#define TEST1(name, p1) \
void test__ ## name ## __function(p1)

#define EXPECT_TRUE(condition, message) \
  do { \
  if (!(condition)) { \
    fprintf(stderr, "  [Error] %s\n" \
                    "  | %s expected to be true.\n", message, #condition); \
    test__result = 1; \
  } \
  } while (0)

#define ASSERT_TRUE(condition, message) \
  do { \
  if (!(condition)) { \
    fprintf(stderr, "  [Fatal] %s\n" \
                    "  | %s expected to be true.\n", message, #condition); \
    test__result = 1; \
    return; \
  } \
  } while (0)

#define TEST_BENCH \
  static int test__result; \
  static int test__runs; \
  static int test__failures;

#define TEST_RESULT test__result

#define TEST_SUMMARY \
  do { \
    if (test__failures == 0) { \
      fprintf(stderr, "\n==============================" \
                      "\n==     PASSED ALL TESTS     ==" \
                      "\n==============================\n\n"); \
    } else { \
      fprintf(stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" \
                      "\n!! FAILED %3d of %3d TESTS  !!" \
                      "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n", \
              test__failures, \
              test__runs); \
    } \
  } while (0)

#define RUN_TEST(name) \
  do { \
    test__result = 0; \
    test__runs++; \
    test__ ## name ## __function(); \
    if (test__result != 0) { \
      test__failures++; \
      fprintf(stderr, "[FAIL] %s", " " #name "\n"); \
    } else { \
      fprintf(stdout, "[pass] %s\n", #name); \
    } \
  } while (0)

#define RUN_TEST1(name, p1) \
  do { \
    test__result = 0; \
    test__ ## name ## __function((p1)); \
    test__runs++; \
    if (test__result != 0) { \
      test__failures++; \
      fprintf(stderr, "[FAIL] %s(%s)\n", #name, #p1); \
    } else { \
      fprintf(stdout, "[pass] %s\n", #name); \
    } \
  } while (0)
