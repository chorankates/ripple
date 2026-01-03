#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <pebble.h>
#include "timer_state.h"
#include "colors.h"

// =============================================================================
// Settings Module - Timer Settings and Persistence
// =============================================================================
// Manages user preferences that persist across app restarts.
// Pure data structures here; SDK persistence calls in pebble-timer.c.

// =============================================================================
// Persistent Storage Keys
// =============================================================================

#define SETTINGS_KEY_VERSION       0x1000
#define SETTINGS_KEY_DATA          0x1001
#define SETTINGS_KEY_DISPLAY_MODE  0x1002  // Legacy v1
#define SETTINGS_KEY_DEFAULT_TIME  0x1003  // Legacy v1
#define SETTINGS_KEY_HIDE_TIME     0x1004  // Legacy v1

// Current settings version (increment when structure changes)
#define SETTINGS_VERSION 2

// =============================================================================
// Settings Structure
// =============================================================================

typedef struct {
    // Display preferences
    DisplayMode default_display_mode;  // Which visualization to start with
    bool hide_time_text;               // Hide m:ss overlay on visualizations
    bool visualization_enabled[DISPLAY_MODE_COUNT];  // Per-visualization toggle
    VisualizationColors visualization_colors[DISPLAY_MODE_COUNT];  // Per-visualization palette
    
    // Timer defaults
    int default_preset_index;          // Default preset to select (0-3 for presets, 4 for custom)
    int default_custom_minutes;        // Default custom time in minutes (when preset is custom)
} TimerSettings;

// =============================================================================
// Settings Initialization
// =============================================================================

// Initialize settings with defaults
void settings_init_defaults(TimerSettings *settings);

// =============================================================================
// Settings Validation
// =============================================================================

// Validate and clamp settings to valid ranges
void settings_validate(TimerSettings *settings);

// =============================================================================
// Settings Application
// =============================================================================

// Apply settings to timer context (called on init)
void settings_apply_to_context(const TimerSettings *settings, TimerContext *ctx);

// Update settings from current context (for saving user preferences)
void settings_update_from_context(TimerSettings *settings, const TimerContext *ctx);

// Load settings from persistent storage (falls back to defaults)
void settings_persist_load(TimerSettings *settings);

// Save settings to persistent storage
void settings_persist_save(TimerSettings *settings);



