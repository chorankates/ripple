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

static AppState s_state = STATE_SELECT_PRESET;
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
#else
  #define COLOR_BACKGROUND GColorBlack
  #define COLOR_TEXT_NORMAL GColorWhite
  #define COLOR_TEXT_RUNNING GColorWhite
  #define COLOR_TEXT_PAUSED GColorWhite
  #define COLOR_TEXT_LOW GColorWhite
  #define COLOR_TEXT_COMPLETED GColorWhite
  #define COLOR_HINT GColorWhite
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
  static char hint_buf[48];
  
  // Set background color
  window_set_background_color(s_main_window, COLOR_BACKGROUND);
  
  switch (s_state) {
    case STATE_SELECT_PRESET:
      snprintf(title_buf, sizeof(title_buf), "Select Time");
      format_preset_option(s_selected_preset, time_buf, sizeof(time_buf));
      snprintf(hint_buf, sizeof(hint_buf), "UP/DOWN: Change\nSELECT: Start");
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
