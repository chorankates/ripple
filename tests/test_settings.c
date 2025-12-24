#include "test_framework.h"
#include "../src/c/settings.h"
#include "../src/c/time_utils.h"

// =============================================================================
// Settings Defaults
// =============================================================================

bool test_settings_defaults(void) {
    TimerSettings s;
    settings_init_defaults(&s);

    TEST_ASSERT_EQUAL(DISPLAY_MODE_TEXT, s.default_display_mode);
    TEST_ASSERT_FALSE(s.hide_time_text);
    TEST_ASSERT_EQUAL(0, s.default_preset_index);
    TEST_ASSERT_EQUAL(5, s.default_custom_minutes);
    return true;
}

// =============================================================================
// Settings Validation
// =============================================================================

bool test_settings_validation_clamps_display_mode(void) {
    TimerSettings s = {
        .default_display_mode = DISPLAY_MODE_COUNT + 5,
        .default_preset_index = 0,
        .default_custom_minutes = 5,
        .hide_time_text = false
    };

    settings_validate(&s);

    TEST_ASSERT_EQUAL(DISPLAY_MODE_TEXT, s.default_display_mode);
    return true;
}

bool test_settings_validation_clamps_preset_index(void) {
    TimerSettings s = {
        .default_display_mode = DISPLAY_MODE_TEXT,
        .default_preset_index = 99,
        .default_custom_minutes = 5,
        .hide_time_text = false
    };

    settings_validate(&s);

    TEST_ASSERT_EQUAL(0, s.default_preset_index);
    return true;
}

bool test_settings_validation_clamps_custom_minutes_low_high(void) {
    TimerSettings s_low = {
        .default_display_mode = DISPLAY_MODE_TEXT,
        .default_preset_index = TIMER_CUSTOM_OPTION,
        .default_custom_minutes = 0,
        .hide_time_text = false
    };
    settings_validate(&s_low);
    TEST_ASSERT_EQUAL(5, s_low.default_custom_minutes);

    TimerSettings s_high = {
        .default_display_mode = DISPLAY_MODE_TEXT,
        .default_preset_index = TIMER_CUSTOM_OPTION,
        .default_custom_minutes = 9999,
        .hide_time_text = false
    };
    settings_validate(&s_high);
    TEST_ASSERT_EQUAL(1440, s_high.default_custom_minutes);
    return true;
}

// =============================================================================
// Settings Application / Round Trip
// =============================================================================

bool test_settings_apply_to_context_custom_time(void) {
    TimerSettings s;
    settings_init_defaults(&s);
    s.default_display_mode = DISPLAY_MODE_CLOCK;
    s.hide_time_text = true;
    s.default_preset_index = TIMER_CUSTOM_OPTION;
    s.default_custom_minutes = 95; // 1h35m

    TimerContext ctx;
    timer_context_init(&ctx);

    settings_apply_to_context(&s, &ctx);

    TEST_ASSERT_EQUAL(DISPLAY_MODE_CLOCK, ctx.display_mode);
    TEST_ASSERT_TRUE(ctx.hide_time_text);
    TEST_ASSERT_EQUAL(TIMER_CUSTOM_OPTION, ctx.selected_preset);
    TEST_ASSERT_EQUAL(1, ctx.custom_hours);
    TEST_ASSERT_EQUAL(35, ctx.custom_minutes);
    return true;
}

bool test_settings_update_from_context_round_trip(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.display_mode = DISPLAY_MODE_MATRIX;
    ctx.hide_time_text = true;

    TimerSettings s;
    settings_init_defaults(&s);

    settings_update_from_context(&s, &ctx);

    TEST_ASSERT_EQUAL(DISPLAY_MODE_MATRIX, s.default_display_mode);
    TEST_ASSERT_TRUE(s.hide_time_text);
    return true;
}

// =============================================================================
// Test Suite Runner
// =============================================================================

void run_settings_tests(void) {
    TEST_SUITE_BEGIN("Settings Defaults");
    RUN_TEST(test_settings_defaults);
    TEST_SUITE_END();

    TEST_SUITE_BEGIN("Settings Validation");
    RUN_TEST(test_settings_validation_clamps_display_mode);
    RUN_TEST(test_settings_validation_clamps_preset_index);
    RUN_TEST(test_settings_validation_clamps_custom_minutes_low_high);
    TEST_SUITE_END();

    TEST_SUITE_BEGIN("Settings Apply/Update");
    RUN_TEST(test_settings_apply_to_context_custom_time);
    RUN_TEST(test_settings_update_from_context_round_trip);
    TEST_SUITE_END();
}

