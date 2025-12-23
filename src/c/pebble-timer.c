#include <pebble.h>

// =============================================================================
// App States
// =============================================================================

typedef enum {
  STATE_SELECT_PRESET,
  STATE_SET_CUSTOM_HOURS,
  STATE_SET_CUSTOM_MINUTES,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_COMPLETED,
  STATE_CONFIRM_EXIT
} AppState;

// =============================================================================
// Display Modes
// =============================================================================

typedef enum {
  DISPLAY_MODE_TEXT,
  DISPLAY_MODE_BLOCKS,
  DISPLAY_MODE_CLOCK,
  DISPLAY_MODE_COUNT  // Used for cycling
} DisplayMode;

// =============================================================================
// Preset Timer Options (in minutes)
// =============================================================================

static const int PRESETS[] = {5, 10, 15, 30};
static const int NUM_PRESETS = 4;
static const int CUSTOM_OPTION = 4;  // Index for "Custom" option

// =============================================================================
// Global Variables
// =============================================================================

static Window *s_main_window;
static TextLayer *s_title_layer;
static TextLayer *s_time_layer;
static TextLayer *s_hint_layer;
static Layer *s_canvas_layer;

static AppState s_state = STATE_SELECT_PRESET;
static DisplayMode s_display_mode = DISPLAY_MODE_TEXT;
static int s_selected_preset = 0;
static int s_custom_hours = 0;
static int s_custom_minutes = 5;
static int s_remaining_seconds = 0;
static int s_total_seconds = 0;

static AppTimer *s_vibrate_timer = NULL;

// =============================================================================
// Color Definitions (platform-aware)
// =============================================================================

#ifdef PBL_COLOR
  #define COLOR_BACKGROUND GColorBlack
  #define COLOR_TEXT_NORMAL GColorWhite
  #define COLOR_TEXT_RUNNING GColorGreen
  #define COLOR_TEXT_PAUSED GColorYellow
  #define COLOR_TEXT_LOW GColorRed
  #define COLOR_TEXT_COMPLETED GColorBrightGreen
  #define COLOR_HINT GColorLightGray
  #define COLOR_BLOCKS_FULL GColorVividCerulean
  #define COLOR_BLOCKS_EMPTY GColorDarkGray
  #define COLOR_CLOCK_FACE GColorWhite
  #define COLOR_CLOCK_HAND GColorRed
  #define COLOR_CLOCK_REMAINING GColorMelon
#else
  #define COLOR_BACKGROUND GColorBlack
  #define COLOR_TEXT_NORMAL GColorWhite
  #define COLOR_TEXT_RUNNING GColorWhite
  #define COLOR_TEXT_PAUSED GColorWhite
  #define COLOR_TEXT_LOW GColorWhite
  #define COLOR_TEXT_COMPLETED GColorWhite
  #define COLOR_HINT GColorWhite
  #define COLOR_BLOCKS_FULL GColorWhite
  #define COLOR_BLOCKS_EMPTY GColorBlack
  #define COLOR_CLOCK_FACE GColorWhite
  #define COLOR_CLOCK_HAND GColorWhite
  #define COLOR_CLOCK_REMAINING GColorWhite
#endif

// =============================================================================
// Forward Declarations
// =============================================================================

static void update_display(void);
static void start_vibration_loop(void);
static void stop_vibration_loop(void);

// =============================================================================
// Time Formatting
// =============================================================================

static void format_time_adaptive(int total_seconds, char *buffer, size_t buffer_size) {
  int hours = total_seconds / 3600;
  int minutes = (total_seconds % 3600) / 60;
  int seconds = total_seconds % 60;
  
  if (hours > 0) {
    snprintf(buffer, buffer_size, "%d:%02d:%02d", hours, minutes, seconds);
  } else {
    snprintf(buffer, buffer_size, "%d:%02d", minutes, seconds);
  }
}

static void format_preset_option(int index, char *buffer, size_t buffer_size) {
  if (index < NUM_PRESETS) {
    snprintf(buffer, buffer_size, "%d min", PRESETS[index]);
  } else {
    snprintf(buffer, buffer_size, "Custom");
  }
}

static const char* get_display_mode_name(DisplayMode mode) {
  switch (mode) {
    case DISPLAY_MODE_TEXT: return "Text";
    case DISPLAY_MODE_BLOCKS: return "Blocks";
    case DISPLAY_MODE_CLOCK: return "Clock";
    default: return "Text";
  }
}

// =============================================================================
// Vibration Handling
// =============================================================================

static void vibrate_callback(void *data) {
  if (s_state == STATE_COMPLETED) {
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
// Canvas Drawing - Blocks Mode
// =============================================================================

#define BLOCK_COLS 12
#define BLOCK_ROWS 8
#define BLOCK_PADDING 2

static void draw_blocks_mode(GContext *ctx, GRect bounds) {
  // Calculate block size based on available space
  int available_width = bounds.size.w - 20;
  int available_height = bounds.size.h - 60;
  
  int block_width = (available_width - (BLOCK_COLS - 1) * BLOCK_PADDING) / BLOCK_COLS;
  int block_height = (available_height - (BLOCK_ROWS - 1) * BLOCK_PADDING) / BLOCK_ROWS;
  
  // Ensure square blocks
  int block_size = (block_width < block_height) ? block_width : block_height;
  
  // Calculate starting position to center the grid
  int grid_width = BLOCK_COLS * block_size + (BLOCK_COLS - 1) * BLOCK_PADDING;
  int grid_height = BLOCK_ROWS * block_size + (BLOCK_ROWS - 1) * BLOCK_PADDING;
  int start_x = (bounds.size.w - grid_width) / 2;
  int start_y = (bounds.size.h - grid_height) / 2 - 10;
  
  // Calculate how many blocks should be filled
  int total_blocks = BLOCK_COLS * BLOCK_ROWS;
  int filled_blocks = 0;
  if (s_total_seconds > 0) {
    filled_blocks = (s_remaining_seconds * total_blocks) / s_total_seconds;
  }
  
  // Draw blocks from bottom-right to top-left (emptying pattern)
  for (int row = 0; row < BLOCK_ROWS; row++) {
    for (int col = 0; col < BLOCK_COLS; col++) {
      int block_index = (BLOCK_ROWS - 1 - row) * BLOCK_COLS + (BLOCK_COLS - 1 - col);
      
      int x = start_x + col * (block_size + BLOCK_PADDING);
      int y = start_y + row * (block_size + BLOCK_PADDING);
      
      GRect block_rect = GRect(x, y, block_size, block_size);
      
      if (block_index < filled_blocks) {
        // Block is filled (time remaining)
        graphics_context_set_fill_color(ctx, COLOR_BLOCKS_FULL);
        graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
      } else {
        // Block is empty (time passed)
        graphics_context_set_stroke_color(ctx, COLOR_BLOCKS_EMPTY);
        graphics_draw_round_rect(ctx, block_rect, 2);
      }
    }
  }
  
  // Draw time text below the blocks
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect text_rect = GRect(0, start_y + grid_height + 5, bounds.size.w, 30);
  
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect, 
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Clock Mode
// =============================================================================

static void draw_clock_mode(GContext *ctx, GRect bounds) {
  // Calculate center and radius
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2 - 10;
  int radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 20;
  
  GPoint center = GPoint(center_x, center_y);
  
  // Draw outer circle
  graphics_context_set_stroke_color(ctx, COLOR_CLOCK_FACE);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, radius);
  
  // Draw tick marks for 12 positions (like a clock)
  for (int i = 0; i < 12; i++) {
    int angle = (i * 360 / 12) - 90;  // Start from top (12 o'clock)
    int angle_rad_x = (angle * TRIG_MAX_ANGLE) / 360;
    
    int inner_r = radius - 8;
    int outer_r = radius - 3;
    
    int x1 = center_x + (cos_lookup(angle_rad_x) * inner_r / TRIG_MAX_RATIO);
    int y1 = center_y + (sin_lookup(angle_rad_x) * inner_r / TRIG_MAX_RATIO);
    int x2 = center_x + (cos_lookup(angle_rad_x) * outer_r / TRIG_MAX_RATIO);
    int y2 = center_y + (sin_lookup(angle_rad_x) * outer_r / TRIG_MAX_RATIO);
    
    graphics_context_set_stroke_width(ctx, (i % 3 == 0) ? 3 : 1);
    graphics_draw_line(ctx, GPoint(x1, y1), GPoint(x2, y2));
  }
  
  // Draw remaining time arc (filled pie slice)
  if (s_remaining_seconds > 0 && s_total_seconds > 0) {
    graphics_context_set_fill_color(ctx, COLOR_CLOCK_REMAINING);
    
    // Draw as filled segments for the remaining time
    int segments = 60;  // Draw in 60 segments for smoothness
    int filled_segments = (s_remaining_seconds * segments) / s_total_seconds;
    
    for (int i = 0; i < filled_segments; i++) {
      // Calculate angle for this segment (starting from top, going clockwise)
      int32_t seg_angle = -TRIG_MAX_ANGLE / 4 + (i * TRIG_MAX_ANGLE / segments);
      int32_t next_angle = -TRIG_MAX_ANGLE / 4 + ((i + 1) * TRIG_MAX_ANGLE / segments);
      
      int inner_r = radius / 3;
      int outer_r = radius - 12;
      
      // Draw small filled arc segment
      GPoint p1 = GPoint(
        center_x + (cos_lookup(seg_angle) * inner_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(seg_angle) * inner_r / TRIG_MAX_RATIO)
      );
      GPoint p2 = GPoint(
        center_x + (cos_lookup(seg_angle) * outer_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(seg_angle) * outer_r / TRIG_MAX_RATIO)
      );
      GPoint p3 = GPoint(
        center_x + (cos_lookup(next_angle) * outer_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(next_angle) * outer_r / TRIG_MAX_RATIO)
      );
      // Draw as lines to form segment
      graphics_context_set_stroke_color(ctx, COLOR_CLOCK_REMAINING);
      graphics_context_set_stroke_width(ctx, 3);
      graphics_draw_line(ctx, p1, p2);
      graphics_draw_line(ctx, p2, p3);
      graphics_draw_line(ctx, p3, p1);
    }
  }
  
  // Draw center dot
  graphics_context_set_fill_color(ctx, COLOR_CLOCK_FACE);
  graphics_fill_circle(ctx, center, 5);
  
  // Draw "hand" pointing to current remaining time position
  if (s_total_seconds > 0) {
    int32_t hand_angle = -TRIG_MAX_ANGLE / 4;  // Start at top
    if (s_remaining_seconds < s_total_seconds) {
      // Calculate where the hand should point based on remaining time
      hand_angle = -TRIG_MAX_ANGLE / 4 + 
                   ((s_total_seconds - s_remaining_seconds) * TRIG_MAX_ANGLE / s_total_seconds);
    }
    
    int hand_length = radius - 15;
    GPoint hand_end = GPoint(
      center_x + (cos_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO),
      center_y + (sin_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO)
    );
    
    graphics_context_set_stroke_color(ctx, COLOR_CLOCK_HAND);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, center, hand_end);
  }
  
  // Draw time text in center
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect text_rect = GRect(center_x - 40, center_y + radius + 5, 80, 24);
  
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Update Procedure
// =============================================================================

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Only draw custom modes when running or paused with timer active
  if (s_state != STATE_RUNNING && s_state != STATE_PAUSED) {
    return;
  }
  
  // Clear the canvas
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  switch (s_display_mode) {
    case DISPLAY_MODE_BLOCKS:
      draw_blocks_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_CLOCK:
      draw_clock_mode(ctx, bounds);
      break;
    default:
      // Text mode - don't draw anything, let text layers show
      break;
  }
}

// =============================================================================
// Timer Tick Handler
// =============================================================================

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (s_state == STATE_RUNNING) {
    s_remaining_seconds--;
    
    if (s_remaining_seconds <= 0) {
      s_remaining_seconds = 0;
      s_state = STATE_COMPLETED;
      start_vibration_loop();
    }
    
    update_display();
  }
}

// =============================================================================
// Display Update
// =============================================================================

static void update_display(void) {
  static char title_buf[32];
  static char time_buf[16];
  static char hint_buf[64];
  
  // Set background color
  window_set_background_color(s_main_window, COLOR_BACKGROUND);
  
  // Determine if we should show canvas or text layers
  bool show_canvas = (s_state == STATE_RUNNING || s_state == STATE_PAUSED) && 
                     s_display_mode != DISPLAY_MODE_TEXT;
  
  layer_set_hidden(s_canvas_layer, !show_canvas);
  layer_set_hidden(text_layer_get_layer(s_time_layer), show_canvas);
  layer_set_hidden(text_layer_get_layer(s_title_layer), show_canvas && s_state == STATE_RUNNING);
  layer_set_hidden(text_layer_get_layer(s_hint_layer), show_canvas && s_state == STATE_RUNNING);
  
  if (show_canvas) {
    layer_mark_dirty(s_canvas_layer);
  }
  
  switch (s_state) {
    case STATE_SELECT_PRESET:
      snprintf(title_buf, sizeof(title_buf), "Select Time");
      format_preset_option(s_selected_preset, time_buf, sizeof(time_buf));
      snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Change\nSELECT: Start\nHold: %s", 
               get_display_mode_name(s_display_mode));
      text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
      break;
      
    case STATE_SET_CUSTOM_HOURS:
      snprintf(title_buf, sizeof(title_buf), "Set Hours");
      snprintf(time_buf, sizeof(time_buf), "%d hr", s_custom_hours);
      snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Adjust\nSELECT: Next");
      text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
      break;
      
    case STATE_SET_CUSTOM_MINUTES:
      snprintf(title_buf, sizeof(title_buf), "Set Minutes");
      snprintf(time_buf, sizeof(time_buf), "%d min", s_custom_minutes);
      snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Adjust\nSELECT: Start");
      text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
      break;
      
    case STATE_RUNNING:
      title_buf[0] = '\0';
      format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
      hint_buf[0] = '\0';
      
      // Color based on remaining time
      if (s_remaining_seconds <= 10) {
        text_layer_set_text_color(s_time_layer, COLOR_TEXT_LOW);
      } else {
        text_layer_set_text_color(s_time_layer, COLOR_TEXT_RUNNING);
      }
      break;
      
    case STATE_PAUSED:
      snprintf(title_buf, sizeof(title_buf), "Paused");
      format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
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
// Timer Control Functions
// =============================================================================

static void start_timer(int minutes) {
  s_total_seconds = minutes * 60;
  s_remaining_seconds = s_total_seconds;
  s_state = STATE_RUNNING;
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_display();
}

static void start_custom_timer(void) {
  int total_minutes = (s_custom_hours * 60) + s_custom_minutes;
  if (total_minutes > 0) {
    start_timer(total_minutes);
  }
}

static void pause_timer(void) {
  s_state = STATE_PAUSED;
  update_display();
}

static void resume_timer(void) {
  s_state = STATE_RUNNING;
  update_display();
}

static void cancel_timer(void) {
  tick_timer_service_unsubscribe();
  s_state = STATE_SELECT_PRESET;
  s_remaining_seconds = 0;
  update_display();
}

static void restart_timer(void) {
  s_remaining_seconds = s_total_seconds;
  s_state = STATE_RUNNING;
  update_display();
}

static void dismiss_completion(void) {
  stop_vibration_loop();
  tick_timer_service_unsubscribe();
  s_state = STATE_SELECT_PRESET;
  update_display();
}

static void cycle_display_mode(void) {
  s_display_mode = (s_display_mode + 1) % DISPLAY_MODE_COUNT;
  vibes_short_pulse();  // Haptic feedback for mode change
  update_display();
}

// =============================================================================
// Button Click Handlers
// =============================================================================

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_state) {
    case STATE_SELECT_PRESET:
      if (s_selected_preset < NUM_PRESETS) {
        start_timer(PRESETS[s_selected_preset]);
      } else {
        // Custom option selected
        s_state = STATE_SET_CUSTOM_HOURS;
        update_display();
      }
      break;
      
    case STATE_SET_CUSTOM_HOURS:
      s_state = STATE_SET_CUSTOM_MINUTES;
      update_display();
      break;
      
    case STATE_SET_CUSTOM_MINUTES:
      start_custom_timer();
      break;
      
    case STATE_RUNNING:
      pause_timer();
      break;
      
    case STATE_PAUSED:
      resume_timer();
      break;
      
    case STATE_COMPLETED:
      dismiss_completion();
      break;
      
    case STATE_CONFIRM_EXIT:
      // Do nothing on select in confirm dialog
      break;
  }
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Long press on select cycles display mode (only in preset selection or running/paused)
  if (s_state == STATE_SELECT_PRESET || s_state == STATE_RUNNING || s_state == STATE_PAUSED) {
    cycle_display_mode();
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_state) {
    case STATE_SELECT_PRESET:
      s_selected_preset--;
      if (s_selected_preset < 0) {
        s_selected_preset = CUSTOM_OPTION;
      }
      update_display();
      break;
      
    case STATE_SET_CUSTOM_HOURS:
      s_custom_hours++;
      if (s_custom_hours > 23) {
        s_custom_hours = 0;
      }
      update_display();
      break;
      
    case STATE_SET_CUSTOM_MINUTES:
      s_custom_minutes++;
      if (s_custom_minutes > 59) {
        s_custom_minutes = 0;
      }
      update_display();
      break;
      
    case STATE_PAUSED:
      restart_timer();
      break;
      
    case STATE_COMPLETED:
      dismiss_completion();
      break;
      
    case STATE_CONFIRM_EXIT:
      // Yes, exit
      cancel_timer();
      window_stack_pop(true);
      break;
      
    default:
      break;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_state) {
    case STATE_SELECT_PRESET:
      s_selected_preset++;
      if (s_selected_preset > CUSTOM_OPTION) {
        s_selected_preset = 0;
      }
      update_display();
      break;
      
    case STATE_SET_CUSTOM_HOURS:
      s_custom_hours--;
      if (s_custom_hours < 0) {
        s_custom_hours = 23;
      }
      update_display();
      break;
      
    case STATE_SET_CUSTOM_MINUTES:
      s_custom_minutes--;
      if (s_custom_minutes < 0) {
        s_custom_minutes = 59;
      }
      update_display();
      break;
      
    case STATE_PAUSED:
      cancel_timer();
      break;
      
    case STATE_COMPLETED:
      dismiss_completion();
      break;
      
    case STATE_CONFIRM_EXIT:
      // No, stay - go back to paused
      s_state = STATE_PAUSED;
      update_display();
      break;
      
    default:
      break;
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_state) {
    case STATE_RUNNING:
    case STATE_PAUSED:
      // Prompt before exiting
      if (s_state == STATE_RUNNING) {
        pause_timer();
      }
      s_state = STATE_CONFIRM_EXIT;
      update_display();
      break;
      
    case STATE_SET_CUSTOM_HOURS:
    case STATE_SET_CUSTOM_MINUTES:
      // Go back to preset selection
      s_state = STATE_SELECT_PRESET;
      update_display();
      break;
      
    case STATE_CONFIRM_EXIT:
      // No, stay - go back to paused
      s_state = STATE_PAUSED;
      update_display();
      break;
      
    case STATE_COMPLETED:
      dismiss_completion();
      break;
      
    default:
      // Normal exit
      window_stack_pop(true);
      break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  
  // Long press on SELECT to cycle display mode
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

// =============================================================================
// Window Load/Unload
// =============================================================================

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Adjust layout for round displays (Chalk)
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
  
  // Canvas layer for custom drawing modes (full screen)
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_hidden(s_canvas_layer, true);  // Hidden by default
  layer_add_child(window_layer, s_canvas_layer);
  
  // Title layer
  s_title_layer = text_layer_create(GRect(inset, title_y, bounds.size.w - (inset * 2), 30));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, COLOR_HINT);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));
  
  // Main time display layer
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
  
  // Initial display
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
  stop_vibration_loop();
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
