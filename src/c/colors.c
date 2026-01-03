#include "colors.h"

static VisualizationColors make_palette(GColor background, GColor primary, GColor secondary, GColor accent) {
    VisualizationColors palette = {
        .background = background,
        .primary = primary,
        .secondary = secondary,
        .accent = accent
    };
    return palette;
}

void colors_load_default_palettes(VisualizationColors palettes[DISPLAY_MODE_COUNT]) {
    #ifdef PBL_COLOR
        const GColor bg = GColorBlack;
        palettes[DISPLAY_MODE_TEXT]            = make_palette(bg, GColorWhite, GColorLightGray, GColorWhite);
        palettes[DISPLAY_MODE_BLOCKS]          = make_palette(bg, GColorVividCerulean, GColorDarkGray, GColorVividCerulean);
        palettes[DISPLAY_MODE_VERTICAL_BLOCKS] = make_palette(bg, GColorVividCerulean, GColorDarkGray, GColorVividCerulean);
        palettes[DISPLAY_MODE_CLOCK]           = make_palette(bg, GColorMelon, GColorWhite, GColorRed);
        palettes[DISPLAY_MODE_RING]            = make_palette(bg, GColorCyan, GColorDarkGray, GColorCyan);
        palettes[DISPLAY_MODE_HOURGLASS]       = make_palette(bg, GColorRajah, GColorWhite, GColorRajah);
        palettes[DISPLAY_MODE_BINARY]          = make_palette(bg, GColorMintGreen, GColorDarkGray, GColorMintGreen);
        palettes[DISPLAY_MODE_RADIAL]          = make_palette(bg, GColorRed, GColorOrange, GColorYellow);
        palettes[DISPLAY_MODE_HEX]             = make_palette(bg, GColorVividViolet, GColorLightGray, GColorVividViolet);
        palettes[DISPLAY_MODE_MATRIX]          = make_palette(bg, GColorBrightGreen, GColorGreen, GColorDarkGreen);
        palettes[DISPLAY_MODE_WATER_LEVEL]     = make_palette(bg, GColorVividCerulean, GColorWhite, GColorVividCerulean);
        palettes[DISPLAY_MODE_SPIRAL_OUT]      = make_palette(bg, GColorMagenta, GColorDarkGray, GColorMagenta);
        palettes[DISPLAY_MODE_SPIRAL_IN]       = make_palette(bg, GColorMagenta, GColorDarkGray, GColorMagenta);
        palettes[DISPLAY_MODE_PERCENT]         = make_palette(bg, GColorChromeYellow, GColorDarkGray, GColorChromeYellow);
        palettes[DISPLAY_MODE_PERCENT_REMAINING] = make_palette(bg, GColorChromeYellow, GColorDarkGray, GColorChromeYellow);
    #else
        // Monochrome defaults
        const GColor bg = GColorBlack;
        const GColor primary = GColorWhite;
        const GColor secondary = GColorBlack;
        palettes[DISPLAY_MODE_TEXT]            = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_BLOCKS]          = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_VERTICAL_BLOCKS] = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_CLOCK]           = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_RING]            = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_HOURGLASS]       = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_BINARY]          = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_RADIAL]          = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_HEX]             = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_MATRIX]          = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_WATER_LEVEL]     = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_SPIRAL_OUT]      = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_SPIRAL_IN]       = make_palette(bg, primary, secondary, primary);
        palettes[DISPLAY_MODE_PERCENT]         = make_palette(bg, primary, primary, primary);
        palettes[DISPLAY_MODE_PERCENT_REMAINING] = make_palette(bg, primary, primary, primary);
    #endif
}


