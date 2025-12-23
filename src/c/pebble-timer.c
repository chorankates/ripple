#include <pebble.h>
#include "colors.h"
#include "time_utils.h"
#include "timer_state.h"
#include "settings.h"
#include "display/display_common.h"

// =============================================================================
// Pebble Timer - SDK Integration Layer
// =============================================================================
// This file contains only Pebble SDK integration code.
// All business logic is in timer_state.c (testable without SDK).
// All display rendering is in display/display_modes.c.

// =============================================================================
// Global State
// =============================================================================

static Window *s_main_window;
static TextLayer *s_title_layer;
static TextLayer *s_time_layer;
static TextLayer *s_hint_layer;
static Layer *s_canvas_layer;

static TimerContext s_timer_ctx;
static TimerSettings s_settings;
static AnimationState s_anim_state;
static AppTimer *s_vibrate_timer = NULL;

// =============================================================================
// Forward Declarations
// =============================================================================

static void update_display(void);
static void apply_effects(TimerEffects effects);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

// =============================================================================
// Settings Persistence
// =============================================================================

static void settings_load(void) {
    // Initialize with defaults
    settings_init_defaults(&s_settings);
    
    // Check if we have saved settings
    if (persist_exists(SETTINGS_KEY_VERSION)) {
        int version = persist_read_int(SETTINGS_KEY_VERSION);
        if (version == SETTINGS_VERSION) {
            // Load saved settings
            if (persist_exists(SETTINGS_KEY_DISPLAY_MODE)) {
                s_settings.default_display_mode = persist_read_int(SETTINGS_KEY_DISPLAY_MODE);
            }
            if (persist_exists(SETTINGS_KEY_DEFAULT_TIME)) {
                s_settings.default_preset_index = persist_read_int(SETTINGS_KEY_DEFAULT_TIME);
            }
            if (persist_exists(SETTINGS_KEY_HIDE_TIME)) {
                s_settings.hide_time_text = persist_read_bool(SETTINGS_KEY_HIDE_TIME);
            }
        }
    }
    
    // Validate loaded settings
    settings_validate(&s_settings);
}

static void settings_save(void) {
    // Update settings from current context
    settings_update_from_context(&s_settings, &s_timer_ctx);
    
    // Write to persistent storage
    persist_write_int(SETTINGS_KEY_VERSION, SETTINGS_VERSION);
    persist_write_int(SETTINGS_KEY_DISPLAY_MODE, s_settings.default_display_mode);
    persist_write_int(SETTINGS_KEY_DEFAULT_TIME, s_settings.default_preset_index);
    persist_write_bool(SETTINGS_KEY_HIDE_TIME, s_settings.hide_time_text);
}

// =============================================================================
// Vibration Handling
// =============================================================================

static void vibrate_callback(void *data) {
    if (s_timer_ctx.state == STATE_COMPLETED) {
        vibes_short_pulse();
        s_vibrate_timer = app_timer_register(1000, vibrate_callback, NULL);
    }
}

static void start_vibration_loop(void) {
    vibes_long_pulse();
    s_vibrate_timer = app_timer_register(1000, vibrate_callback, NULL);
}

static void stop_vibration_loop(void) {
    if (s_vibrate_timer) {
        app_timer_cancel(s_vibrate_timer);
        s_vibrate_timer = NULL;
    }
    vibes_cancel();
}

// =============================================================================
// Effect Application - Translates Pure Logic Effects to SDK Calls
// =============================================================================

static void apply_effects(TimerEffects effects) {
    if (effects.init_hourglass) {
        animation_init_hourglass(&s_anim_state.hourglass);
    }
    
    if (effects.init_matrix) {
        animation_init_matrix(&s_anim_state.matrix, s_timer_ctx.remaining_seconds);
    }
    
    if (effects.subscribe_tick_timer) {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    }
    
    if (effects.unsubscribe_tick_timer) {
        tick_timer_service_unsubscribe();
    }
    
    if (effects.start_vibration) {
        start_vibration_loop();
    }
    
    if (effects.stop_vibration) {
        stop_vibration_loop();
    }
    
    if (effects.vibrate_short) {
        vibes_short_pulse();
    }
    
    if (effects.update_display) {
        update_display();
    }
    
    if (effects.pop_window) {
        window_stack_pop(true);
    }
}

// =============================================================================
// Timer Tick Handler
// =============================================================================

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    TimerEffects effects = timer_tick(&s_timer_ctx);
    apply_effects(effects);
}

// =============================================================================
// Canvas Update Procedure
// =============================================================================

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    
    if (!timer_should_show_canvas(&s_timer_ctx)) {
        return;
    }
    
    display_draw(ctx, bounds, &s_timer_ctx, &s_anim_state);
}

// =============================================================================
// Display Update
// =============================================================================

static void update_display(void) {
    static char title_buf[32];
    static char time_buf[16];
    static char hint_buf[64];
    
    window_set_background_color(s_main_window, COLOR_BACKGROUND);
    
    bool show_canvas = timer_should_show_canvas(&s_timer_ctx);
    
    layer_set_hidden(s_canvas_layer, !show_canvas);
    layer_set_hidden(text_layer_get_layer(s_time_layer), show_canvas);
    layer_set_hidden(text_layer_get_layer(s_title_layer), 
                     show_canvas && s_timer_ctx.state == STATE_RUNNING);
    layer_set_hidden(text_layer_get_layer(s_hint_layer), 
                     show_canvas && s_timer_ctx.state == STATE_RUNNING);
    
    if (show_canvas) {
        layer_mark_dirty(s_canvas_layer);
    }
    
    switch (s_timer_ctx.state) {
        case STATE_SELECT_PRESET:
            snprintf(title_buf, sizeof(title_buf), "Select Time");
            time_format_preset(s_timer_ctx.selected_preset, time_buf, sizeof(time_buf));
            snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Change\nSELECT: Start\nHold: %s", 
                     timer_display_mode_name(s_timer_ctx.display_mode));
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
            break;
            
        case STATE_SET_CUSTOM_HOURS:
            snprintf(title_buf, sizeof(title_buf), "Set Hours");
            snprintf(time_buf, sizeof(time_buf), "%d hr", s_timer_ctx.custom_hours);
            snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Adjust\nSELECT: Next");
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
            break;
            
        case STATE_SET_CUSTOM_MINUTES:
            snprintf(title_buf, sizeof(title_buf), "Set Minutes");
            snprintf(time_buf, sizeof(time_buf), "%d min", s_timer_ctx.custom_minutes);
            snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Adjust\nSELECT: Start");
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
            break;
            
        case STATE_RUNNING:
            title_buf[0] = '\0';
            time_format_adaptive(s_timer_ctx.remaining_seconds, time_buf, sizeof(time_buf));
            hint_buf[0] = '\0';
            
            if (s_timer_ctx.remaining_seconds <= 10) {
                text_layer_set_text_color(s_time_layer, COLOR_TEXT_LOW);
            } else {
                text_layer_set_text_color(s_time_layer, COLOR_TEXT_RUNNING);
            }
            break;
            
        case STATE_PAUSED:
            snprintf(title_buf, sizeof(title_buf), "Paused");
            time_format_adaptive(s_timer_ctx.remaining_seconds, time_buf, sizeof(time_buf));
            snprintf(hint_buf, sizeof(hint_buf), "SELECT: Resume\nUP: Restart\nDOWN: Cancel");
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_PAUSED);
            break;
            
        case STATE_COMPLETED:
            snprintf(title_buf, sizeof(title_buf), "Complete!");
            snprintf(time_buf, sizeof(time_buf), "0:00");
            snprintf(hint_buf, sizeof(hint_buf), "Press any button");
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_COMPLETED);
            break;
            
        case STATE_CONFIRM_EXIT:
            snprintf(title_buf, sizeof(title_buf), "Timer Active!");
            snprintf(time_buf, sizeof(time_buf), "Exit?");
            snprintf(hint_buf, sizeof(hint_buf), "UP: Yes, exit\nDOWN: No, stay");
            text_layer_set_text_color(s_time_layer, COLOR_TEXT_PAUSED);
            break;
    }
    
    text_layer_set_text(s_title_layer, title_buf);
    text_layer_set_text(s_time_layer, time_buf);
    text_layer_set_text(s_hint_layer, hint_buf);
}

// =============================================================================
// Button Click Handlers
// =============================================================================

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_select(&s_timer_ctx);
    apply_effects(effects);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_select_long(&s_timer_ctx);
    apply_effects(effects);
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_up_long(&s_timer_ctx);
    apply_effects(effects);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_up(&s_timer_ctx);
    apply_effects(effects);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_down(&s_timer_ctx);
    apply_effects(effects);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
    TimerEffects effects = timer_handle_back(&s_timer_ctx);
    apply_effects(effects);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
    window_long_click_subscribe(BUTTON_ID_UP, 500, up_long_click_handler, NULL);
}

// =============================================================================
// Window Load/Unload
// =============================================================================

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    #ifdef PBL_ROUND
        int title_y = 30;
        int time_y = 55;
        int hint_y = 110;
        int inset = 20;
    #else
        int title_y = 15;
        int time_y = 45;
        int hint_y = 100;
        int inset = 5;
    #endif
    
    // Canvas layer for graphical display modes
    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_set_hidden(s_canvas_layer, true);
    layer_add_child(window_layer, s_canvas_layer);
    
    // Title layer
    s_title_layer = text_layer_create(GRect(inset, title_y, bounds.size.w - (inset * 2), 30));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, COLOR_HINT);
    text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_title_layer));
    
    // Time layer
    s_time_layer = text_layer_create(GRect(inset, time_y, bounds.size.w - (inset * 2), 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    
    // Hint layer
    s_hint_layer = text_layer_create(GRect(inset, hint_y, bounds.size.w - (inset * 2), 60));
    text_layer_set_background_color(s_hint_layer, GColorClear);
    text_layer_set_text_color(s_hint_layer, COLOR_HINT);
    text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_hint_layer));
    
    update_display();
}

static void window_unload(Window *window) {
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_hint_layer);
    layer_destroy(s_canvas_layer);
}

// =============================================================================
// App Init/Deinit
// =============================================================================

static void init(void) {
    // Load saved settings
    settings_load();
    
    // Initialize timer context with defaults
    timer_context_init(&s_timer_ctx);
    
    // Apply saved settings to context
    settings_apply_to_context(&s_settings, &s_timer_ctx);
    
    // Initialize animation state
    animation_init_hourglass(&s_anim_state.hourglass);
    animation_init_matrix(&s_anim_state.matrix, 0);
    
    // Create main window
    s_main_window = window_create();
    
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    
    window_set_background_color(s_main_window, COLOR_BACKGROUND);
    window_stack_push(s_main_window, true);
}

static void deinit(void) {
    // Save settings before exiting
    settings_save();
    
    stop_vibration_loop();
    tick_timer_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
