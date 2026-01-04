#include "display_common.h"

// =============================================================================
// Display Context Creation
// =============================================================================

DisplayContext display_context_from_timer(const TimerContext *timer, const VisualizationColors *colors) {
    DisplayContext dctx = {
        .remaining_seconds = timer->remaining_seconds,
        .total_seconds = timer->total_seconds,
        .state = timer->state,
        .display_mode = timer->display_mode,
        .hide_time_text = timer->hide_time_text,
        .colors = colors
    };
    return dctx;
}

// =============================================================================
// Animation Initialization
// =============================================================================

void animation_init_hourglass(HourglassState *state) {
    state->num_sand_top = MAX_SAND_PARTICLES;
    state->num_sand_bottom = 0;
    
    for (int i = 0; i < MAX_SAND_PARTICLES; i++) {
        state->sand_top[i] = i / 8;
    }
}

void animation_init_matrix(MatrixState *state, int seed) {
    for (int col = 0; col < MATRIX_COLS; col++) {
        state->drops[col] = (col * 3 + seed) % MATRIX_ROWS;
        state->speeds[col] = 1 + (col % 3);
        
        for (int row = 0; row < MATRIX_ROWS; row++) {
            state->chars[col][row] = '0' + ((col + row * 7) % 10);
        }
    }
}

// =============================================================================
// Animation Updates
// =============================================================================

void animation_update_hourglass(HourglassState *state, int remaining_seconds, int total_seconds) {
    if (total_seconds <= 0) return;
    
    int elapsed = total_seconds - remaining_seconds;
    int target_bottom = (elapsed * MAX_SAND_PARTICLES) / total_seconds;
    
    if (target_bottom > state->num_sand_bottom && state->num_sand_top > 0) {
        state->num_sand_bottom = target_bottom;
        state->num_sand_top = MAX_SAND_PARTICLES - state->num_sand_bottom;
    }
}

void animation_update_matrix(MatrixState *state, int remaining_seconds) {
    for (int col = 0; col < MATRIX_COLS; col++) {
        state->drops[col] = (state->drops[col] + state->speeds[col]) % (MATRIX_ROWS + 5);
        
        int change_row = (remaining_seconds + col) % MATRIX_ROWS;
        state->chars[col][change_row] = '0' + ((remaining_seconds + col) % 10);
    }
}

// =============================================================================
// Helper: Draw Time Text at Position
// =============================================================================

static void draw_time_text(GContext *ctx, int remaining_seconds, GRect text_rect, GFont font) {
    static char time_buf[16];
    time_format_adaptive(remaining_seconds, time_buf, sizeof(time_buf));
    graphics_context_set_text_color(ctx, COLOR_TEXT_NORMAL);
    graphics_draw_text(ctx, time_buf, font, text_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Blocks Mode
// =============================================================================

#define BLOCK_COLS 12
#define BLOCK_ROWS 8
#define BLOCK_PADDING 2

void display_draw_blocks(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
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
    int filled_blocks = progress_calculate_blocks(dctx->remaining_seconds, dctx->total_seconds, total_blocks);
    
    for (int row = 0; row < BLOCK_ROWS; row++) {
        for (int col = 0; col < BLOCK_COLS; col++) {
            int block_index = (BLOCK_ROWS - 1 - row) * BLOCK_COLS + (BLOCK_COLS - 1 - col);
            int x = start_x + col * (block_size + BLOCK_PADDING);
            int y = start_y + row * (block_size + BLOCK_PADDING);
            GRect block_rect = GRect(x, y, block_size, block_size);
            
            if (block_index < filled_blocks) {
                graphics_context_set_fill_color(ctx, c->primary);
                graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
            } else {
                graphics_context_set_stroke_color(ctx, c->secondary);
                graphics_draw_round_rect(ctx, block_rect, 2);
            }
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, start_y + grid_height + 5, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Vertical Blocks Mode
// =============================================================================

#define VERTICAL_BLOCK_COLS 8
#define VERTICAL_BLOCK_ROWS 12
#define VERTICAL_BLOCK_PADDING 2

void display_draw_vertical_blocks(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int available_width = bounds.size.w - 20;
    int available_height = bounds.size.h - 60;
    
    int block_width = (available_width - (VERTICAL_BLOCK_COLS - 1) * VERTICAL_BLOCK_PADDING) / VERTICAL_BLOCK_COLS;
    int block_height = (available_height - (VERTICAL_BLOCK_ROWS - 1) * VERTICAL_BLOCK_PADDING) / VERTICAL_BLOCK_ROWS;
    int block_size = (block_width < block_height) ? block_width : block_height;
    
    int grid_width = VERTICAL_BLOCK_COLS * block_size + (VERTICAL_BLOCK_COLS - 1) * VERTICAL_BLOCK_PADDING;
    int grid_height = VERTICAL_BLOCK_ROWS * block_size + (VERTICAL_BLOCK_ROWS - 1) * VERTICAL_BLOCK_PADDING;
    int start_x = (bounds.size.w - grid_width) / 2;
    int start_y = (bounds.size.h - grid_height) / 2 - 10;
    
    int total_blocks = VERTICAL_BLOCK_COLS * VERTICAL_BLOCK_ROWS;
    int filled_blocks = progress_calculate_blocks(dctx->remaining_seconds, dctx->total_seconds, total_blocks);
    
    for (int col = 0; col < VERTICAL_BLOCK_COLS; col++) {
        for (int row = VERTICAL_BLOCK_ROWS - 1; row >= 0; row--) {
            int reversed_col = VERTICAL_BLOCK_COLS - 1 - col;
            int block_index = reversed_col * VERTICAL_BLOCK_ROWS + (VERTICAL_BLOCK_ROWS - 1 - row);
            int x = start_x + col * (block_size + VERTICAL_BLOCK_PADDING);
            int y = start_y + row * (block_size + VERTICAL_BLOCK_PADDING);
            GRect block_rect = GRect(x, y, block_size, block_size);
            
            if (block_index < filled_blocks) {
                graphics_context_set_fill_color(ctx, c->primary);
                graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
            } else {
                graphics_context_set_stroke_color(ctx, c->secondary);
                graphics_draw_round_rect(ctx, block_rect, 2);
            }
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, start_y + grid_height + 5, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Clock Mode
// =============================================================================

void display_draw_clock(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_x = bounds.size.w / 2;
    int center_y = bounds.size.h / 2 - 10;
    int radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 20;
    GPoint center = GPoint(center_x, center_y);
    
    // Clock face
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, center, radius);
    
    // Hour markers
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
    
    // Progress arc
    if (dctx->remaining_seconds > 0 && dctx->total_seconds > 0) {
        graphics_context_set_fill_color(ctx, c->primary);
        int segments = 60;
        int filled_segments = (dctx->remaining_seconds * segments) / dctx->total_seconds;
        
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
            
            graphics_context_set_stroke_color(ctx, c->primary);
            graphics_context_set_stroke_width(ctx, 3);
            graphics_draw_line(ctx, p1, p2);
            graphics_draw_line(ctx, p2, p3);
            graphics_draw_line(ctx, p3, p1);
        }
    }
    
    // Center dot
    graphics_context_set_fill_color(ctx, c->secondary);
    graphics_fill_circle(ctx, center, 5);
    
    // Clock hand
    if (dctx->total_seconds > 0) {
        int32_t hand_angle = -TRIG_MAX_ANGLE / 4;
        if (dctx->remaining_seconds < dctx->total_seconds) {
            hand_angle = -TRIG_MAX_ANGLE / 4 + 
                         ((dctx->total_seconds - dctx->remaining_seconds) * TRIG_MAX_ANGLE / dctx->total_seconds);
        }
        int hand_length = radius - 15;
        GPoint hand_end = GPoint(
            center_x + (cos_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO),
            center_y + (sin_lookup(hand_angle) * hand_length / TRIG_MAX_RATIO));
        graphics_context_set_stroke_color(ctx, c->accent);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, center, hand_end);
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        GRect text_rect = GRect(center_x - 40, center_y + radius + 5, 80, 24);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Ring Mode
// =============================================================================

void display_draw_ring(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_x = bounds.size.w / 2;
    int center_y = bounds.size.h / 2 - 5;
    int radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 15;
    GPoint center = GPoint(center_x, center_y);
    
    // Background ring
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, 12);
    graphics_draw_circle(ctx, center, radius);
    
    // Progress arc
    if (dctx->remaining_seconds > 0 && dctx->total_seconds > 0) {
        int progress_degrees = progress_calculate_degrees(dctx->remaining_seconds, dctx->total_seconds);
        
        graphics_context_set_stroke_color(ctx, c->primary);
        graphics_context_set_stroke_width(ctx, 10);
        
        for (int deg = 0; deg < progress_degrees; deg += 3) {
            int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
            int x = center_x + (cos_lookup(angle) * radius / TRIG_MAX_RATIO);
            int y = center_y + (sin_lookup(angle) * radius / TRIG_MAX_RATIO);
            graphics_fill_circle(ctx, GPoint(x, y), 5);
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
        GRect text_rect = GRect(0, center_y - 20, bounds.size.w, 44);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Hourglass Mode
// =============================================================================

void display_draw_hourglass(GContext *ctx, GRect bounds, const DisplayContext *dctx, HourglassState *anim) {
    const VisualizationColors *c = dctx->colors;
    animation_update_hourglass(anim, dctx->remaining_seconds, dctx->total_seconds);
    
    int center_x = bounds.size.w / 2;
    int center_y = bounds.size.h / 2;
    int glass_width = 60;
    int glass_height = 100;
    int neck_width = 8;
    
    int top = center_y - glass_height / 2;
    int bottom = center_y + glass_height / 2;
    int middle = center_y;
    
    // Hourglass outline
    graphics_context_set_stroke_color(ctx, c->secondary);
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
    
    // Sand in top chamber
    graphics_context_set_fill_color(ctx, c->primary);
    int top_chamber_bottom = middle - 5;
    int sand_rows_top = (anim->num_sand_top + 7) / 8;
    
    for (int row = 0; row < sand_rows_top && row < 6; row++) {
        int y = top_chamber_bottom - (row + 1) * 7;
        int row_from_neck = row;
        int row_width = neck_width + row_from_neck * 8;
        if (row_width > glass_width - 10) row_width = glass_width - 10;
        
        int particles_in_row = (row < sand_rows_top - 1) ? 8 : (anim->num_sand_top % 8);
        if (particles_in_row == 0 && row < sand_rows_top) particles_in_row = 8;
        
        for (int p = 0; p < particles_in_row; p++) {
            int x = center_x - row_width/2 + (row_width * p) / 7;
            graphics_fill_circle(ctx, GPoint(x, y), 3);
        }
    }
    
    // Sand in bottom chamber
    int sand_rows_bottom = (anim->num_sand_bottom + 7) / 8;
    
    for (int row = 0; row < sand_rows_bottom && row < 6; row++) {
        int y = bottom - 8 - row * 7;
        int row_from_bottom = row;
        int row_width = glass_width - 10 - row_from_bottom * 8;
        if (row_width < neck_width) row_width = neck_width;
        
        int particles_in_row = (row < sand_rows_bottom - 1) ? 8 : (anim->num_sand_bottom % 8);
        if (particles_in_row == 0 && row < sand_rows_bottom) particles_in_row = 8;
        
        for (int p = 0; p < particles_in_row; p++) {
            int x = center_x - row_width/2 + (row_width * p) / 7;
            graphics_fill_circle(ctx, GPoint(x, y), 3);
        }
    }
    
    // Falling sand particle
    if (dctx->state == STATE_RUNNING && anim->num_sand_top > 0) {
        int fall_y = middle + ((dctx->remaining_seconds % 2) * 5);
        graphics_context_set_fill_color(ctx, c->primary);
        graphics_fill_circle(ctx, GPoint(center_x, fall_y), 2);
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, bottom + 5, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Binary Mode
// =============================================================================

void display_draw_binary(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    TimeComponents t = time_decompose(dctx->remaining_seconds);
    
    int center_x = bounds.size.w / 2;
    int start_y = 25;
    int dot_radius = 8;
    int dot_spacing = 22;
    int row_spacing = 30;
    
    GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    graphics_context_set_text_color(ctx, COLOR_HINT);
    
    // Hours row
    graphics_draw_text(ctx, "H", label_font, GRect(5, start_y, 20, 20),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    for (int bit = 5; bit >= 0; bit--) {
        int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
        bool is_set = (t.hours >> bit) & 1;
        
        if (is_set) {
            graphics_context_set_fill_color(ctx, c->primary);
            graphics_fill_circle(ctx, GPoint(x, start_y + 10), dot_radius);
        } else {
            graphics_context_set_stroke_color(ctx, c->secondary);
            graphics_context_set_stroke_width(ctx, 2);
            graphics_draw_circle(ctx, GPoint(x, start_y + 10), dot_radius);
        }
    }
    
    // Minutes row
    int min_y = start_y + row_spacing;
    graphics_draw_text(ctx, "M", label_font, GRect(5, min_y, 20, 20),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    for (int bit = 5; bit >= 0; bit--) {
        int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
        bool is_set = (t.minutes >> bit) & 1;
        
        if (is_set) {
            graphics_context_set_fill_color(ctx, c->primary);
            graphics_fill_circle(ctx, GPoint(x, min_y + 10), dot_radius);
        } else {
            graphics_context_set_stroke_color(ctx, c->secondary);
            graphics_context_set_stroke_width(ctx, 2);
            graphics_draw_circle(ctx, GPoint(x, min_y + 10), dot_radius);
        }
    }
    
    // Seconds row
    int sec_y = start_y + row_spacing * 2;
    graphics_draw_text(ctx, "S", label_font, GRect(5, sec_y, 20, 20),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    for (int bit = 5; bit >= 0; bit--) {
        int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2;
        bool is_set = (t.seconds >> bit) & 1;
        
        if (is_set) {
            graphics_context_set_fill_color(ctx, c->primary);
            graphics_fill_circle(ctx, GPoint(x, sec_y + 10), dot_radius);
        } else {
            graphics_context_set_stroke_color(ctx, c->secondary);
            graphics_context_set_stroke_width(ctx, 2);
            graphics_draw_circle(ctx, GPoint(x, sec_y + 10), dot_radius);
        }
    }
    
    // Bit labels
    graphics_context_set_text_color(ctx, COLOR_HINT);
    GFont tiny_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    for (int bit = 5; bit >= 0; bit--) {
        int x = center_x - (3 * dot_spacing) + (5 - bit) * dot_spacing + dot_spacing/2 - 8;
        static char bit_label[4];
        snprintf(bit_label, sizeof(bit_label), "%d", 1 << bit);
        graphics_draw_text(ctx, bit_label, tiny_font, GRect(x, sec_y + 25, 20, 16),
                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, bounds.size.h - 40, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Radial Mode
// =============================================================================

void display_draw_radial(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_x = bounds.size.w / 2;
    int center_y = bounds.size.h / 2 - 10;
    GPoint center = GPoint(center_x, center_y);
    
    TimeComponents t = time_decompose(dctx->remaining_seconds);
    
    int ring_width = 8;
    int ring_gap = 4;
    int outer_radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2 - 20;
    
    // Seconds ring (innermost)
    int sec_radius = outer_radius - 2 * (ring_width + ring_gap);
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, ring_width);
    graphics_draw_circle(ctx, center, sec_radius);
    
    if (t.seconds > 0) {
        int sec_degrees = (t.seconds * 360) / 60;
        graphics_context_set_stroke_color(ctx, c->accent);
        for (int deg = 0; deg < sec_degrees; deg += 4) {
            int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
            int x = center_x + (cos_lookup(angle) * sec_radius / TRIG_MAX_RATIO);
            int y = center_y + (sin_lookup(angle) * sec_radius / TRIG_MAX_RATIO);
            graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
        }
    }
    
    // Minutes ring (middle)
    int min_radius = outer_radius - (ring_width + ring_gap);
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, ring_width);
    graphics_draw_circle(ctx, center, min_radius);
    
    if (t.minutes > 0) {
        int min_degrees = (t.minutes * 360) / 60;
        graphics_context_set_stroke_color(ctx, c->secondary);
        for (int deg = 0; deg < min_degrees; deg += 4) {
            int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
            int x = center_x + (cos_lookup(angle) * min_radius / TRIG_MAX_RATIO);
            int y = center_y + (sin_lookup(angle) * min_radius / TRIG_MAX_RATIO);
            graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
        }
    }
    
    // Hours ring (outermost)
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, ring_width);
    graphics_draw_circle(ctx, center, outer_radius);
    
    if (t.hours > 0) {
        int hour_degrees = (t.hours * 360) / 24;
        graphics_context_set_stroke_color(ctx, c->primary);
        for (int deg = 0; deg < hour_degrees; deg += 4) {
            int32_t angle = (-90 + deg) * TRIG_MAX_ANGLE / 360;
            int x = center_x + (cos_lookup(angle) * outer_radius / TRIG_MAX_RATIO);
            int y = center_y + (sin_lookup(angle) * outer_radius / TRIG_MAX_RATIO);
            graphics_fill_circle(ctx, GPoint(x, y), ring_width / 2 - 1);
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, center_y - 14, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
    
    // Legend
    GFont tiny = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, "H", tiny, GRect(center_x - 45, bounds.size.h - 25, 20, 16),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, c->secondary);
    graphics_draw_text(ctx, "M", tiny, GRect(center_x - 10, bounds.size.h - 25, 20, 16),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, c->accent);
    graphics_draw_text(ctx, "S", tiny, GRect(center_x + 25, bounds.size.h - 25, 20, 16),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// =============================================================================
// Hex Mode
// =============================================================================

void display_draw_hex(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_y = bounds.size.h / 2;
    
    static char hex_buf[16];
    time_format_hex(dctx->remaining_seconds, hex_buf, sizeof(hex_buf));
    
    // Hex time
    GFont hex_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    GRect hex_rect = GRect(0, center_y - 30, bounds.size.w, 50);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, hex_buf, hex_font, hex_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // 0x prefix
    GFont prefix_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    GRect prefix_rect = GRect(10, center_y - 50, 30, 24);
    graphics_context_set_text_color(ctx, c->secondary);
    graphics_draw_text(ctx, "0x", prefix_font, prefix_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    
    // Decimal equivalent
    static char dec_buf[20];
    snprintf(dec_buf, sizeof(dec_buf), "= %d sec", dctx->remaining_seconds);
    GFont dec_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    GRect dec_rect = GRect(0, center_y + 25, bounds.size.w, 24);
    graphics_context_set_text_color(ctx, c->secondary);
    graphics_draw_text(ctx, dec_buf, dec_font, dec_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // Progress bar
    int bar_y = bounds.size.h - 30;
    int bar_height = 10;
    int bar_margin = 20;
    int bar_width = bounds.size.w - bar_margin * 2;
    
    graphics_context_set_fill_color(ctx, c->secondary);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 3, GCornersAll);
    
    if (dctx->total_seconds > 0) {
        int progress_width = (dctx->remaining_seconds * bar_width) / dctx->total_seconds;
        graphics_context_set_fill_color(ctx, c->primary);
        graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 3, GCornersAll);
    }
}

// =============================================================================
// Matrix Mode
// =============================================================================

void display_draw_matrix(GContext *ctx, GRect bounds, const DisplayContext *dctx, MatrixState *anim) {
    const VisualizationColors *c = dctx->colors;
    animation_update_matrix(anim, dctx->remaining_seconds);
    
    int col_width = bounds.size.w / MATRIX_COLS;
    int row_height = 14;
    int start_y = 10;
    
    GFont char_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    
    // Falling characters
    for (int col = 0; col < MATRIX_COLS; col++) {
        int drop_head = anim->drops[col];
        int x = col * col_width + col_width / 2 - 4;
        
        for (int row = 0; row < MATRIX_ROWS; row++) {
            int y = start_y + row * row_height;
            
            int dist = drop_head - row;
            if (dist < 0) dist += MATRIX_ROWS + 5;
            
            if (dist <= 6) {
                static char char_buf[2];
                char_buf[0] = anim->chars[col][row];
                char_buf[1] = '\0';
                
                if (dist == 0) {
                    graphics_context_set_text_color(ctx, c->primary);
                } else if (dist <= 2) {
                    graphics_context_set_text_color(ctx, c->secondary);
                } else {
                    graphics_context_set_text_color(ctx, c->accent);
                }
                
                graphics_draw_text(ctx, char_buf, char_font, 
                                  GRect(x, y, 12, 16),
                                  GTextOverflowModeTrailingEllipsis, 
                                  GTextAlignmentCenter, NULL);
            }
        }
    }
    
    // Time display with background
    static char time_buf[16];
    time_format_adaptive(dctx->remaining_seconds, time_buf, sizeof(time_buf));
    
    int time_center_y = bounds.size.h / 2;
    GFont time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
    GRect time_rect = GRect(10, time_center_y - 22, bounds.size.w - 20, 44);
    
    graphics_context_set_fill_color(ctx, c->background);
    graphics_fill_rect(ctx, GRect(15, time_center_y - 20, bounds.size.w - 30, 40), 4, GCornersAll);
    
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, time_buf, time_font, time_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // Progress bar
    int bar_y = bounds.size.h - 8;
    int bar_height = 3;
    int bar_margin = 20;
    int bar_width = bounds.size.w - bar_margin * 2;
    
    if (dctx->total_seconds > 0) {
        int progress_width = (dctx->remaining_seconds * bar_width) / dctx->total_seconds;
        graphics_context_set_fill_color(ctx, c->accent);
        graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 1, GCornersAll);
        graphics_context_set_fill_color(ctx, c->primary);
        graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 1, GCornersAll);
    }
}

// =============================================================================
// Water Level Mode
// =============================================================================

void display_draw_water_level(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_x = bounds.size.w / 2;
    int center_y = bounds.size.h / 2 - 10;
    
    int container_width = 50;
    int container_height = 100;
    int container_top = center_y - container_height / 2;
    int container_bottom = container_top + container_height;
    int container_left = center_x - container_width / 2;
    int container_right = center_x + container_width / 2;
    
    // Container outline
    graphics_context_set_stroke_color(ctx, c->secondary);
    graphics_context_set_stroke_width(ctx, 2);
    
    graphics_draw_line(ctx, GPoint(container_left, container_top + 10), 
                           GPoint(container_left, container_bottom));
    graphics_draw_line(ctx, GPoint(container_right, container_top + 10), 
                           GPoint(container_right, container_bottom));
    graphics_draw_line(ctx, GPoint(container_left, container_bottom), 
                           GPoint(container_right, container_bottom));
    
    int rim_width = container_width + 8;
    graphics_draw_line(ctx, GPoint(center_x - rim_width/2, container_top + 10), 
                           GPoint(center_x + rim_width/2, container_top + 10));
    graphics_draw_line(ctx, GPoint(center_x - rim_width/2, container_top + 10), 
                           GPoint(container_left, container_top + 10));
    graphics_draw_line(ctx, GPoint(center_x + rim_width/2, container_top + 10), 
                           GPoint(container_right, container_top + 10));
    
    // Water level
    int water_height = 0;
    if (dctx->total_seconds > 0) {
        water_height = (dctx->remaining_seconds * (container_height - 20)) / dctx->total_seconds;
    }
    
    if (water_height > 0) {
        int water_top = container_bottom - water_height;
        
        graphics_context_set_fill_color(ctx, c->primary);
        graphics_fill_rect(ctx, GRect(container_left + 1, water_top, 
                                      container_width - 2, water_height), 0, GCornerNone);
        
        // Wave effect
        graphics_context_set_stroke_color(ctx, c->primary);
        graphics_context_set_stroke_width(ctx, 2);
        
        int wave_offset = (dctx->remaining_seconds % 4) - 2;
        for (int x = container_left + 2; x < container_right - 2; x += 3) {
            int y = water_top + (wave_offset * (x % 3 - 1)) / 2;
            if (y >= water_top - 1 && y <= water_top + 1) {
                graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 2, y));
            }
        }
    }
    
    // Measurement marks
    graphics_context_set_stroke_color(ctx, c->accent);
    graphics_context_set_stroke_width(ctx, 1);
    for (int i = 1; i <= 4; i++) {
        int mark_y = container_top + 10 + (i * (container_height - 20) / 5);
        graphics_draw_line(ctx, GPoint(container_left - 5, mark_y), 
                               GPoint(container_left, mark_y));
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, container_bottom + 10, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Spiral Out Mode
// =============================================================================

#define SPIRAL_COLS 9
#define SPIRAL_ROWS 9
#define SPIRAL_PADDING 2

// Generate spiral indices from center outward
// Returns the order in which blocks should fill (0 = first to fill from center)
static int spiral_out_index(int row, int col) {
    int center_row = SPIRAL_ROWS / 2;
    int center_col = SPIRAL_COLS / 2;
    
    // Distance from center determines spiral ring
    int dr = row - center_row;
    int dc = col - center_col;
    int ring = (dr < 0 ? -dr : dr);
    int dc_abs = (dc < 0 ? -dc : dc);
    if (dc_abs > ring) ring = dc_abs;
    
    if (ring == 0) return 0;  // Center block
    
    // Calculate position within ring (clockwise from top)
    int ring_start = (2 * ring - 1) * (2 * ring - 1);  // First index in this ring
    int ring_size = 8 * ring;  // Number of blocks in this ring
    
    int pos = 0;
    if (row == center_row - ring) {
        // Top edge: left to right
        pos = (col - (center_col - ring));
    } else if (col == center_col + ring) {
        // Right edge: top to bottom
        pos = (2 * ring) + (row - (center_row - ring));
    } else if (row == center_row + ring) {
        // Bottom edge: right to left
        pos = (4 * ring) + ((center_col + ring) - col);
    } else if (col == center_col - ring) {
        // Left edge: bottom to top
        pos = (6 * ring) + ((center_row + ring) - row);
    }
    
    return ring_start + (pos % ring_size);
}

void display_draw_spiral_out(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int available_width = bounds.size.w - 20;
    int available_height = bounds.size.h - 60;
    
    int block_width = (available_width - (SPIRAL_COLS - 1) * SPIRAL_PADDING) / SPIRAL_COLS;
    int block_height = (available_height - (SPIRAL_ROWS - 1) * SPIRAL_PADDING) / SPIRAL_ROWS;
    int block_size = (block_width < block_height) ? block_width : block_height;
    
    int grid_width = SPIRAL_COLS * block_size + (SPIRAL_COLS - 1) * SPIRAL_PADDING;
    int grid_height = SPIRAL_ROWS * block_size + (SPIRAL_ROWS - 1) * SPIRAL_PADDING;
    int start_x = (bounds.size.w - grid_width) / 2;
    int start_y = (bounds.size.h - grid_height) / 2 - 10;
    
    int total_blocks = SPIRAL_COLS * SPIRAL_ROWS;
    int filled_blocks = progress_calculate_blocks(dctx->remaining_seconds, dctx->total_seconds, total_blocks);
    
    for (int row = 0; row < SPIRAL_ROWS; row++) {
        for (int col = 0; col < SPIRAL_COLS; col++) {
            int spiral_idx = spiral_out_index(row, col);
            int x = start_x + col * (block_size + SPIRAL_PADDING);
            int y = start_y + row * (block_size + SPIRAL_PADDING);
            GRect block_rect = GRect(x, y, block_size, block_size);
            
            // Spiral out fills from center, so lower spiral indices fill first
            if (spiral_idx < filled_blocks) {
                graphics_context_set_fill_color(ctx, c->primary);
                graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
            } else {
                graphics_context_set_stroke_color(ctx, c->secondary);
                graphics_draw_round_rect(ctx, block_rect, 2);
            }
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, start_y + grid_height + 5, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Percent Elapsed Mode
// =============================================================================

void display_draw_percent(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_y = bounds.size.h / 2;
    
    // Calculate elapsed percentage
    int percent = 0;
    if (dctx->total_seconds > 0) {
        int elapsed = dctx->total_seconds - dctx->remaining_seconds;
        percent = (elapsed * 100) / dctx->total_seconds;
    }
    
    // Large percentage display
    static char percent_buf[8];
    snprintf(percent_buf, sizeof(percent_buf), "%d%%", percent);
    
    GFont large_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    GRect percent_rect = GRect(0, center_y - 35, bounds.size.w, 50);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, percent_buf, large_font, percent_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // "elapsed" label
    GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    GRect label_rect = GRect(0, center_y - 55, bounds.size.w, 20);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, "elapsed", label_font, label_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // Progress bar
    int bar_y = center_y + 25;
    int bar_height = 12;
    int bar_margin = 20;
    int bar_width = bounds.size.w - bar_margin * 2;
    
    // Background bar
    graphics_context_set_fill_color(ctx, c->secondary);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 4, GCornersAll);
    
    // Filled portion (elapsed)
    if (dctx->total_seconds > 0) {
        int elapsed = dctx->total_seconds - dctx->remaining_seconds;
        int progress_width = (elapsed * bar_width) / dctx->total_seconds;
        if (progress_width > 0) {
            graphics_context_set_fill_color(ctx, c->primary);
            graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 4, GCornersAll);
        }
    }
    
    // Remaining time below
    if (!dctx->hide_time_text) {
        GFont time_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect time_rect = GRect(0, bar_y + bar_height + 10, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, time_rect, time_font);
    }
}

// =============================================================================
// Percent Remaining Mode
// =============================================================================

void display_draw_percent_remaining(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int center_y = bounds.size.h / 2;
    
    // Calculate remaining percentage
    int percent = 0;
    if (dctx->total_seconds > 0) {
        percent = (dctx->remaining_seconds * 100) / dctx->total_seconds;
    }
    
    // Large percentage display
    static char percent_buf[8];
    snprintf(percent_buf, sizeof(percent_buf), "%d%%", percent);
    
    GFont large_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    GRect percent_rect = GRect(0, center_y - 35, bounds.size.w, 50);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, percent_buf, large_font, percent_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // "remaining" label
    GFont label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    GRect label_rect = GRect(0, center_y - 55, bounds.size.w, 20);
    graphics_context_set_text_color(ctx, c->primary);
    graphics_draw_text(ctx, "remaining", label_font, label_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // Progress bar
    int bar_y = center_y + 25;
    int bar_height = 12;
    int bar_margin = 20;
    int bar_width = bounds.size.w - bar_margin * 2;
    
    // Background bar
    graphics_context_set_fill_color(ctx, c->secondary);
    graphics_fill_rect(ctx, GRect(bar_margin, bar_y, bar_width, bar_height), 4, GCornersAll);
    
    // Filled portion (remaining)
    if (dctx->total_seconds > 0) {
        int progress_width = (dctx->remaining_seconds * bar_width) / dctx->total_seconds;
        if (progress_width > 0) {
            graphics_context_set_fill_color(ctx, c->primary);
            graphics_fill_rect(ctx, GRect(bar_margin, bar_y, progress_width, bar_height), 4, GCornersAll);
        }
    }
    
    // Remaining time below
    if (!dctx->hide_time_text) {
        GFont time_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect time_rect = GRect(0, bar_y + bar_height + 10, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, time_rect, time_font);
    }
}

// =============================================================================
// Spiral In Mode
// =============================================================================

void display_draw_spiral_in(GContext *ctx, GRect bounds, const DisplayContext *dctx) {
    const VisualizationColors *c = dctx->colors;
    int available_width = bounds.size.w - 20;
    int available_height = bounds.size.h - 60;
    
    int block_width = (available_width - (SPIRAL_COLS - 1) * SPIRAL_PADDING) / SPIRAL_COLS;
    int block_height = (available_height - (SPIRAL_ROWS - 1) * SPIRAL_PADDING) / SPIRAL_ROWS;
    int block_size = (block_width < block_height) ? block_width : block_height;
    
    int grid_width = SPIRAL_COLS * block_size + (SPIRAL_COLS - 1) * SPIRAL_PADDING;
    int grid_height = SPIRAL_ROWS * block_size + (SPIRAL_ROWS - 1) * SPIRAL_PADDING;
    int start_x = (bounds.size.w - grid_width) / 2;
    int start_y = (bounds.size.h - grid_height) / 2 - 10;
    
    int total_blocks = SPIRAL_COLS * SPIRAL_ROWS;
    int filled_blocks = progress_calculate_blocks(dctx->remaining_seconds, dctx->total_seconds, total_blocks);
    
    for (int row = 0; row < SPIRAL_ROWS; row++) {
        for (int col = 0; col < SPIRAL_COLS; col++) {
            int spiral_idx = spiral_out_index(row, col);
            // Invert: higher spiral index (outer) fills first
            int inverted_idx = (total_blocks - 1) - spiral_idx;
            int x = start_x + col * (block_size + SPIRAL_PADDING);
            int y = start_y + row * (block_size + SPIRAL_PADDING);
            GRect block_rect = GRect(x, y, block_size, block_size);
            
            // Spiral in fills from outside, so higher spiral indices fill first
            if (inverted_idx < filled_blocks) {
                graphics_context_set_fill_color(ctx, c->primary);
                graphics_fill_rect(ctx, block_rect, 2, GCornersAll);
            } else {
                graphics_context_set_stroke_color(ctx, c->secondary);
                graphics_draw_round_rect(ctx, block_rect, 2);
            }
        }
    }
    
    if (!dctx->hide_time_text) {
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect text_rect = GRect(0, start_y + grid_height + 5, bounds.size.w, 30);
        draw_time_text(ctx, dctx->remaining_seconds, text_rect, font);
    }
}

// =============================================================================
// Master Draw Function
// =============================================================================

void display_draw(GContext *ctx, GRect bounds, const TimerContext *timer, AnimationState *anim, const VisualizationColors *palettes) {
    DisplayMode mode = timer->display_mode;
    if (mode >= DISPLAY_MODE_COUNT) {
        mode = DISPLAY_MODE_TEXT;
    }
    
    const VisualizationColors *colors = &palettes[mode];
    DisplayContext dctx = display_context_from_timer(timer, colors);
    dctx.display_mode = mode;
    
    // Clear background
    graphics_context_set_fill_color(ctx, colors->background);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
    switch (mode) {
        case DISPLAY_MODE_BLOCKS:
            display_draw_blocks(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_VERTICAL_BLOCKS:
            display_draw_vertical_blocks(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_CLOCK:
            display_draw_clock(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_RING:
            display_draw_ring(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_HOURGLASS:
            display_draw_hourglass(ctx, bounds, &dctx, &anim->hourglass);
            break;
        case DISPLAY_MODE_BINARY:
            display_draw_binary(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_RADIAL:
            display_draw_radial(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_HEX:
            display_draw_hex(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_MATRIX:
            display_draw_matrix(ctx, bounds, &dctx, &anim->matrix);
            break;
        case DISPLAY_MODE_WATER_LEVEL:
            display_draw_water_level(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_SPIRAL_OUT:
            display_draw_spiral_out(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_SPIRAL_IN:
            display_draw_spiral_in(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_PERCENT:
            display_draw_percent(ctx, bounds, &dctx);
            break;
        case DISPLAY_MODE_PERCENT_REMAINING:
            display_draw_percent_remaining(ctx, bounds, &dctx);
            break;
        default:
            break;
    }
}

