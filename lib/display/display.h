#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// RGB565 color constants
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F

class Display {
public:
    bool begin(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc,
               int8_t rst, int8_t bl,
               uint16_t width = 172, uint16_t height = 320,
               uint8_t col_offset = 34, uint8_t row_offset = 0);

    void setBacklight(bool on);
    void clear(uint16_t color = COLOR_BLACK);
    void printText(const char *text, int16_t x, int16_t y,
                   uint16_t color = COLOR_WHITE, uint8_t size = 2);

    Arduino_GFX *gfx() { return _gfx; }

private:
    Arduino_DataBus *_bus = nullptr;
    Arduino_GFX     *_gfx = nullptr;
    int8_t           _bl_pin = -1;
};
