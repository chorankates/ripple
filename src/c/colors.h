#pragma once

#include <pebble.h>
#include "timer_state.h"

// =============================================================================
// Text & Hint Colors (global)
// =============================================================================
// Text colors remain global so text layers stay readable regardless of
// visualization palette. Visualization colors are configured per mode.

#ifdef PBL_COLOR
  #define COLOR_TEXT_NORMAL GColorWhite
  #define COLOR_TEXT_RUNNING GColorGreen
  #define COLOR_TEXT_PAUSED GColorYellow
  #define COLOR_TEXT_LOW GColorRed
  #define COLOR_TEXT_COMPLETED GColorBrightGreen
  #define COLOR_HINT GColorLightGray
#else
  #define COLOR_TEXT_NORMAL GColorWhite
  #define COLOR_TEXT_RUNNING GColorWhite
  #define COLOR_TEXT_PAUSED GColorWhite
  #define COLOR_TEXT_LOW GColorWhite
  #define COLOR_TEXT_COMPLETED GColorWhite
  #define COLOR_HINT GColorWhite
#endif

// =============================================================================
// Visualization Palette
// =============================================================================
// Users can customize these per visualization. The defaults match the previous
// hard-coded colors.

typedef struct {
    GColor background;  // Canvas background for the visualization
    GColor primary;     // Main foreground / fill color
    GColor secondary;   // Secondary / outline / background accents
    GColor accent;      // Optional accent (third tone)
} VisualizationColors;

// Populate an array with the default palette for every display mode
void colors_load_default_palettes(VisualizationColors palettes[DISPLAY_MODE_COUNT]);

