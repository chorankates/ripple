// =============================================================================
// Timer State Machine Unit Tests
// =============================================================================

#include "test_framework.h"
#include "../src/c/timer_state.h"

// =============================================================================
// Context Initialization Tests
// =============================================================================

bool test_context_init_defaults(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_EQUAL(DISPLAY_MODE_TEXT, ctx.display_mode);
    TEST_ASSERT_EQUAL(0, ctx.remaining_seconds);
    TEST_ASSERT_EQUAL(0, ctx.total_seconds);
    TEST_ASSERT_EQUAL(0, ctx.selected_preset);
    TEST_ASSERT_EQUAL(0, ctx.custom_hours);
    TEST_ASSERT_EQUAL(5, ctx.custom_minutes);  // Default 5 minutes
    return true;
}

bool test_effects_none_all_false(void) {
    TimerEffects effects = timer_effects_none();
    
    TEST_ASSERT_FALSE(effects.update_display);
    TEST_ASSERT_FALSE(effects.subscribe_tick_timer);
    TEST_ASSERT_FALSE(effects.unsubscribe_tick_timer);
    TEST_ASSERT_FALSE(effects.start_vibration);
    TEST_ASSERT_FALSE(effects.stop_vibration);
    TEST_ASSERT_FALSE(effects.init_hourglass);
    TEST_ASSERT_FALSE(effects.init_matrix);
    TEST_ASSERT_FALSE(effects.vibrate_short);
    TEST_ASSERT_FALSE(effects.pop_window);
    return true;
}

// =============================================================================
// Timer Start Tests
// =============================================================================

bool test_timer_start_initializes_correctly(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TimerEffects effects = timer_start(&ctx, 5);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_EQUAL(300, ctx.remaining_seconds);
    TEST_ASSERT_EQUAL(300, ctx.total_seconds);
    TEST_ASSERT_TRUE(effects.subscribe_tick_timer);
    TEST_ASSERT_TRUE(effects.update_display);
    TEST_ASSERT_TRUE(effects.init_hourglass);
    TEST_ASSERT_TRUE(effects.init_matrix);
    return true;
}

bool test_timer_start_zero_minutes_no_op(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TimerEffects effects = timer_start(&ctx, 0);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);  // Should not change
    TEST_ASSERT_FALSE(effects.subscribe_tick_timer);
    return true;
}

bool test_timer_start_negative_minutes_no_op(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TimerEffects effects = timer_start(&ctx, -5);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_FALSE(effects.subscribe_tick_timer);
    return true;
}

// =============================================================================
// Timer Tick Tests
// =============================================================================

bool test_timer_tick_decrements_time(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_tick(&ctx);
    
    TEST_ASSERT_EQUAL(99, ctx.remaining_seconds);
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    TEST_ASSERT_FALSE(effects.start_vibration);
    return true;
}

bool test_timer_tick_completes_at_one(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 1,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_tick(&ctx);
    
    TEST_ASSERT_EQUAL(0, ctx.remaining_seconds);
    TEST_ASSERT_EQUAL(STATE_COMPLETED, ctx.state);
    TEST_ASSERT_TRUE(effects.start_vibration);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_timer_tick_no_op_when_paused(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 100,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_tick(&ctx);
    
    TEST_ASSERT_EQUAL(100, ctx.remaining_seconds);  // Unchanged
    TEST_ASSERT_EQUAL(STATE_PAUSED, ctx.state);
    TEST_ASSERT_FALSE(effects.update_display);
    return true;
}

bool test_timer_tick_no_op_when_completed(void) {
    TimerContext ctx = {
        .state = STATE_COMPLETED,
        .remaining_seconds = 0,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_tick(&ctx);
    
    TEST_ASSERT_EQUAL(0, ctx.remaining_seconds);
    TEST_ASSERT_EQUAL(STATE_COMPLETED, ctx.state);
    TEST_ASSERT_FALSE(effects.update_display);
    return true;
}

// =============================================================================
// Pause/Resume Tests
// =============================================================================

bool test_timer_pause_changes_state(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_pause(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_PAUSED, ctx.state);
    TEST_ASSERT_EQUAL(100, ctx.remaining_seconds);  // Time preserved
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_timer_pause_no_op_if_not_running(void) {
    TimerContext ctx = {
        .state = STATE_SELECT_PRESET,
        .remaining_seconds = 0
    };
    
    TimerEffects effects = timer_pause(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_FALSE(effects.update_display);
    return true;
}

bool test_timer_resume_changes_state(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_resume(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_timer_resume_no_op_if_not_paused(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_resume(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_FALSE(effects.update_display);
    return true;
}

// =============================================================================
// Cancel/Restart Tests
// =============================================================================

bool test_timer_cancel_resets_state(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_cancel(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_EQUAL(0, ctx.remaining_seconds);
    TEST_ASSERT_TRUE(effects.unsubscribe_tick_timer);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_timer_restart_resets_time(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 50,
        .total_seconds = 300
    };
    
    TimerEffects effects = timer_restart(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_EQUAL(300, ctx.remaining_seconds);
    TEST_ASSERT_TRUE(effects.init_hourglass);
    TEST_ASSERT_TRUE(effects.init_matrix);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

// =============================================================================
// Display Mode Tests
// =============================================================================

bool test_cycle_display_mode(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TEST_ASSERT_EQUAL(DISPLAY_MODE_TEXT, ctx.display_mode);
    
    timer_cycle_display_mode(&ctx);
    TEST_ASSERT_EQUAL(DISPLAY_MODE_BLOCKS, ctx.display_mode);
    
    timer_cycle_display_mode(&ctx);
    TEST_ASSERT_EQUAL(DISPLAY_MODE_VERTICAL_BLOCKS, ctx.display_mode);
    
    return true;
}

bool test_cycle_display_mode_wraps(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.display_mode = DISPLAY_MODE_SPIRAL_IN;  // Last mode
    
    TimerEffects effects = timer_cycle_display_mode(&ctx);
    
    TEST_ASSERT_EQUAL(DISPLAY_MODE_TEXT, ctx.display_mode);  // Wrapped
    TEST_ASSERT_TRUE(effects.vibrate_short);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_display_mode_name(void) {
    TEST_ASSERT_EQUAL_STRING("Text", timer_display_mode_name(DISPLAY_MODE_TEXT));
    TEST_ASSERT_EQUAL_STRING("Blocks", timer_display_mode_name(DISPLAY_MODE_BLOCKS));
    TEST_ASSERT_EQUAL_STRING("Matrix", timer_display_mode_name(DISPLAY_MODE_MATRIX));
    TEST_ASSERT_EQUAL_STRING("Water Level", timer_display_mode_name(DISPLAY_MODE_WATER_LEVEL));
    TEST_ASSERT_EQUAL_STRING("Spiral Out", timer_display_mode_name(DISPLAY_MODE_SPIRAL_OUT));
    TEST_ASSERT_EQUAL_STRING("Spiral In", timer_display_mode_name(DISPLAY_MODE_SPIRAL_IN));
    return true;
}

// =============================================================================
// State Query Tests
// =============================================================================

bool test_timer_is_active_running(void) {
    TimerContext ctx = { .state = STATE_RUNNING };
    TEST_ASSERT_TRUE(timer_is_active(&ctx));
    return true;
}

bool test_timer_is_active_paused(void) {
    TimerContext ctx = { .state = STATE_PAUSED };
    TEST_ASSERT_TRUE(timer_is_active(&ctx));
    return true;
}

bool test_timer_is_active_select_preset(void) {
    TimerContext ctx = { .state = STATE_SELECT_PRESET };
    TEST_ASSERT_FALSE(timer_is_active(&ctx));
    return true;
}

bool test_timer_is_active_completed(void) {
    TimerContext ctx = { .state = STATE_COMPLETED };
    TEST_ASSERT_FALSE(timer_is_active(&ctx));
    return true;
}

bool test_should_show_canvas_running_blocks(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .display_mode = DISPLAY_MODE_BLOCKS
    };
    TEST_ASSERT_TRUE(timer_should_show_canvas(&ctx));
    return true;
}

bool test_should_show_canvas_running_text(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .display_mode = DISPLAY_MODE_TEXT
    };
    TEST_ASSERT_FALSE(timer_should_show_canvas(&ctx));
    return true;
}

bool test_should_show_canvas_select_preset(void) {
    TimerContext ctx = {
        .state = STATE_SELECT_PRESET,
        .display_mode = DISPLAY_MODE_BLOCKS
    };
    TEST_ASSERT_FALSE(timer_should_show_canvas(&ctx));
    return true;
}

// =============================================================================
// Input Handler Tests - SELECT Button
// =============================================================================

bool test_handle_select_starts_preset_timer(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = 1;  // 10 minutes
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_EQUAL(600, ctx.remaining_seconds);  // 10 * 60
    TEST_ASSERT_TRUE(effects.subscribe_tick_timer);
    return true;
}

bool test_handle_select_enters_custom_hours(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = TIMER_CUSTOM_OPTION;
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SET_CUSTOM_HOURS, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_handle_select_advances_to_minutes(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.state = STATE_SET_CUSTOM_HOURS;
    ctx.custom_hours = 1;
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SET_CUSTOM_MINUTES, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_handle_select_starts_custom_timer(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.state = STATE_SET_CUSTOM_MINUTES;
    ctx.custom_hours = 1;
    ctx.custom_minutes = 30;
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_EQUAL(5400, ctx.remaining_seconds);  // (1*60 + 30) * 60
    TEST_ASSERT_TRUE(effects.subscribe_tick_timer);
    return true;
}

bool test_handle_select_pauses_running_timer(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_PAUSED, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_handle_select_resumes_paused_timer(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_TRUE(effects.update_display);
    return true;
}

bool test_handle_select_dismisses_completion(void) {
    TimerContext ctx = {
        .state = STATE_COMPLETED,
        .remaining_seconds = 0
    };
    
    TimerEffects effects = timer_handle_select(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_TRUE(effects.stop_vibration);
    TEST_ASSERT_TRUE(effects.unsubscribe_tick_timer);
    return true;
}

// =============================================================================
// Input Handler Tests - UP Button
// =============================================================================

bool test_handle_up_decrements_preset(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = 2;
    
    timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(1, ctx.selected_preset);
    return true;
}

bool test_handle_up_wraps_preset(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = 0;
    
    timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(TIMER_CUSTOM_OPTION, ctx.selected_preset);
    return true;
}

bool test_handle_up_increments_custom_hours(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.state = STATE_SET_CUSTOM_HOURS;
    ctx.custom_hours = 5;
    
    timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(6, ctx.custom_hours);
    return true;
}

bool test_handle_up_wraps_custom_hours(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.state = STATE_SET_CUSTOM_HOURS;
    ctx.custom_hours = 23;
    
    timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(0, ctx.custom_hours);
    return true;
}

bool test_handle_up_restarts_when_paused(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 50,
        .total_seconds = 300
    };
    
    timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_RUNNING, ctx.state);
    TEST_ASSERT_EQUAL(300, ctx.remaining_seconds);
    return true;
}

bool test_handle_up_confirms_exit(void) {
    TimerContext ctx = {
        .state = STATE_CONFIRM_EXIT,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_handle_up(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_TRUE(effects.pop_window);
    return true;
}

// =============================================================================
// Input Handler Tests - DOWN Button
// =============================================================================

bool test_handle_down_increments_preset(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = 1;
    
    timer_handle_down(&ctx);
    
    TEST_ASSERT_EQUAL(2, ctx.selected_preset);
    return true;
}

bool test_handle_down_wraps_preset(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    ctx.selected_preset = TIMER_CUSTOM_OPTION;
    
    timer_handle_down(&ctx);
    
    TEST_ASSERT_EQUAL(0, ctx.selected_preset);
    return true;
}

bool test_handle_down_cancels_when_paused(void) {
    TimerContext ctx = {
        .state = STATE_PAUSED,
        .remaining_seconds = 100
    };
    
    TimerEffects effects = timer_handle_down(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    TEST_ASSERT_TRUE(effects.unsubscribe_tick_timer);
    return true;
}

bool test_handle_down_declines_exit(void) {
    TimerContext ctx = {
        .state = STATE_CONFIRM_EXIT,
        .remaining_seconds = 100
    };
    
    timer_handle_down(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_PAUSED, ctx.state);
    return true;
}

// =============================================================================
// Input Handler Tests - BACK Button
// =============================================================================

bool test_handle_back_shows_exit_confirm_when_running(void) {
    TimerContext ctx = {
        .state = STATE_RUNNING,
        .remaining_seconds = 100
    };
    
    timer_handle_back(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_CONFIRM_EXIT, ctx.state);
    return true;
}

bool test_handle_back_returns_to_preset_from_custom(void) {
    TimerContext ctx = {
        .state = STATE_SET_CUSTOM_HOURS
    };
    
    timer_handle_back(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_SELECT_PRESET, ctx.state);
    return true;
}

bool test_handle_back_returns_to_paused_from_confirm(void) {
    TimerContext ctx = {
        .state = STATE_CONFIRM_EXIT,
        .remaining_seconds = 100
    };
    
    timer_handle_back(&ctx);
    
    TEST_ASSERT_EQUAL(STATE_PAUSED, ctx.state);
    return true;
}

bool test_handle_back_pops_window_from_select_preset(void) {
    TimerContext ctx;
    timer_context_init(&ctx);
    
    TimerEffects effects = timer_handle_back(&ctx);
    
    TEST_ASSERT_TRUE(effects.pop_window);
    return true;
}

// =============================================================================
// Test Suite Runner
// =============================================================================

void run_timer_state_tests(void) {
    TEST_SUITE_BEGIN("Context Initialization");
    RUN_TEST(test_context_init_defaults);
    RUN_TEST(test_effects_none_all_false);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Timer Start");
    RUN_TEST(test_timer_start_initializes_correctly);
    RUN_TEST(test_timer_start_zero_minutes_no_op);
    RUN_TEST(test_timer_start_negative_minutes_no_op);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Timer Tick");
    RUN_TEST(test_timer_tick_decrements_time);
    RUN_TEST(test_timer_tick_completes_at_one);
    RUN_TEST(test_timer_tick_no_op_when_paused);
    RUN_TEST(test_timer_tick_no_op_when_completed);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Pause/Resume");
    RUN_TEST(test_timer_pause_changes_state);
    RUN_TEST(test_timer_pause_no_op_if_not_running);
    RUN_TEST(test_timer_resume_changes_state);
    RUN_TEST(test_timer_resume_no_op_if_not_paused);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Cancel/Restart");
    RUN_TEST(test_timer_cancel_resets_state);
    RUN_TEST(test_timer_restart_resets_time);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("Display Mode");
    RUN_TEST(test_cycle_display_mode);
    RUN_TEST(test_cycle_display_mode_wraps);
    RUN_TEST(test_display_mode_name);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("State Queries");
    RUN_TEST(test_timer_is_active_running);
    RUN_TEST(test_timer_is_active_paused);
    RUN_TEST(test_timer_is_active_select_preset);
    RUN_TEST(test_timer_is_active_completed);
    RUN_TEST(test_should_show_canvas_running_blocks);
    RUN_TEST(test_should_show_canvas_running_text);
    RUN_TEST(test_should_show_canvas_select_preset);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("SELECT Button Handler");
    RUN_TEST(test_handle_select_starts_preset_timer);
    RUN_TEST(test_handle_select_enters_custom_hours);
    RUN_TEST(test_handle_select_advances_to_minutes);
    RUN_TEST(test_handle_select_starts_custom_timer);
    RUN_TEST(test_handle_select_pauses_running_timer);
    RUN_TEST(test_handle_select_resumes_paused_timer);
    RUN_TEST(test_handle_select_dismisses_completion);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("UP Button Handler");
    RUN_TEST(test_handle_up_decrements_preset);
    RUN_TEST(test_handle_up_wraps_preset);
    RUN_TEST(test_handle_up_increments_custom_hours);
    RUN_TEST(test_handle_up_wraps_custom_hours);
    RUN_TEST(test_handle_up_restarts_when_paused);
    RUN_TEST(test_handle_up_confirms_exit);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("DOWN Button Handler");
    RUN_TEST(test_handle_down_increments_preset);
    RUN_TEST(test_handle_down_wraps_preset);
    RUN_TEST(test_handle_down_cancels_when_paused);
    RUN_TEST(test_handle_down_declines_exit);
    TEST_SUITE_END();
    
    TEST_SUITE_BEGIN("BACK Button Handler");
    RUN_TEST(test_handle_back_shows_exit_confirm_when_running);
    RUN_TEST(test_handle_back_returns_to_preset_from_custom);
    RUN_TEST(test_handle_back_returns_to_paused_from_confirm);
    RUN_TEST(test_handle_back_pops_window_from_select_preset);
    TEST_SUITE_END();
}

