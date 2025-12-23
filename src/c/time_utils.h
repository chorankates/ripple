#pragma once

#include <stddef.h>
#include <stdbool.h>

// =============================================================================
// Time Utilities - Pure Functions (No SDK Dependencies)
// =============================================================================
// All functions are pure and testable without Pebble SDK

// Time decomposition structure
typedef struct {
    int hours;
    int minutes;
    int seconds;
} TimeComponents;

// Preset timer definitions
#define TIMER_PRESETS_COUNT 4
#define TIMER_CUSTOM_OPTION 4

extern const int TIMER_PRESETS[TIMER_PRESETS_COUNT];

// =============================================================================
// Time Decomposition & Composition
// =============================================================================

// Break total seconds into hours, minutes, seconds components
TimeComponents time_decompose(int total_seconds);

// Compose hours, minutes, seconds into total seconds
int time_compose(int hours, int minutes, int seconds);

// =============================================================================
// Time Formatting
// =============================================================================

// Format time adaptively: "H:MM:SS" if hours > 0, otherwise "M:SS"
void time_format_adaptive(int total_seconds, char *buffer, size_t buffer_size);

// Format time in hexadecimal: "H:MM:SS" or "M:SS" in hex
void time_format_hex(int total_seconds, char *buffer, size_t buffer_size);

// Format preset option: "5 min", "10 min", or "Custom"
void time_format_preset(int preset_index, char *buffer, size_t buffer_size);

// =============================================================================
// Progress Calculations
// =============================================================================

// Calculate filled blocks for grid displays
// Returns: number of blocks that should be filled (0 to total_blocks)
int progress_calculate_blocks(int remaining_seconds, int total_seconds, int total_blocks);

// Calculate progress as degrees (0-360) for circular displays
int progress_calculate_degrees(int remaining_seconds, int total_seconds);

// Calculate progress as a ratio (0.0 to 1.0)
// Uses fixed-point: returns value 0-1000 representing 0.0-1.0
int progress_calculate_ratio_fp(int remaining_seconds, int total_seconds);

// =============================================================================
// Value Wrapping (for input handling)
// =============================================================================

// Wrap value in range [min, max] inclusive
int wrap_value(int value, int min, int max);

// Increment with wrap: 23 -> 0 for hours, 59 -> 0 for minutes
int increment_wrap(int value, int max);

// Decrement with wrap: 0 -> 23 for hours, 0 -> 59 for minutes
int decrement_wrap(int value, int max);

