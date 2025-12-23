// =============================================================================
// Time Utils Unit Tests
// =============================================================================

#include "test_framework.h"
#include "../src/c/time_utils.h"

// =============================================================================
// Time Decomposition Tests
// =============================================================================

bool test_time_decompose_zero(void) {
    TimeComponents t = time_decompose(0);
    TEST_ASSERT_EQUAL(0, t.hours);
    TEST_ASSERT_EQUAL(0, t.minutes);
    TEST_ASSERT_EQUAL(0, t.seconds);
    return true;
}

bool test_time_decompose_seconds_only(void) {
    TimeComponents t = time_decompose(45);
    TEST_ASSERT_EQUAL(0, t.hours);
    TEST_ASSERT_EQUAL(0, t.minutes);
    TEST_ASSERT_EQUAL(45, t.seconds);
    return true;
}

bool test_time_decompose_minutes_and_seconds(void) {
    TimeComponents t = time_decompose(125);  // 2:05
    TEST_ASSERT_EQUAL(0, t.hours);
    TEST_ASSERT_EQUAL(2, t.minutes);
    TEST_ASSERT_EQUAL(5, t.seconds);
    return true;
}

bool test_time_decompose_hours_minutes_seconds(void) {
    TimeComponents t = time_decompose(3661);  // 1:01:01
    TEST_ASSERT_EQUAL(1, t.hours);
    TEST_ASSERT_EQUAL(1, t.minutes);
    TEST_ASSERT_EQUAL(1, t.seconds);
    return true;
}

bool test_time_decompose_large_value(void) {
    TimeComponents t = time_decompose(86399);  // 23:59:59
    TEST_ASSERT_EQUAL(23, t.hours);
    TEST_ASSERT_EQUAL(59, t.minutes);
    TEST_ASSERT_EQUAL(59, t.seconds);
    return true;
}

bool test_time_decompose_negative(void) {
    TimeComponents t = time_decompose(-100);  // Should clamp to 0
    TEST_ASSERT_EQUAL(0, t.hours);
    TEST_ASSERT_EQUAL(0, t.minutes);
    TEST_ASSERT_EQUAL(0, t.seconds);
    return true;
}

// =============================================================================
// Time Composition Tests
// =============================================================================

bool test_time_compose_zero(void) {
    TEST_ASSERT_EQUAL(0, time_compose(0, 0, 0));
    return true;
}

bool test_time_compose_seconds_only(void) {
    TEST_ASSERT_EQUAL(30, time_compose(0, 0, 30));
    return true;
}

bool test_time_compose_minutes_seconds(void) {
    TEST_ASSERT_EQUAL(125, time_compose(0, 2, 5));
    return true;
}

bool test_time_compose_hours_minutes_seconds(void) {
    TEST_ASSERT_EQUAL(3661, time_compose(1, 1, 1));
    return true;
}

bool test_time_compose_roundtrip(void) {
    int original = 7384;
    TimeComponents t = time_decompose(original);
    int result = time_compose(t.hours, t.minutes, t.seconds);
    TEST_ASSERT_EQUAL(original, result);
    return true;
}

// =============================================================================
// Time Formatting Tests
// =============================================================================

bool test_time_format_adaptive_zero(void) {
    char buf[16];
    time_format_adaptive(0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("0:00", buf);
    return true;
}

bool test_time_format_adaptive_seconds(void) {
    char buf[16];
    time_format_adaptive(45, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("0:45", buf);
    return true;
}

bool test_time_format_adaptive_minutes(void) {
    char buf[16];
    time_format_adaptive(125, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("2:05", buf);
    return true;
}

bool test_time_format_adaptive_with_hours(void) {
    char buf[16];
    time_format_adaptive(3661, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("1:01:01", buf);
    return true;
}

bool test_time_format_adaptive_null_buffer(void) {
    // Should not crash
    time_format_adaptive(100, NULL, 0);
    return true;
}

bool test_time_format_hex_minutes(void) {
    char buf[16];
    time_format_hex(125, buf, sizeof(buf));  // 2:05 -> 2:5 in hex
    TEST_ASSERT_EQUAL_STRING("2:05", buf);
    return true;
}

bool test_time_format_hex_with_hex_digits(void) {
    char buf[16];
    time_format_hex(690, buf, sizeof(buf));  // 11:30 -> B:1E in hex
    TEST_ASSERT_EQUAL_STRING("B:1E", buf);
    return true;
}

bool test_time_format_preset_five_minutes(void) {
    char buf[16];
    time_format_preset(0, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("5 min", buf);
    return true;
}

bool test_time_format_preset_thirty_minutes(void) {
    char buf[16];
    time_format_preset(3, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("30 min", buf);
    return true;
}

bool test_time_format_preset_custom(void) {
    char buf[16];
    time_format_preset(4, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("Custom", buf);
    return true;
}

// =============================================================================
// Progress Calculation Tests
// =============================================================================

bool test_progress_blocks_full(void) {
    TEST_ASSERT_EQUAL(96, progress_calculate_blocks(300, 300, 96));
    return true;
}

bool test_progress_blocks_half(void) {
    TEST_ASSERT_EQUAL(48, progress_calculate_blocks(150, 300, 96));
    return true;
}

bool test_progress_blocks_empty(void) {
    TEST_ASSERT_EQUAL(0, progress_calculate_blocks(0, 300, 96));
    return true;
}

bool test_progress_blocks_zero_total(void) {
    TEST_ASSERT_EQUAL(0, progress_calculate_blocks(100, 0, 96));
    return true;
}

bool test_progress_degrees_full(void) {
    TEST_ASSERT_EQUAL(360, progress_calculate_degrees(300, 300));
    return true;
}

bool test_progress_degrees_half(void) {
    TEST_ASSERT_EQUAL(180, progress_calculate_degrees(150, 300));
    return true;
}

bool test_progress_degrees_quarter(void) {
    TEST_ASSERT_EQUAL(90, progress_calculate_degrees(75, 300));
    return true;
}

bool test_progress_ratio_full(void) {
    TEST_ASSERT_EQUAL(1000, progress_calculate_ratio_fp(300, 300));
    return true;
}

bool test_progress_ratio_half(void) {
    TEST_ASSERT_EQUAL(500, progress_calculate_ratio_fp(150, 300));
    return true;
}

// =============================================================================
// Value Wrapping Tests
// =============================================================================

bool test_increment_wrap_normal(void) {
    TEST_ASSERT_EQUAL(5, increment_wrap(4, 10));
    return true;
}

bool test_increment_wrap_at_max(void) {
    TEST_ASSERT_EQUAL(0, increment_wrap(10, 10));
    return true;
}

bool test_increment_wrap_hours(void) {
    TEST_ASSERT_EQUAL(0, increment_wrap(23, 23));
    return true;
}

bool test_decrement_wrap_normal(void) {
    TEST_ASSERT_EQUAL(4, decrement_wrap(5, 10));
    return true;
}

bool test_decrement_wrap_at_zero(void) {
    TEST_ASSERT_EQUAL(10, decrement_wrap(0, 10));
    return true;
}

bool test_decrement_wrap_minutes(void) {
    TEST_ASSERT_EQUAL(59, decrement_wrap(0, 59));
    return true;
}

bool test_wrap_value_in_range(void) {
    TEST_ASSERT_EQUAL(5, wrap_value(5, 0, 10));
    return true;
}

bool test_wrap_value_below_min(void) {
    TEST_ASSERT_EQUAL(10, wrap_value(-1, 0, 10));
    return true;
}

bool test_wrap_value_above_max(void) {
    TEST_ASSERT_EQUAL(0, wrap_value(11, 0, 10));
    return true;
}

// =============================================================================
// Test Suite Runner
// =============================================================================

void run_time_utils_tests(void) {
    TEST_SUITE_BEGIN("Time Decomposition");
    RUN_TEST(test_time_decompose_zero);
    RUN_TEST(test_time_decompose_seconds_only);
    RUN_TEST(test_time_decompose_minutes_and_seconds);
    RUN_TEST(test_time_decompose_hours_minutes_seconds);
    RUN_TEST(test_time_decompose_large_value);
    RUN_TEST(test_time_decompose_negative);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Time Composition");
    RUN_TEST(test_time_compose_zero);
    RUN_TEST(test_time_compose_seconds_only);
    RUN_TEST(test_time_compose_minutes_seconds);
    RUN_TEST(test_time_compose_hours_minutes_seconds);
    RUN_TEST(test_time_compose_roundtrip);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Time Formatting");
    RUN_TEST(test_time_format_adaptive_zero);
    RUN_TEST(test_time_format_adaptive_seconds);
    RUN_TEST(test_time_format_adaptive_minutes);
    RUN_TEST(test_time_format_adaptive_with_hours);
    RUN_TEST(test_time_format_adaptive_null_buffer);
    RUN_TEST(test_time_format_hex_minutes);
    RUN_TEST(test_time_format_hex_with_hex_digits);
    RUN_TEST(test_time_format_preset_five_minutes);
    RUN_TEST(test_time_format_preset_thirty_minutes);
    RUN_TEST(test_time_format_preset_custom);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Progress Calculations");
    RUN_TEST(test_progress_blocks_full);
    RUN_TEST(test_progress_blocks_half);
    RUN_TEST(test_progress_blocks_empty);
    RUN_TEST(test_progress_blocks_zero_total);
    RUN_TEST(test_progress_degrees_full);
    RUN_TEST(test_progress_degrees_half);
    RUN_TEST(test_progress_degrees_quarter);
    RUN_TEST(test_progress_ratio_full);
    RUN_TEST(test_progress_ratio_half);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Value Wrapping");
    RUN_TEST(test_increment_wrap_normal);
    RUN_TEST(test_increment_wrap_at_max);
    RUN_TEST(test_increment_wrap_hours);
    RUN_TEST(test_decrement_wrap_normal);
    RUN_TEST(test_decrement_wrap_at_zero);
    RUN_TEST(test_decrement_wrap_minutes);
    RUN_TEST(test_wrap_value_in_range);
    RUN_TEST(test_wrap_value_below_min);
    RUN_TEST(test_wrap_value_above_max);
    TEST_SUITE_END();
}

