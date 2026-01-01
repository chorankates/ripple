#pragma once

#include <stdbool.h>
#include "time_utils.h"

// =============================================================================
// Timer State Machine - Pure Logic (No SDK Dependencies)
// =============================================================================
// This module contains pure state machine logic that can be unit tested
// without the Pebble SDK. Side effects are signaled via the TimerEffects
// structure, which the caller (SDK layer) translates into actual actions.

// =============================================================================
// State Definitions
// =============================================================================

typedef enum {
    STATE_SELECT_PRESET,
    STATE_SET_CUSTOM_HOURS,
    STATE_SET_CUSTOM_MINUTES,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_COMPLETED,
    STATE_CONFIRM_EXIT
} TimerState;

// =============================================================================
// Display Mode Definitions
// =============================================================================

typedef enum {
    DISPLAY_MODE_TEXT,
    DISPLAY_MODE_BLOCKS,
    DISPLAY_MODE_VERTICAL_BLOCKS,
    DISPLAY_MODE_CLOCK,
    DISPLAY_MODE_RING,
    DISPLAY_MODE_HOURGLASS,
    DISPLAY_MODE_BINARY,
    DISPLAY_MODE_RADIAL,
    DISPLAY_MODE_HEX,
    DISPLAY_MODE_MATRIX,
    DISPLAY_MODE_WATER_LEVEL,
    DISPLAY_MODE_SPIRAL_OUT,
    DISPLAY_MODE_SPIRAL_IN,
    DISPLAY_MODE_PERCENT,
    DISPLAY_MODE_PERCENT_REMAINING,
    DISPLAY_MODE_FUZZY,
    DISPLAY_MODE_COUNT  // Used for cycling
} DisplayMode;

// =============================================================================
// Timer Context - All State in One Structure
// =============================================================================

typedef struct {
    // Core state
    TimerState state;
    DisplayMode display_mode;
    
    // Timer values
    int remaining_seconds;
    int total_seconds;
    
    // Selection state
    int selected_preset;
    int custom_hours;
    int custom_minutes;
    
    // Display options
    bool hide_time_text;  // Hide m:ss overlay on visualizations
} TimerContext;

// =============================================================================
// Side Effects - Signals for SDK Layer
// =============================================================================
// These flags tell the SDK integration layer what actions to take.
// This keeps the pure logic separate from SDK calls.

typedef struct {
    bool update_display;
    bool subscribe_tick_timer;
    bool unsubscribe_tick_timer;
    bool start_vibration;
    bool stop_vibration;
    bool init_hourglass;
    bool init_matrix;
    bool vibrate_short;
    bool pop_window;
} TimerEffects;

// =============================================================================
// Context Initialization
// =============================================================================

// Initialize context with default values
void timer_context_init(TimerContext *ctx);

// Get a zeroed effects structure
TimerEffects timer_effects_none(void);

// =============================================================================
// State Queries
// =============================================================================

// Get display mode name as string
const char* timer_display_mode_name(DisplayMode mode);

// Check if timer is active (running or paused)
bool timer_is_active(const TimerContext *ctx);

// Check if canvas should be shown (vs text layers)
bool timer_should_show_canvas(const TimerContext *ctx);

// =============================================================================
// Timer Actions - Return Effects to Apply
// =============================================================================

// Start timer with given minutes
TimerEffects timer_start(TimerContext *ctx, int minutes);

// Handle tick (called every second when subscribed)
TimerEffects timer_tick(TimerContext *ctx);

// Pause running timer
TimerEffects timer_pause(TimerContext *ctx);

// Resume paused timer
TimerEffects timer_resume(TimerContext *ctx);

// Cancel timer and return to preset selection
TimerEffects timer_cancel(TimerContext *ctx);

// Restart timer from beginning
TimerEffects timer_restart(TimerContext *ctx);

// Dismiss completion alert
TimerEffects timer_dismiss_completion(TimerContext *ctx);

// Cycle to next display mode
TimerEffects timer_cycle_display_mode(TimerContext *ctx);

// Toggle hide time text setting
TimerEffects timer_toggle_hide_time_text(TimerContext *ctx);

// =============================================================================
// Input Handling - Button Press Actions
// =============================================================================

// Handle SELECT button press
TimerEffects timer_handle_select(TimerContext *ctx);

// Handle SELECT long press
TimerEffects timer_handle_select_long(TimerContext *ctx);

// Handle UP button press
TimerEffects timer_handle_up(TimerContext *ctx);

// Handle DOWN button press
TimerEffects timer_handle_down(TimerContext *ctx);

// Handle BACK button press
TimerEffects timer_handle_back(TimerContext *ctx);

// Handle UP long press
TimerEffects timer_handle_up_long(TimerContext *ctx);

