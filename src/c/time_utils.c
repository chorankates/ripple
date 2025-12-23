#include "time_utils.h"
#include <stdio.h>

// =============================================================================
// Preset Definitions
// =============================================================================

const int TIMER_PRESETS[TIMER_PRESETS_COUNT] = {5, 10, 15, 30};

// =============================================================================
// Time Decomposition & Composition
// =============================================================================

TimeComponents time_decompose(int total_seconds) {
    TimeComponents result;
    
    if (total_seconds < 0) {
        total_seconds = 0;
    }
    
    result.hours = total_seconds / 3600;
    result.minutes = (total_seconds % 3600) / 60;
    result.seconds = total_seconds % 60;
    
    return result;
}

int time_compose(int hours, int minutes, int seconds) {
    if (hours < 0) hours = 0;
    if (minutes < 0) minutes = 0;
    if (seconds < 0) seconds = 0;
    
    return (hours * 3600) + (minutes * 60) + seconds;
}

// =============================================================================
// Time Formatting
// =============================================================================

void time_format_adaptive(int total_seconds, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return;
    
    if (total_seconds < 0) {
        total_seconds = 0;
    }
    
    TimeComponents t = time_decompose(total_seconds);
    
    if (t.hours > 0) {
        snprintf(buffer, buffer_size, "%d:%02d:%02d", t.hours, t.minutes, t.seconds);
    } else {
        snprintf(buffer, buffer_size, "%d:%02d", t.minutes, t.seconds);
    }
}

void time_format_hex(int total_seconds, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return;
    
    if (total_seconds < 0) {
        total_seconds = 0;
    }
    
    TimeComponents t = time_decompose(total_seconds);
    
    if (t.hours > 0) {
        snprintf(buffer, buffer_size, "%X:%02X:%02X", t.hours, t.minutes, t.seconds);
    } else {
        snprintf(buffer, buffer_size, "%X:%02X", t.minutes, t.seconds);
    }
}

void time_format_preset(int preset_index, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return;
    
    if (preset_index >= 0 && preset_index < TIMER_PRESETS_COUNT) {
        snprintf(buffer, buffer_size, "%d min", TIMER_PRESETS[preset_index]);
    } else {
        snprintf(buffer, buffer_size, "Custom");
    }
}

// =============================================================================
// Progress Calculations
// =============================================================================

int progress_calculate_blocks(int remaining_seconds, int total_seconds, int total_blocks) {
    if (total_seconds <= 0 || total_blocks <= 0) {
        return 0;
    }
    
    if (remaining_seconds <= 0) {
        return 0;
    }
    
    if (remaining_seconds >= total_seconds) {
        return total_blocks;
    }
    
    return (remaining_seconds * total_blocks) / total_seconds;
}

int progress_calculate_degrees(int remaining_seconds, int total_seconds) {
    if (total_seconds <= 0) {
        return 0;
    }
    
    if (remaining_seconds <= 0) {
        return 0;
    }
    
    if (remaining_seconds >= total_seconds) {
        return 360;
    }
    
    return (remaining_seconds * 360) / total_seconds;
}

int progress_calculate_ratio_fp(int remaining_seconds, int total_seconds) {
    if (total_seconds <= 0) {
        return 0;
    }
    
    if (remaining_seconds <= 0) {
        return 0;
    }
    
    if (remaining_seconds >= total_seconds) {
        return 1000;
    }
    
    return (remaining_seconds * 1000) / total_seconds;
}

// =============================================================================
// Value Wrapping
// =============================================================================

int wrap_value(int value, int min, int max) {
    if (max < min) {
        // Invalid range, return min
        return min;
    }
    
    int range = max - min + 1;
    
    while (value < min) {
        value += range;
    }
    
    while (value > max) {
        value -= range;
    }
    
    return value;
}

int increment_wrap(int value, int max) {
    value++;
    if (value > max) {
        return 0;
    }
    return value;
}

int decrement_wrap(int value, int max) {
    value--;
    if (value < 0) {
        return max;
    }
    return value;
}


