#include "timer_state.h"

// =============================================================================
// Context Initialization
// =============================================================================

void timer_context_init(TimerContext *ctx) {
    ctx->state = STATE_SELECT_PRESET;
    ctx->display_mode = DISPLAY_MODE_TEXT;
    
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        ctx->display_mode_enabled[i] = true;
    }
    
    ctx->remaining_seconds = 0;
    ctx->total_seconds = 0;
    ctx->selected_preset = 0;
    ctx->custom_hours = 0;
    ctx->custom_minutes = 5;
    ctx->hide_time_text = false;
}

TimerEffects timer_effects_none(void) {
    TimerEffects effects = {
        .update_display = false,
        .subscribe_tick_timer = false,
        .unsubscribe_tick_timer = false,
        .start_vibration = false,
        .stop_vibration = false,
        .init_hourglass = false,
        .init_matrix = false,
        .vibrate_short = false,
        .pop_window = false
    };
    return effects;
}

// =============================================================================
// State Queries
// =============================================================================

const char* timer_display_mode_name(DisplayMode mode) {
    switch (mode) {
        case DISPLAY_MODE_TEXT:            return "Text";
        case DISPLAY_MODE_BLOCKS:          return "Blocks";
        case DISPLAY_MODE_VERTICAL_BLOCKS: return "Vertical Blocks";
        case DISPLAY_MODE_CLOCK:           return "Clock";
        case DISPLAY_MODE_RING:            return "Ring";
        case DISPLAY_MODE_HOURGLASS:       return "Hourglass";
        case DISPLAY_MODE_BINARY:          return "Binary";
        case DISPLAY_MODE_RADIAL:          return "Radial";
        case DISPLAY_MODE_HEX:             return "Hex";
        case DISPLAY_MODE_MATRIX:          return "Matrix";
        case DISPLAY_MODE_WATER_LEVEL:     return "Water Level";
        case DISPLAY_MODE_SPIRAL_OUT:      return "Spiral Out";
        case DISPLAY_MODE_SPIRAL_IN:       return "Spiral In";
        case DISPLAY_MODE_PERCENT:         return "% Elapsed";
        case DISPLAY_MODE_PERCENT_REMAINING: return "% Remaining";
        default:                           return "Text";
    }
}

bool timer_is_active(const TimerContext *ctx) {
    return ctx->state == STATE_RUNNING || ctx->state == STATE_PAUSED;
}

bool timer_should_show_canvas(const TimerContext *ctx) {
    return (ctx->state == STATE_RUNNING || ctx->state == STATE_PAUSED) &&
           ctx->display_mode != DISPLAY_MODE_TEXT;
}

// =============================================================================
// Timer Actions
// =============================================================================

TimerEffects timer_start(TimerContext *ctx, int minutes) {
    TimerEffects effects = timer_effects_none();
    
    if (minutes <= 0) {
        return effects;
    }
    
    ctx->total_seconds = minutes * 60;
    ctx->remaining_seconds = ctx->total_seconds;
    ctx->state = STATE_RUNNING;
    
    effects.subscribe_tick_timer = true;
    effects.update_display = true;
    effects.init_hourglass = true;
    effects.init_matrix = true;
    
    return effects;
}

TimerEffects timer_tick(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    if (ctx->state != STATE_RUNNING) {
        return effects;
    }
    
    ctx->remaining_seconds--;
    effects.update_display = true;
    
    if (ctx->remaining_seconds <= 0) {
        ctx->remaining_seconds = 0;
        ctx->state = STATE_COMPLETED;
        effects.start_vibration = true;
    }
    
    return effects;
}

TimerEffects timer_pause(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    if (ctx->state == STATE_RUNNING) {
        ctx->state = STATE_PAUSED;
        effects.update_display = true;
    }
    
    return effects;
}

TimerEffects timer_resume(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    if (ctx->state == STATE_PAUSED) {
        ctx->state = STATE_RUNNING;
        effects.update_display = true;
    }
    
    return effects;
}

TimerEffects timer_cancel(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    ctx->state = STATE_SELECT_PRESET;
    ctx->remaining_seconds = 0;
    
    effects.unsubscribe_tick_timer = true;
    effects.update_display = true;
    
    return effects;
}

TimerEffects timer_restart(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    // Stop vibration if restarting from completed state
    if (ctx->state == STATE_COMPLETED) {
        effects.stop_vibration = true;
    }
    
    ctx->remaining_seconds = ctx->total_seconds;
    ctx->state = STATE_RUNNING;
    
    effects.update_display = true;
    effects.init_hourglass = true;
    effects.init_matrix = true;
    
    return effects;
}

TimerEffects timer_dismiss_completion(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    ctx->state = STATE_SELECT_PRESET;
    
    effects.stop_vibration = true;
    effects.unsubscribe_tick_timer = true;
    effects.update_display = true;
    
    return effects;
}

TimerEffects timer_cycle_display_mode(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    // If the enabled mask is empty (e.g., uninitialized), treat all as enabled
    bool any_enabled = false;
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        if (ctx->display_mode_enabled[i]) {
            any_enabled = true;
            break;
        }
    }
    if (!any_enabled) {
        for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
            ctx->display_mode_enabled[i] = true;
        }
        any_enabled = true;
    }
    
    // Find the next enabled mode (skip the current one)
    DisplayMode next_mode = ctx->display_mode;
    for (int step = 0; step < DISPLAY_MODE_COUNT; step++) {
        next_mode = (next_mode + 1) % DISPLAY_MODE_COUNT;
        if (ctx->display_mode_enabled[next_mode]) {
            break;
        }
    }
    ctx->display_mode = next_mode;
    
    effects.vibrate_short = true;
    effects.update_display = true;
    
    return effects;
}

TimerEffects timer_toggle_hide_time_text(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    ctx->hide_time_text = !ctx->hide_time_text;
    
    effects.vibrate_short = true;
    effects.update_display = true;
    
    return effects;
}

// =============================================================================
// Input Handling - SELECT Button
// =============================================================================

TimerEffects timer_handle_select(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    switch (ctx->state) {
        case STATE_SELECT_PRESET:
            if (ctx->selected_preset < TIMER_PRESETS_COUNT) {
                return timer_start(ctx, TIMER_PRESETS[ctx->selected_preset]);
            } else {
                // Custom timer selected
                ctx->state = STATE_SET_CUSTOM_HOURS;
                effects.update_display = true;
            }
            break;
            
        case STATE_SET_CUSTOM_HOURS:
            ctx->state = STATE_SET_CUSTOM_MINUTES;
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_MINUTES: {
            int total_minutes = (ctx->custom_hours * 60) + ctx->custom_minutes;
            if (total_minutes > 0) {
                return timer_start(ctx, total_minutes);
            }
            break;
        }
            
        case STATE_RUNNING:
            // No action on select when running (use DOWN to pause)
            break;
            
        case STATE_PAUSED:
            // No action on select when paused (use DOWN to resume)
            break;
            
        case STATE_COMPLETED:
            return timer_restart(ctx);
            
        case STATE_CONFIRM_EXIT:
            // Do nothing on select in confirm state
            break;
    }
    
    return effects;
}

TimerEffects timer_handle_select_long(TimerContext *ctx) {
    if (ctx->state == STATE_SELECT_PRESET ||
        ctx->state == STATE_RUNNING ||
        ctx->state == STATE_PAUSED) {
        return timer_cycle_display_mode(ctx);
    }
    
    return timer_effects_none();
}

TimerEffects timer_handle_up_long(TimerContext *ctx) {
    // Toggle hide time text in running/paused states
    if (ctx->state == STATE_RUNNING || ctx->state == STATE_PAUSED) {
        return timer_toggle_hide_time_text(ctx);
    }
    
    return timer_effects_none();
}

// =============================================================================
// Input Handling - UP Button
// =============================================================================

TimerEffects timer_handle_up(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    switch (ctx->state) {
        case STATE_SELECT_PRESET:
            ctx->selected_preset = decrement_wrap(ctx->selected_preset, TIMER_CUSTOM_OPTION);
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_HOURS:
            ctx->custom_hours = increment_wrap(ctx->custom_hours, 23);
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_MINUTES:
            ctx->custom_minutes = increment_wrap(ctx->custom_minutes, 59);
            effects.update_display = true;
            break;
            
        case STATE_PAUSED:
            return timer_restart(ctx);
            
        case STATE_COMPLETED:
            return timer_restart(ctx);
            
        case STATE_CONFIRM_EXIT:
            effects = timer_cancel(ctx);
            effects.pop_window = true;
            break;
            
        case STATE_RUNNING:
            // No action
            break;
    }
    
    return effects;
}

// =============================================================================
// Input Handling - DOWN Button
// =============================================================================

TimerEffects timer_handle_down(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    switch (ctx->state) {
        case STATE_SELECT_PRESET:
            ctx->selected_preset = increment_wrap(ctx->selected_preset, TIMER_CUSTOM_OPTION);
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_HOURS:
            ctx->custom_hours = decrement_wrap(ctx->custom_hours, 23);
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_MINUTES:
            ctx->custom_minutes = decrement_wrap(ctx->custom_minutes, 59);
            effects.update_display = true;
            break;
            
        case STATE_PAUSED:
            return timer_resume(ctx);
            
        case STATE_COMPLETED:
            return timer_dismiss_completion(ctx);
            
        case STATE_CONFIRM_EXIT:
            ctx->state = STATE_PAUSED;
            effects.update_display = true;
            break;
            
        case STATE_RUNNING:
            return timer_pause(ctx);
    }
    
    return effects;
}

// =============================================================================
// Input Handling - BACK Button
// =============================================================================

TimerEffects timer_handle_back(TimerContext *ctx) {
    TimerEffects effects = timer_effects_none();
    
    switch (ctx->state) {
        case STATE_RUNNING:
            timer_pause(ctx);
            ctx->state = STATE_CONFIRM_EXIT;
            effects.update_display = true;
            break;
            
        case STATE_PAUSED:
            ctx->state = STATE_CONFIRM_EXIT;
            effects.update_display = true;
            break;
            
        case STATE_SET_CUSTOM_HOURS:
        case STATE_SET_CUSTOM_MINUTES:
            ctx->state = STATE_SELECT_PRESET;
            effects.update_display = true;
            break;
            
        case STATE_CONFIRM_EXIT:
            ctx->state = STATE_PAUSED;
            effects.update_display = true;
            break;
            
        case STATE_COMPLETED:
            return timer_dismiss_completion(ctx);
            
        case STATE_SELECT_PRESET:
            effects.pop_window = true;
            break;
    }
    
    return effects;
}

