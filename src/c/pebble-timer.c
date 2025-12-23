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
  DISPLAY_MODE_RING,
  DISPLAY_MODE_HOURGLASS,
  DISPLAY_MODE_BINARY,
  DISPLAY_MODE_RADIAL,
  DISPLAY_MODE_HEX,
  DISPLAY_MODE_MATRIX,
  DISPLAY_MODE_WATER_LEVEL,
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

// Hourglass sand simulation
#define MAX_SAND_PARTICLES 48
static int s_sand_top[MAX_SAND_PARTICLES];     // Y positions in top chamber
static int s_sand_bottom[MAX_SAND_PARTICLES];  // Y positions in bottom chamber
static int s_num_sand_top = MAX_SAND_PARTICLES;
static int s_num_sand_bottom = 0;

// Matrix rain simulation
#define MATRIX_COLS 12
#define MATRIX_ROWS 10
static int s_matrix_drops[MATRIX_COLS];        // Y position of each drop head
static int s_matrix_chars[MATRIX_COLS][MATRIX_ROWS];  // Character at each position
static int s_matrix_speeds[MATRIX_COLS];       // Speed of each column

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
  #define COLOR_RING_FULL GColorCyan
  #define COLOR_RING_EMPTY GColorDarkGray
  #define COLOR_SAND GColorRajah
  #define COLOR_HOURGLASS GColorWhite
  #define COLOR_BINARY_ON GColorMintGreen
  #define COLOR_BINARY_OFF GColorDarkGray
  #define COLOR_RADIAL_HOURS GColorRed
  #define COLOR_RADIAL_MINUTES GColorOrange
  #define COLOR_RADIAL_SECONDS GColorYellow
  #define COLOR_HEX GColorVividViolet
  #define COLOR_MATRIX_BRIGHT GColorBrightGreen
  #define COLOR_MATRIX_MED GColorGreen
  #define COLOR_MATRIX_DIM GColorDarkGreen
  #define COLOR_WATER GColorVividCerulean
  #define COLOR_WATER_CONTAINER GColorWhite
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
  #define COLOR_RING_FULL GColorWhite
  #define COLOR_RING_EMPTY GColorBlack
  #define COLOR_SAND GColorWhite
  #define COLOR_HOURGLASS GColorWhite
  #define COLOR_BINARY_ON GColorWhite
  #define COLOR_BINARY_OFF GColorBlack
  #define COLOR_RADIAL_HOURS GColorWhite
  #define COLOR_RADIAL_MINUTES GColorWhite
  #define COLOR_RADIAL_SECONDS GColorWhite
  #define COLOR_HEX GColorWhite
  #define COLOR_MATRIX_BRIGHT GColorWhite
  #define COLOR_MATRIX_MED GColorWhite
  #define COLOR_MATRIX_DIM GColorWhite
  #define COLOR_WATER GColorWhite
  #define COLOR_WATER_CONTAINER GColorWhite
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
    case DISPLAY_MODE_RING: return "Ring";
    case DISPLAY_MODE_HOURGLASS: return "Hourglass";
    case DISPLAY_MODE_BINARY: return "Binary";
    case DISPLAY_MODE_RADIAL: return "Radial";
    case DISPLAY_MODE_HEX: return "Hex";
    case DISPLAY_MODE_MATRIX: return "Matrix";
    case DISPLAY_MODE_WATER_LEVEL: return "Water Level";
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
// Hourglass Sand Initialization
// =============================================================================

static void init_hourglass_sand(void) {
  s_num_sand_top = MAX_SAND_PARTICLES;
  s_num_sand_bottom = 0;
  
  // Initialize top chamber sand positions (stacked at bottom of top chamber)
  for (int i = 0; i < MAX_SAND_PARTICLES; i++) {
    s_sand_top[i] = i / 8;  // Stack in rows of 8
  }
}

static void update_hourglass_sand(void) {
  if (s_total_seconds <= 0) return;
  
  // Calculate how many particles should be in bottom based on elapsed time
  int elapsed = s_total_seconds - s_remaining_seconds;
  int target_bottom = (elapsed * MAX_SAND_PARTICLES) / s_total_seconds;
  
  // Move particles from top to bottom as needed
  if (target_bottom > s_num_sand_bottom && s_num_sand_top > 0) {
    s_num_sand_bottom = target_bottom;
    s_num_sand_top = MAX_SAND_PARTICLES - s_num_sand_bottom;
  }
}

// =============================================================================
// Matrix Rain Initialization
// =============================================================================

static void init_matrix_rain(void) {
  for (int col = 0; col < MATRIX_COLS; col++) {
    // Random starting positions (use remaining_seconds as seed variation)
    s_matrix_drops[col] = (col * 3 + s_remaining_seconds) % MATRIX_ROWS;
    s_matrix_speeds[col] = 1 + (col % 3);  // Speeds 1-3
    
    // Initialize with random characters (digits and symbols)
    for (int row = 0; row < MATRIX_ROWS; row++) {
      s_matrix_chars[col][row] = '0' + ((col + row * 7) % 10);
    }
  }
}

static void update_matrix_rain(void) {
  for (int col = 0; col < MATRIX_COLS; col++) {
    // Move drops down based on speed and time
    s_matrix_drops[col] = (s_matrix_drops[col] + s_matrix_speeds[col]) % (MATRIX_ROWS + 5);
    
    // Occasionally change characters for animation effect
    int change_row = (s_remaining_seconds + col) % MATRIX_ROWS;
    s_matrix_chars[col][change_row] = '0' + ((s_remaining_seconds + col) % 10);
  }
}

// =============================================================================
// Canvas Drawing - Blocks Mode
// =============================================================================

#define BLOCK_COLS 12
#define BLOCK_ROWS 8
#define BLOCK_PADDING 2

static void draw_blocks_mode(GContext *ctx, GRect bounds) {
  int available_width = bounds.size.w - 20;
  int available_height = bounds.size.h - 60;
  
  int block_width = (available_width - (BLOCK_COLS - 1) * BLOCK_PADDING) / BLOCK_COLS;
  int block_height = (available_height - (BLOCK_ROWS - 1) * BLOCK_PADDING) / BLOCK_ROWS;
  int block_size = (block_width < block_height) ? block_width : block_height;
  
  int grid_width = BLOCK_COLS * block_size + (BLOCK_COLS - 1) * BLOCK_PADDING;
  int grid_height = BLOCK_ROWS * block_size + (BLOCK_ROWS - 1) * BLOCK_PADDING;
  int start_x = (bounds.size.w - grid_width) / 2;
  int start_y = (bounds.size.h - grid_height) / 2 - 10;
  
  int total_blocks = BLOCK_COLS * BLOCK_ROWS;
  int filled_blocks = 0;
  if (s_total_seconds > 0) {
    filled_blocks = (s_remaining_seconds * total_blocks) / s_total_seconds;
  }
  
  for (int row = 0; row < BLOCK_ROWS; row++) {
    for (int col = 0; col < BLOCK_COLS; col++) {
      int block_index = (BLOCK_ROWS - 1 - row) * BLOCK_COLS + (BLOCK_COLS - 1 - col);
      int x = start_x + col * (block_size + BLOCK_PADDING);
      int y = start_y + row * (block_size + BLOCK_PADDING);
      GRect block_rect = GRect(x, y, block_size, block_size);
      
      if (block_index < filled_blocks) {
        graphics_context_set_fill_color(ctx, COLOR_BLOCKS_FULL);
        graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
      } else {
        graphics_context_set_stroke_color(ctx, COLOR_BLOCKS_EMPTY);
        graphics_draw_round_rect(ctx, block_rect, 2);
      }
    }
  }
  
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
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2 - 10;
  int radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 20;
  GPoint center = GPoint(center_x, center_y);
  
  graphics_context_set_stroke_color(ctx, COLOR_CLOCK_FACE);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, radius);
  
  for (int i = 0; i < 12; i++) {
    int angle = (i * 360 / 12) - 90;
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
  
  if (s_remaining_seconds > 0 && s_total_seconds > 0) {
    graphics_context_set_fill_color(ctx, COLOR_CLOCK_REMAINING);
    int segments = 60;
    int filled_segments = (s_remaining_seconds * segments) / s_total_seconds;
    
    for (int i = 0; i < filled_segments; i++) {
      int32_t seg_angle = -TRIG_MAX_ANGLE / 4 + (i * TRIG_MAX_ANGLE / segments);
      int32_t next_angle = -TRIG_MAX_ANGLE / 4 + ((i + 1) * TRIG_MAX_ANGLE / segments);
      int inner_r = radius / 3;
      int outer_r = radius - 12;
      
      GPoint p1 = GPoint(
        center_x + (cos_lookup(seg_angle) * inner_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(seg_angle) * inner_r / TRIG_MAX_RATIO));
      GPoint p2 = GPoint(
        center_x + (cos_lookup(seg_angle) * outer_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(seg_angle) * outer_r / TRIG_MAX_RATIO));
      GPoint p3 = GPoint(
        center_x + (cos_lookup(next_angle) * outer_r / TRIG_MAX_RATIO),
        center_y + (sin_lookup(next_angle) * outer_r / TRIG_MAX_RATIO));
      
      graphics_context_set_stroke_color(ctx, COLOR_CLOCK_REMAINING);
      graphics_context_set_stroke_width(ctx, 3);
      graphics_draw_line(ctx, p1, p2);
      graphics_draw_line(ctx, p2, p3);
      graphics_draw_line(ctx, p3, p1);
    }
  }
  
  graphics_context_set_fill_color(ctx, COLOR_CLOCK_FACE);
  graphics_fill_circle(ctx, center, 5);
  
  if (s_total_seconds > 0) {
    int32_t hand_angle = -TRIG_MAX_ANGLE / 4;
    if (s_remaining_seconds < s_total_seconds) {
      hand_angle = -TRIG_MAX_ANGLE / 4 + 
                   ((s_total_seconds - s_remaining_seconds) * TRIG_MAX_ANGLE / s_total_seconds);
    }
    int hand_length = radius - 15;
    GPoint hand_end = GPoint(
      center_x + (cos_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO),
      center_y + (sin_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO));
    graphics_context_set_stroke_color(ctx, COLOR_CLOCK_HAND);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, center, hand_end);
  }
  
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect text_rect = GRect(center_x - 40, center_y + radius + 5, 80, 24);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Progress Ring Mode
// =============================================================================

static void draw_ring_mode(GContext *ctx, GRect bounds) {
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2 - 5;
  int radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 15;
  GPoint center = GPoint(center_x, center_y);
  
  // Draw background ring (empty)
  graphics_context_set_stroke_color(ctx, COLOR_RING_EMPTY);
  graphics_context_set_stroke_width(ctx, 12);
  graphics_draw_circle(ctx, center, radius);
  
  // Draw progress arc
  if (s_remaining_seconds > 0 && s_total_seconds > 0) {
    int progress_degrees = (s_remaining_seconds * 360) / s_total_seconds;
    
    graphics_context_set_stroke_color(ctx, COLOR_RING_FULL);
    graphics_context_set_stroke_width(ctx, 10);
    
    // Draw arc as segments
    for (int deg = 0; deg < progress_degrees; deg += 3) {
      int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
      int x = center_x + (cos_lookup(angle) * radius / TRIG_MAX_RATIO);
      int y = center_y + (sin_lookup(angle) * radius / TRIG_MAX_RATIO);
      graphics_fill_circle(ctx, GPoint(x, y), 5);
    }
  }
  
  // Draw time in center
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  GRect text_rect = GRect(0, center_y - 20, bounds.size.w, 44);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Hourglass Mode
// =============================================================================

static void draw_hourglass_mode(GContext *ctx, GRect bounds) {
  update_hourglass_sand();
  
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2;
  int glass_width = 60;
  int glass_height = 100;
  int neck_width = 8;
  
  // Hourglass outline points
  int top = center_y - glass_height / 2;
  int bottom = center_y + glass_height / 2;
  int middle = center_y;
  
  // Draw hourglass outline
  graphics_context_set_stroke_color(ctx, COLOR_HOURGLASS);
  graphics_context_set_stroke_width(ctx, 2);
  
  // Top triangle
  graphics_draw_line(ctx, GPoint(center_x - glass_width/2, top), 
                         GPoint(center_x - neck_width/2, middle));
  graphics_draw_line(ctx, GPoint(center_x + glass_width/2, top), 
                         GPoint(center_x + neck_width/2, middle));
  graphics_draw_line(ctx, GPoint(center_x - glass_width/2, top), 
                         GPoint(center_x + glass_width/2, top));
  
  // Bottom triangle
  graphics_draw_line(ctx, GPoint(center_x - neck_width/2, middle), 
                         GPoint(center_x - glass_width/2, bottom));
  graphics_draw_line(ctx, GPoint(center_x + neck_width/2, middle), 
                         GPoint(center_x + glass_width/2, bottom));
  graphics_draw_line(ctx, GPoint(center_x - glass_width/2, bottom), 
                         GPoint(center_x + glass_width/2, bottom));
  
  // Draw sand in top chamber
  graphics_context_set_fill_color(ctx, COLOR_SAND);
  int top_chamber_bottom = middle - 5;
  int sand_rows_top = (s_num_sand_top + 7) / 8;  // Rows needed for remaining sand
  
  for (int row = 0; row < sand_rows_top && row < 6; row++) {
    int y = top_chamber_bottom - (row + 1) * 7;
    // Width narrows toward neck
    int row_from_neck = row;
    int row_width = neck_width + row_from_neck * 8;
    if (row_width > glass_width - 10) row_width = glass_width - 10;
    
    int particles_in_row = (row < sand_rows_top - 1) ? 8 : (s_num_sand_top % 8);
    if (particles_in_row == 0 && row < sand_rows_top) particles_in_row = 8;
    
    for (int p = 0; p < particles_in_row; p++) {
      int x = center_x - row_width/2 + (row_width * p) / 7;
      graphics_fill_circle(ctx, GPoint(x, y), 3);
    }
  }
  
  // Draw sand in bottom chamber
  int bottom_chamber_top = middle + 5;
  int sand_rows_bottom = (s_num_sand_bottom + 7) / 8;
  
  for (int row = 0; row < sand_rows_bottom && row < 6; row++) {
    int y = bottom - 8 - row * 7;
    // Width narrows toward neck (from bottom)
    int row_from_bottom = row;
    int row_width = glass_width - 10 - row_from_bottom * 8;
    if (row_width < neck_width) row_width = neck_width;
    
    int particles_in_row = (row < sand_rows_bottom - 1) ? 8 : (s_num_sand_bottom % 8);
    if (particles_in_row == 0 && row < sand_rows_bottom) particles_in_row = 8;
    
    for (int p = 0; p < particles_in_row; p++) {
      int x = center_x - row_width/2 + (row_width * p) / 7;
      graphics_fill_circle(ctx, GPoint(x, y), 3);
    }
  }
  
  // Draw falling sand particle if timer is running
  if (s_state == STATE_RUNNING && s_num_sand_top > 0) {
    int fall_y = middle + ((s_remaining_seconds % 2) * 5);
    graphics_fill_circle(ctx, GPoint(center_x, fall_y), 2);
  }
  
  // Draw time below
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect text_rect = GRect(0, bottom + 5, bounds.size.w, 30);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Binary Mode
// =============================================================================

static void draw_binary_mode(GContext *ctx, GRect bounds) {
  int hours = s_remaining_seconds / 3600;
  int minutes = (s_remaining_seconds % 3600) / 60;
  int seconds = s_remaining_seconds % 60;
  
  int center_x = bounds.size.w / 2;
  int start_y = 25;
  int dot_radius = 8;
  int dot_spacing = 22;
  int row_spacing = 30;
  
  // Labels
  GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  graphics_context_set_text_color(ctx, COLOR_HINT);
  
  // Draw binary representation for hours (6 bits: 0-23)
  graphics_draw_text(ctx, "H", label_font, GRect(5, start_y, 20, 20),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  for (int bit = 5; bit >= 0; bit--) {
    int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
    bool is_set = (hours >> bit) & 1;
    
    if (is_set) {
      graphics_context_set_fill_color(ctx, COLOR_BINARY_ON);
      graphics_fill_circle(ctx, GPoint(x, start_y + 10), dot_radius);
    } else {
      graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_circle(ctx, GPoint(x, start_y + 10), dot_radius);
    }
  }
  
  // Draw binary representation for minutes (6 bits: 0-59)
  int min_y = start_y + row_spacing;
  graphics_draw_text(ctx, "M", label_font, GRect(5, min_y, 20, 20),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  for (int bit = 5; bit >= 0; bit--) {
    int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
    bool is_set = (minutes >> bit) & 1;
    
    if (is_set) {
      graphics_context_set_fill_color(ctx, COLOR_BINARY_ON);
      graphics_fill_circle(ctx, GPoint(x, min_y + 10), dot_radius);
    } else {
      graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_circle(ctx, GPoint(x, min_y + 10), dot_radius);
    }
  }
  
  // Draw binary representation for seconds (6 bits: 0-59)
  int sec_y = start_y + row_spacing * 2;
  graphics_draw_text(ctx, "S", label_font, GRect(5, sec_y, 20, 20),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  for (int bit = 5; bit >= 0; bit--) {
    int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
    bool is_set = (seconds >> bit) & 1;
    
    if (is_set) {
      graphics_context_set_fill_color(ctx, COLOR_BINARY_ON);
      graphics_fill_circle(ctx, GPoint(x, sec_y + 10), dot_radius);
    } else {
      graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_circle(ctx, GPoint(x, sec_y + 10), dot_radius);
    }
  }
  
  // Bit labels at bottom
  graphics_context_set_text_color(ctx, COLOR_HINT);
  GFont tiny_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  for (int bit = 5; bit >= 0; bit--) {
    int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2 - 8;
    static char bit_label[4];
    snprintf(bit_label, sizeof(bit_label), "%d", 1 << bit);
    graphics_draw_text(ctx, bit_label, tiny_font, GRect(x, sec_y + 25, 20, 16),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
  
  // Draw decimal time below
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect text_rect = GRect(0, bounds.size.h - 40, bounds.size.w, 30);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Radial Bars Mode
// =============================================================================

static void draw_radial_mode(GContext *ctx, GRect bounds) {
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2 - 10;
  GPoint center = GPoint(center_x, center_y);
  
  int hours = s_remaining_seconds / 3600;
  int minutes = (s_remaining_seconds % 3600) / 60;
  int seconds = s_remaining_seconds % 60;
  
  // Three concentric rings: outer=hours, middle=minutes, inner=seconds
  int ring_width = 8;
  int ring_gap = 4;
  int outer_radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 20;
  
  // Seconds ring (innermost)
  int sec_radius = outer_radius - 2 * (ring_width + ring_gap);
  graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
  graphics_context_set_stroke_width(ctx, ring_width);
  graphics_draw_circle(ctx, center, sec_radius);
  
  if (seconds > 0) {
    int sec_degrees = (seconds * 360) / 60;
    graphics_context_set_stroke_color(ctx, COLOR_RADIAL_SECONDS);
    for (int deg = 0; deg < sec_degrees; deg += 4) {
      int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
      int x = center_x + (cos_lookup(angle) * sec_radius / TRIG_MAX_RATIO);
      int y = center_y + (sin_lookup(angle) * sec_radius / TRIG_MAX_RATIO);
      graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
    }
  }
  
  // Minutes ring (middle)
  int min_radius = outer_radius - (ring_width + ring_gap);
  graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
  graphics_context_set_stroke_width(ctx, ring_width);
  graphics_draw_circle(ctx, center, min_radius);
  
  if (minutes > 0) {
    int min_degrees = (minutes * 360) / 60;
    graphics_context_set_stroke_color(ctx, COLOR_RADIAL_MINUTES);
    for (int deg = 0; deg < min_degrees; deg += 4) {
      int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
      int x = center_x + (cos_lookup(angle) * min_radius / TRIG_MAX_RATIO);
      int y = center_y + (sin_lookup(angle) * min_radius / TRIG_MAX_RATIO);
      graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
    }
  }
  
  // Hours ring (outermost)
  graphics_context_set_stroke_color(ctx, COLOR_BINARY_OFF);
  graphics_context_set_stroke_width(ctx, ring_width);
  graphics_draw_circle(ctx, center, outer_radius);
  
  if (hours > 0) {
    int hour_degrees = (hours * 360) / 24;  // 24-hour scale
    graphics_context_set_stroke_color(ctx, COLOR_RADIAL_HOURS);
    for (int deg = 0; deg < hour_degrees; deg += 4) {
      int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
      int x = center_x + (cos_lookup(angle) * outer_radius / TRIG_MAX_RATIO);
      int y = center_y + (sin_lookup(angle) * outer_radius / TRIG_MAX_RATIO);
      graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
    }
  }
  
  // Draw time in center
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect text_rect = GRect(0, center_y - 14, bounds.size.w, 30);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  // Legend at bottom
  GFont tiny = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  graphics_context_set_text_color(ctx, COLOR_RADIAL_HOURS);
  graphics_draw_text(ctx, "H", tiny, GRect(center_x - 45, bounds.size.h - 25, 20, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, COLOR_RADIAL_MINUTES);
  graphics_draw_text(ctx, "M", tiny, GRect(center_x - 10, bounds.size.h - 25, 20, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, COLOR_RADIAL_SECONDS);
  graphics_draw_text(ctx, "S", tiny, GRect(center_x + 25, bounds.size.h - 25, 20, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Drawing - Hexadecimal Mode
// =============================================================================

static void draw_hex_mode(GContext *ctx, GRect bounds) {
  int hours = s_remaining_seconds / 3600;
  int minutes = (s_remaining_seconds % 3600) / 60;
  int seconds = s_remaining_seconds % 60;
  
  int center_y = bounds.size.h / 2;
  
  // Format as hex: HH:MM:SS in hexadecimal
  static char hex_buf[16];
  if (hours > 0) {
    snprintf(hex_buf, sizeof(hex_buf), "%X:%02X:%02X", hours, minutes, seconds);
  } else {
    snprintf(hex_buf, sizeof(hex_buf), "%X:%02X", minutes, seconds);
  }
  
  // Draw hex time large
  GFont hex_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  GRect hex_rect = GRect(0, center_y - 30, bounds.size.w, 50);
  graphics_context_set_text_color(ctx, COLOR_HEX);
  graphics_draw_text(ctx, hex_buf, hex_font, hex_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  // Draw "0x" prefix
  GFont prefix_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect prefix_rect = GRect(10, center_y - 50, 30, 24);
  graphics_context_set_text_color(ctx, COLOR_HINT);
  graphics_draw_text(ctx, "0x", prefix_font, prefix_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  
  // Draw decimal equivalent below
  static char dec_buf[20];
  snprintf(dec_buf, sizeof(dec_buf), "= %d sec", s_remaining_seconds);
  GFont dec_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GRect dec_rect = GRect(0, center_y + 25, bounds.size.w, 24);
  graphics_context_set_text_color(ctx, COLOR_HINT);
  graphics_draw_text(ctx, dec_buf, dec_font, dec_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  // Draw progress bar at bottom
  int bar_y = bounds.size.h - 30;
  int bar_height = 10;
  int bar_margin = 20;
  int bar_width = bounds.size.w - bar_margin * 2;
  
  // Background bar
  graphics_context_set_fill_color(ctx, COLOR_BINARY_OFF);
  graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 3, GCornersAll);
  
  // Progress bar
  if (s_total_seconds > 0) {
    int progress_width = (s_remaining_seconds * bar_width) / s_total_seconds;
    graphics_context_set_fill_color(ctx, COLOR_HEX);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 3, GCornersAll);
  }
}

// =============================================================================
// Canvas Drawing - Matrix Rain Mode
// =============================================================================

static void draw_matrix_mode(GContext *ctx, GRect bounds) {
  update_matrix_rain();
  
  int col_width = bounds.size.w / MATRIX_COLS;
  int row_height = 14;
  int start_y = 10;
  
  GFont char_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
  // Draw falling characters
  for (int col = 0; col < MATRIX_COLS; col++) {
    int drop_head = s_matrix_drops[col];
    int x = col * col_width + col_width / 2 - 4;
    
    for (int row = 0; row < MATRIX_ROWS; row++) {
      int y = start_y + row * row_height;
      
      // Calculate distance from drop head for brightness
      int dist = drop_head - row;
      if (dist < 0) dist += MATRIX_ROWS + 5;
      
      // Only draw if within trail length
      if (dist <= 6) {
        static char char_buf[2];
        char_buf[0] = s_matrix_chars[col][row];
        char_buf[1] = '\0';
        
        // Color based on distance from head
        if (dist == 0) {
          graphics_context_set_text_color(ctx, COLOR_MATRIX_BRIGHT);
        } else if (dist <= 2) {
          graphics_context_set_text_color(ctx, COLOR_MATRIX_MED);
        } else {
          graphics_context_set_text_color(ctx, COLOR_MATRIX_DIM);
        }
        
        graphics_draw_text(ctx, char_buf, char_font, 
                          GRect(x, y, 12, 16),
                          GTextOverflowModeTrailingEllipsis, 
                          GTextAlignmentCenter, NULL);
      }
    }
  }
  
  // Draw time prominently in center with black background for readability
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  
  int center_y = bounds.size.h / 2;
  GFont time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  GRect time_rect = GRect(10, center_y - 22, bounds.size.w - 20, 44);
  
  // Draw dark background behind time
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(15, center_y - 20, bounds.size.w - 30, 40), 4, GCornersAll);
  
  // Draw time in bright green
  graphics_context_set_text_color(ctx, COLOR_MATRIX_BRIGHT);
  graphics_draw_text(ctx, time_buf, time_font, time_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  // Draw subtle progress indicator at bottom
  int bar_y = bounds.size.h - 8;
  int bar_height = 3;
  int bar_margin = 20;
  int bar_width = bounds.size.w - bar_margin * 2;
  
  if (s_total_seconds > 0) {
    int progress_width = (s_remaining_seconds * bar_width) / s_total_seconds;
    graphics_context_set_fill_color(ctx, COLOR_MATRIX_DIM);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 1, GCornersAll);
    graphics_context_set_fill_color(ctx, COLOR_MATRIX_BRIGHT);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 1, GCornersAll);
  }
}

// =============================================================================
// Canvas Drawing - Water Level Mode
// =============================================================================

static void draw_water_level_mode(GContext *ctx, GRect bounds) {
  int center_x = bounds.size.w / 2;
  int center_y = bounds.size.h / 2 - 10;
  
  // Container dimensions
  int container_width = 50;
  int container_height = 100;
  int container_top = center_y - container_height / 2;
  int container_bottom = container_top + container_height;
  int container_left = center_x - container_width / 2;
  int container_right = center_x + container_width / 2;
  
  // Draw container outline (beaker/glass shape)
  graphics_context_set_stroke_color(ctx, COLOR_WATER_CONTAINER);
  graphics_context_set_stroke_width(ctx, 2);
  
  // Left side
  graphics_draw_line(ctx, GPoint(container_left, container_top + 10), 
                         GPoint(container_left, container_bottom));
  // Right side
  graphics_draw_line(ctx, GPoint(container_right, container_top + 10), 
                         GPoint(container_right, container_bottom));
  // Bottom
  graphics_draw_line(ctx, GPoint(container_left, container_bottom), 
                         GPoint(container_right, container_bottom));
  // Top rim (wider)
  int rim_width = container_width + 8;
  graphics_draw_line(ctx, GPoint(center_x - rim_width/2, container_top + 10), 
                         GPoint(center_x + rim_width/2, container_top + 10));
  // Rim sides
  graphics_draw_line(ctx, GPoint(center_x - rim_width/2, container_top + 10), 
                         GPoint(container_left, container_top + 10));
  graphics_draw_line(ctx, GPoint(center_x + rim_width/2, container_top + 10), 
                         GPoint(container_right, container_top + 10));
  
  // Calculate water level based on remaining time
  int water_height = 0;
  if (s_total_seconds > 0) {
    water_height = (s_remaining_seconds * (container_height - 20)) / s_total_seconds;
  }
  
  // Draw water (filled area)
  if (water_height > 0) {
    int water_top = container_bottom - water_height;
    int water_bottom = container_bottom;
    
    // Fill water area
    graphics_context_set_fill_color(ctx, COLOR_WATER);
    graphics_fill_rect(ctx, GRect(container_left + 1, water_top, 
                                  container_width - 2, water_height), 0, GCornerNone);
    
    // Draw water surface with slight wave effect
    graphics_context_set_stroke_color(ctx, COLOR_WATER);
    graphics_context_set_stroke_width(ctx, 2);
    
    // Animated wave effect based on remaining seconds
    int wave_offset = (s_remaining_seconds % 4) - 2;
    for (int x = container_left + 2; x < container_right - 2; x += 3) {
      int y = water_top + (wave_offset * (x % 3 - 1)) / 2;
      if (y >= water_top - 1 && y <= water_top + 1) {
        graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 2, y));
      }
    }
  }
  
  // Draw measurement marks on container
  graphics_context_set_stroke_color(ctx, COLOR_HINT);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 1; i <= 4; i++) {
    int mark_y = container_top + 10 + (i * (container_height - 20) / 5);
    graphics_draw_line(ctx, GPoint(container_left - 5, mark_y), 
                           GPoint(container_left, mark_y));
  }
  
  // Draw time below container
  static char time_buf[16];
  format_time_adaptive(s_remaining_seconds, time_buf, sizeof(time_buf));
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GRect text_rect = GRect(0, container_bottom + 10, bounds.size.w, 30);
  graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
  graphics_draw_text(ctx, time_buf, font, text_rect,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Canvas Update Procedure
// =============================================================================

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  if (s_state != STATE_RUNNING && s_state != STATE_PAUSED) {
    return;
  }
  
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  switch (s_display_mode) {
    case DISPLAY_MODE_BLOCKS:
      draw_blocks_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_CLOCK:
      draw_clock_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_RING:
      draw_ring_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_HOURGLASS:
      draw_hourglass_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_BINARY:
      draw_binary_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_RADIAL:
      draw_radial_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_HEX:
      draw_hex_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_MATRIX:
      draw_matrix_mode(ctx, bounds);
      break;
    case DISPLAY_MODE_WATER_LEVEL:
      draw_water_level_mode(ctx, bounds);
      break;
    default:
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
  
  window_set_background_color(s_main_window, COLOR_BACKGROUND);
  
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
  init_hourglass_sand();  // Reset hourglass for new timer
  init_matrix_rain();     // Reset matrix rain for new timer
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
  init_hourglass_sand();  // Reset hourglass on restart
  init_matrix_rain();     // Reset matrix rain on restart
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
  vibes_short_pulse();
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
      break;
  }
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
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
      if (s_state == STATE_RUNNING) {
        pause_timer();
      }
      s_state = STATE_CONFIRM_EXIT;
      update_display();
      break;
      
    case STATE_SET_CUSTOM_HOURS:
    case STATE_SET_CUSTOM_MINUTES:
      s_state = STATE_SELECT_PRESET;
      update_display();
      break;
      
    case STATE_CONFIRM_EXIT:
      s_state = STATE_PAUSED;
      update_display();
      break;
      
    case STATE_COMPLETED:
      dismiss_completion();
      break;
      
    default:
      window_stack_pop(true);
      break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
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
  
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_set_hidden(s_canvas_layer, true);
  layer_add_child(window_layer, s_canvas_layer);
  
  s_title_layer = text_layer_create(GRect(inset, title_y, bounds.size.w - (inset * 2), 30));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, COLOR_HINT);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));
  
  s_time_layer = text_layer_create(GRect(inset, time_y, bounds.size.w - (inset * 2), 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, COLOR_TEXT_NORMAL);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
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
