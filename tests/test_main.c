// =============================================================================
// Pebble Timer Test Runner
// =============================================================================
// Runs all unit tests for pure logic modules (no Pebble SDK required)

#include "test_framework.h"

// Test result tracking - single definition
int g_tests_run = 0;
int g_tests_passed = 0;
int g_tests_failed = 0;

// External test suite runners
extern void run_time_utils_tests(void);
extern void run_timer_state_tests(void);

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║    Pebble Timer Unit Tests           ║\n");
    printf("╚══════════════════════════════════════╝\n");
    
    // Run all test suites
    run_time_utils_tests();
    run_timer_state_tests();
    
    // Print summary
    print_test_summary();
    
    return test_exit_code();
}

