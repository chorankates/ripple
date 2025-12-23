#pragma once

#include <pebble.h>
#include "../timer_state.h"
#include "../time_utils.h"
#include "../colors.h"

// =============================================================================
// Display Module Common Interface
// =============================================================================
// All display modes share this interface for consistent rendering

// =============================================================================
// Display Context - Read-only Timer State for Rendering
// =============================================================================

typedef struct {
    int remaining_seconds;
    int total_seconds;
    TimerState state;
    DisplayMode display_mode;
} DisplayContext;

// Create display context from timer context
DisplayContext display_context_from_timer(const TimerContext *timer);

// =============================================================================
// Hourglass Animation State
// =============================================================================

#define MAX_SAND_PARTICLES 48

typedef struct {
    int sand_top[MAX_SAND_PARTICLES];
    int sand_bottom[MAX_SAND_PARTICLES];
    int num_sand_top;
    int num_sand_bottom;
} HourglassState;

// =============================================================================
// Matrix Rain Animation State
// =============================================================================

#define MATRIX_COLS 12
#define MATRIX_ROWS 10

typedef struct {
    int drops[MATRIX_COLS];
    int chars[MATRIX_COLS][MATRIX_ROWS];
    int speeds[MATRIX_COLS];
} MatrixState;

// =============================================================================
// Animation State Container
// =============================================================================

typedef struct {
    HourglassState hourglass;
    MatrixState matrix;
} AnimationState;

// =============================================================================
// Animation Initialization
// =============================================================================

void animation_init_hourglass(HourglassState *state);
void animation_init_matrix(MatrixState *state, int seed);

// =============================================================================
// Animation Updates
// =============================================================================

void animation_update_hourglass(HourglassState *state, int remaining_seconds, int total_seconds);
void animation_update_matrix(MatrixState *state, int remaining_seconds);

// =============================================================================
// Display Mode Draw Functions
// =============================================================================

void display_draw_blocks(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_vertical_blocks(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_clock(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_ring(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_hourglass(GContext *ctx, GRect bounds, const DisplayContext *dctx, HourglassState *anim);
void display_draw_binary(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_radial(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_hex(GContext *ctx, GRect bounds, const DisplayContext *dctx);
void display_draw_matrix(GContext *ctx, GRect bounds, const DisplayContext *dctx, MatrixState *anim);
void display_draw_water_level(GContext *ctx, GRect bounds, const DisplayContext *dctx);

// =============================================================================
// Master Draw Function
// =============================================================================

// Draw the appropriate display mode, handling animation state internally
void display_draw(GContext *ctx, GRect bounds, const TimerContext *timer, AnimationState *anim);

