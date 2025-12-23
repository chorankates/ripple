#include "settings.h"
#include "time_utils.h"

// =============================================================================
// Settings Initialization
// =============================================================================

void settings_init_defaults(TimerSettings *settings) {
    settings->default_display_mode = DISPLAY_MODE_TEXT;
    settings->hide_time_text = false;
    settings->default_preset_index = 0;  // First preset (5 min)
    settings->default_custom_minutes = 5;
}

// =============================================================================
// Settings Validation
// =============================================================================

void settings_validate(TimerSettings *settings) {
    // Validate display mode
    if (settings->default_display_mode < 0 || 
        settings->default_display_mode >= DISPLAY_MODE_COUNT) {
        settings->default_display_mode = DISPLAY_MODE_TEXT;
    }
    
    // Validate preset index (0-3 for presets, 4 for custom)
    if (settings->default_preset_index < 0 || 
        settings->default_preset_index > TIMER_CUSTOM_OPTION) {
        settings->default_preset_index = 0;
    }
    
    // Validate custom minutes (1 to 24*60 = 1440)
    if (settings->default_custom_minutes < 1) {
        settings->default_custom_minutes = 5;
    }
    if (settings->default_custom_minutes > 1440) {
        settings->default_custom_minutes = 1440;
    }
}

// =============================================================================
// Settings Application
// =============================================================================

void settings_apply_to_context(const TimerSettings *settings, TimerContext *ctx) {
    ctx->display_mode = settings->default_display_mode;
    ctx->selected_preset = settings->default_preset_index;
    ctx->hide_time_text = settings->hide_time_text;
    
    // Set custom time from settings if custom option is selected
    if (settings->default_preset_index == TIMER_CUSTOM_OPTION) {
        ctx->custom_hours = settings->default_custom_minutes / 60;
        ctx->custom_minutes = settings->default_custom_minutes % 60;
    }
}

void settings_update_from_context(TimerSettings *settings, const TimerContext *ctx) {
    settings->default_display_mode = ctx->display_mode;
    settings->hide_time_text = ctx->hide_time_text;
    // Note: preset index and custom time are not auto-saved from context
    // They are saved when the user explicitly changes the default
}


