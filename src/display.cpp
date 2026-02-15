#include "display.h"
#include "board_config.h"
#include "display_context.h"
#include "colors.h"
#include "assets/logo_bitmap.h"
#include "assets/icon_bitmap.h"

#define SCREEN_W 320
#define SCREEN_H 172

Display::Display() : bus(nullptr), gfx(nullptr), dc(nullptr) {
    // Software SPI bus
    bus = new Arduino_SWSPI(
        PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_MOSI, GFX_NOT_DEFINED);

    // ST7789 driver — rotation 0 at init, we'll set rotation 1 after MADCTL fix
    gfx = new Arduino_ST7789(
        bus, PIN_LCD_RST, 0 /* rotation */, false /* IPS */,
        172 /* width */, 320 /* height */,
        34 /* col_offset1 */, 0 /* row_offset1 */,
        34 /* col_offset2 */, 0 /* row_offset2 */);
}

Display::~Display() {
    delete gfx;
    delete bus;
}

bool Display::begin() {
    // Backlight on
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    // Init display
    gfx->begin();
    gfx->setRotation(1);  // 90° CW → landscape 320x172
    
    applyColorFix();
    
    // Initialize DisplayContext after gfx is ready
    dc = DisplayContext(gfx);
    
    // Enable double buffering for flicker-free updates
    // Note: This uses ~110KB of RAM (320x172x2 bytes)
    // Comment out if you need the RAM for other things
    dc.enableDoubleBuffer(true);
    
    return true;
}

void Display::applyColorFix() {
    // JD9853 quirk: needs BGR color order. setRotation wrote MADCTL for
    // rotation but with RGB. Read what it set and OR in the BGR bit (0x08).
    // Rotation 1 on ST7789 = MV+MX = 0x60, add BGR = 0x68.
    bus->beginWrite();
    bus->writeC8D8(0x36, 0x28);
    bus->endWrite();
    gfx->invertDisplay(false);
}

void Display::showBootScreen() {
    dc.clear();
    int16_t logo_x = (SCREEN_W - LOGO_WIDTH) / 2;
    int16_t logo_y = (SCREEN_H - LOGO_HEIGHT) / 2;
    gfx->draw16bitRGBBitmap(logo_x, logo_y, (uint16_t *)logo_bitmap, LOGO_WIDTH, LOGO_HEIGHT);
}

void Display::showHomeScreen() {
    // Batch all drawing operations for faster update
    gfx->startWrite();
    
    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.clear();

    // Icon in top-left with small margin
    gfx->draw16bitRGBBitmap(8, 8, (uint16_t *)icon_bitmap, ICON_WIDTH, ICON_HEIGHT);

    // "testing" text centered on screen - MonkeyC style!
    dc.drawText(
        dc.getWidth() / 2,
        dc.getHeight() / 2,
        DisplayContext::FONT_LARGE,
        "testing",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER
    );
    
    gfx->endWrite();  // Flush all operations at once
}

void Display::showTappedMessage() {
    gfx->startWrite();
    
    // Clear text area
    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.fillRectangle(0, (SCREEN_H - 60) / 2, SCREEN_W, 60);

    // Draw centered text
    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(
        dc.getWidth() / 2,
        dc.getHeight() / 2,
        DisplayContext::FONT_LARGE,
        "tapped",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER
    );
    
    gfx->endWrite();
}

void Display::showSwipedMessage() {
    gfx->startWrite();
    
    // Clear text area
    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.fillRectangle(0, (SCREEN_H - 60) / 2, SCREEN_W, 60);

    // Draw centered text
    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(
        dc.getWidth() / 2,
        dc.getHeight() / 2,
        DisplayContext::FONT_LARGE,
        "swiped",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER
    );
    
    gfx->endWrite();
}