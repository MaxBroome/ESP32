#include "display.h"
#include <SPI.h>

bool Display::begin(int8_t mosi, int8_t sclk, int8_t cs, int8_t dc,
                    int8_t rst, int8_t bl,
                    uint16_t width, uint16_t height,
                    uint8_t col_offset, uint8_t row_offset) {
    _bl_pin = bl;

    // Initialize SPI explicitly for ESP32-P4
    SPI.begin(sclk, -1, mosi, cs);
    
    _bus = new Arduino_ESP32SPI(dc, cs, sclk, mosi, -1 /* miso */, HSPI /* spi_num */);
    
    // Try using ST7789 with IPS flag set to true
    _gfx = new Arduino_ST7789(
        _bus, rst, 0 /* rotation */, true /* IPS */,
        width, height,
        col_offset, row_offset,
        col_offset, row_offset);

    if (!_gfx->begin(80000000)) {  // 80MHz SPI speed
        return false;
    }

    // Send JD9853 specific initialization
    _bus->beginWrite();
    
    // MADCTL (Memory Access Control)
    _bus->writeC8D8(0x36, 0x00);  // Try different orientations
    
    // Pixel Format Set
    _bus->writeC8D8(0x3A, 0x55);  // 16-bit color
    
    _bus->endWrite();

    if (_bl_pin >= 0) {
        pinMode(_bl_pin, OUTPUT);
        setBacklight(true);
    }

    delay(100);
    clear(COLOR_RED);  // Try filling with red first
    delay(500);
    clear(COLOR_BLACK);
    return true;
}

void Display::setBacklight(bool on) {
    if (_bl_pin >= 0) {
        digitalWrite(_bl_pin, on ? HIGH : LOW);
    }
}

void Display::clear(uint16_t color) {
    if (_gfx) {
        _gfx->fillScreen(color);
    }
}

void Display::printText(const char *text, int16_t x, int16_t y,
                        uint16_t color, uint8_t size) {
    if (!_gfx) return;
    _gfx->setCursor(x, y);
    _gfx->setTextColor(color);
    _gfx->setTextSize(size);
    _gfx->print(text);
}
