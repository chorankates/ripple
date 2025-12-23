#pragma once

// =============================================================================
// Minimal Test Framework - No External Dependencies
// =============================================================================

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Test result tracking (defined in test_main.c)
extern int g_tests_run;
extern int g_tests_passed;
extern int g_tests_failed;

// Colors for terminal output
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET  "\033[0m"

// =============================================================================
// Assertion Macros
// =============================================================================

#define TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf(COLOR_RED "  FAIL: %s:%d: %s" COLOR_RESET "\n", __FILE__, __LINE__, #condition); \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf(COLOR_RED "  FAIL: %s:%d: expected %d, got %d" COLOR_RESET "\n", \
               __FILE__, __LINE__, (int)(expected), (int)(actual)); \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        printf(COLOR_RED "  FAIL: %s:%d: expected \"%s\", got \"%s\"" COLOR_RESET "\n", \
               __FILE__, __LINE__, (expected), (actual)); \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_TRUE(condition)  TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))

// =============================================================================
// Test Registration & Running
// =============================================================================

typedef bool (*TestFunction)(void);

typedef struct {
    const char *name;
    TestFunction func;
} TestCase;

#define RUN_TEST(test_func) do { \
    g_tests_run++; \
    printf("  Running: %s... ", #test_func); \
    if (test_func()) { \
        g_tests_passed++; \
        printf(COLOR_GREEN "PASS" COLOR_RESET "\n"); \
    } else { \
        g_tests_failed++; \
    } \
} while(0)

#define TEST_SUITE_BEGIN(name) \
    printf("\n" COLOR_YELLOW "=== %s ===" COLOR_RESET "\n", name)

#define TEST_SUITE_END() \
    printf("\n")

// =============================================================================
// Summary
// =============================================================================

static inline void print_test_summary(void) {
    printf("\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", g_tests_run);
    printf(COLOR_GREEN "Tests passed: %d" COLOR_RESET "\n", g_tests_passed);
    if (g_tests_failed > 0) {
        printf(COLOR_RED "Tests failed: %d" COLOR_RESET "\n", g_tests_failed);
    } else {
        printf("Tests failed: 0\n");
    }
    printf("========================================\n");
}

static inline int test_exit_code(void) {
    return g_tests_failed > 0 ? 1 : 0;
}

