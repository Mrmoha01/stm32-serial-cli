/*
 * A very small unit-test helper, so the project needs no external test library.
 * Each test file is its own program: it runs its tests and exits with 0 on
 * success, 1 if any check failed.
 */
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(condition)                                                       \
    do {                                                                       \
        g_checks++;                                                            \
        if (!(condition)) {                                                    \
            g_failures++;                                                      \
            printf("    FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);    \
        }                                                                      \
    } while (0)

#define CHECK_STR_EQ(actual, expected)                                         \
    do {                                                                       \
        const char *a_ = (actual);                                             \
        const char *e_ = (expected);                                           \
        g_checks++;                                                            \
        if (strcmp(a_, e_) != 0) {                                             \
            g_failures++;                                                      \
            printf("    FAIL %s:%d: strings differ\n      expected: \"%s\"\n"  \
                   "      actual:   \"%s\"\n", __FILE__, __LINE__, e_, a_);    \
        }                                                                      \
    } while (0)

#define CHECK_CONTAINS(text, needle)                                           \
    do {                                                                       \
        const char *t_ = (text);                                               \
        const char *n_ = (needle);                                             \
        g_checks++;                                                            \
        if (strstr(t_, n_) == NULL) {                                          \
            g_failures++;                                                      \
            printf("    FAIL %s:%d: output does not contain \"%s\"\n"          \
                   "      output: \"%s\"\n", __FILE__, __LINE__, n_, t_);      \
        }                                                                      \
    } while (0)

#define RUN_TEST(test)                                                         \
    do {                                                                       \
        printf("  %s\n", #test);                                               \
        test();                                                                \
    } while (0)

/* Print the result and return the process exit code. */
static int test_summary(void)
{
    printf("  -> %d checks, %d failed\n", g_checks, g_failures);
    return (g_failures == 0) ? 0 : 1;
}

#endif /* TEST_FRAMEWORK_H */
