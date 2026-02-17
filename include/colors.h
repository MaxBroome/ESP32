#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// RGB565 Color Constants (MonkeyC style)
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_ORANGE      0xFC00
#define COLOR_PURPLE      0x8010
#define COLOR_GRAY        0x8410
#define COLOR_DARK_GRAY   0x4208
#define COLOR_LIGHT_GRAY  0xC618

// Brand color #6767e4 (RGB565)
#define COLOR_BRAND       0x633C

// Transparent (for overlays)
#define COLOR_TRANSPARENT 0xFFFF

#endif // COLORS_H