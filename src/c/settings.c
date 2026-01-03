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
    
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        settings->visualization_enabled[i] = true;
    }
    colors_load_default_palettes(settings->visualization_colors);
}

// =============================================================================
// Settings Validation
// =============================================================================

void settings_validate(TimerSettings *settings) {
    // Validate display mode (enum is unsigned, so only check upper bound)
    if (settings->default_display_mode >= DISPLAY_MODE_COUNT) {
        settings->default_display_mode = DISPLAY_MODE_TEXT;
    }
    
    // Ensure at least one visualization is enabled
    bool any_enabled = false;
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        if (settings->visualization_enabled[i]) {
            any_enabled = true;
            break;
        }
    }
    if (!any_enabled) {
        for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
            settings->visualization_enabled[i] = true;
        }
    }
    
    // If the default mode is disabled, pick the first enabled one
    if (!settings->visualization_enabled[settings->default_display_mode]) {
        for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
            if (settings->visualization_enabled[i]) {
                settings->default_display_mode = (DisplayMode)i;
                break;
            }
        }
    }
    
    // Validate visualization colors - reload defaults if background matches primary
    // (which would make the display invisible/blank)
    VisualizationColors default_palettes[DISPLAY_MODE_COUNT];
    colors_load_default_palettes(default_palettes);
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        VisualizationColors *colors = &settings->visualization_colors[i];
        // Check if background matches primary (invisible) or if colors look uninitialized
        // GColorWhite on black background is fine, but same color = invisible
        if (gcolor_equal(colors->background, colors->primary)) {
            settings->visualization_colors[i] = default_palettes[i];
        }
    }
    
    // Validate preset index (0-3 for presets, 4 for custom)
    if (settings->default_preset_index > TIMER_CUSTOM_OPTION) {
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
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        ctx->display_mode_enabled[i] = settings->visualization_enabled[i];
    }
    ctx->selected_preset = settings->default_preset_index;
    ctx->hide_time_text = settings->hide_time_text;
    
    // Set custom time from settings if custom option is selected
    if (settings->default_preset_index == TIMER_CUSTOM_OPTION) {
        ctx->custom_hours = settings->default_custom_minutes / 60;
        ctx->custom_minutes = settings->default_custom_minutes % 60;
    }
}

void settings_update_from_context(TimerSettings *settings, const TimerContext *ctx) {
    settings->hide_time_text = ctx->hide_time_text;
    // Note: preset index and custom time are not auto-saved from context
    // They are saved when the user explicitly changes the default
}

// =============================================================================
// Settings Persistence Helpers
// =============================================================================

static bool settings_load_blob(TimerSettings *settings) {
    if (!persist_exists(SETTINGS_KEY_DATA)) {
        return false;
    }
    
    int bytes_read = persist_read_data(SETTINGS_KEY_DATA, settings, sizeof(TimerSettings));
    return bytes_read == (int)sizeof(TimerSettings);
}

static void settings_load_legacy_v1(TimerSettings *settings) {
    if (persist_exists(SETTINGS_KEY_DISPLAY_MODE)) {
        settings->default_display_mode = persist_read_int(SETTINGS_KEY_DISPLAY_MODE);
    }
    if (persist_exists(SETTINGS_KEY_DEFAULT_TIME)) {
        settings->default_preset_index = persist_read_int(SETTINGS_KEY_DEFAULT_TIME);
    }
    if (persist_exists(SETTINGS_KEY_HIDE_TIME)) {
        settings->hide_time_text = persist_read_bool(SETTINGS_KEY_HIDE_TIME);
    }
}

// =============================================================================
// Settings Persistence API
// =============================================================================

void settings_persist_load(TimerSettings *settings) {
    settings_init_defaults(settings);
    
    if (persist_exists(SETTINGS_KEY_VERSION)) {
        int version = persist_read_int(SETTINGS_KEY_VERSION);
        
        if (version == SETTINGS_VERSION) {
            if (!settings_load_blob(settings)) {
                settings_init_defaults(settings);
            }
        } else if (version == 1) {
            settings_load_legacy_v1(settings);
        }
    }
    
    settings_validate(settings);
}

void settings_persist_save(TimerSettings *settings) {
    settings_validate(settings);
    persist_write_int(SETTINGS_KEY_VERSION, SETTINGS_VERSION);
    persist_write_data(SETTINGS_KEY_DATA, settings, sizeof(TimerSettings));
}

