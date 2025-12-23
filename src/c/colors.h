#pragma once

#include <pebble.h>

// =============================================================================
// Platform-Aware Color Definitions
// =============================================================================
// Color Pebble: Basalt, Chalk, Emery
// B&W Pebble: Aplite, Diorite

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
  #define COLOR_SPIRAL_FULL GColorMagenta
  #define COLOR_SPIRAL_EMPTY GColorDarkGray
  #define COLOR_PERCENT_TEXT GColorChromeYellow
  #define COLOR_PERCENT_BAR GColorChromeYellow
  #define COLOR_PERCENT_BAR_BG GColorDarkGray
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
  #define COLOR_SPIRAL_FULL GColorWhite
  #define COLOR_SPIRAL_EMPTY GColorBlack
  #define COLOR_PERCENT_TEXT GColorWhite
  #define COLOR_PERCENT_BAR GColorWhite
  #define COLOR_PERCENT_BAR_BG GColorBlack
#endif

