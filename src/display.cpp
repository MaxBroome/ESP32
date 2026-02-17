#include "display.h"
#include "board_config.h"
#include "display_config.h"
#include "display_context.h"
#include "screens/boot_screen.h"
#include "screens/connect_to_network_screen.h"
#include "screens/onboarding_screen.h"
#include "screens/home_screen.h"
#include "screens/gesture_screen.h"

Display::Display() : bus(nullptr), gfx(nullptr), dc(nullptr) {
    bus = new Arduino_SWSPI(
        PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_MOSI, GFX_NOT_DEFINED);

    gfx = new Arduino_ST7789(
        bus, PIN_LCD_RST, 0, false,
        172, 320,
        34, 0, 34, 0);
}

Display::~Display() {
    if (status_canvas_) {
        delete status_canvas_;
        status_canvas_ = nullptr;
    }
    delete gfx;
    delete bus;
}

bool Display::begin() {
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    gfx->begin();
    gfx->setRotation(1);
    applyColorFix();

    dc = DisplayContext(gfx);
    dc.enableDoubleBuffer(true);

    status_canvas_ = new Arduino_Canvas(SCREEN_W, 24, gfx, 0, SCREEN_H - 24, 0);
    if (status_canvas_ && !status_canvas_->begin(GFX_SKIP_OUTPUT_BEGIN)) {
        delete status_canvas_;
        status_canvas_ = nullptr;
    }

    return true;
}

void Display::applyColorFix() {
    bus->beginWrite();
    bus->writeC8D8(0x36, 0x28);
    bus->endWrite();
    gfx->invertDisplay(false);
}

void Display::showBootScreen(const char* status) {
    drawBootScreen(dc, gfx, status);
}

void Display::showConnectToNetworkScreen(const char* apSsid) {
    drawConnectToNetworkScreen(dc, gfx, apSsid);
}

void Display::showOnboardingScreen(const char* code) {
    drawOnboardingScreen(dc, gfx, code);
}

void Display::updateOnboardingStatus(const char* msg) {
    drawOnboardingStatus(status_canvas_, msg);
}

void Display::showHomeScreen() {
    drawHomeScreen(dc, gfx);
}

void Display::showTappedMessage() {
    drawGestureMessage(dc, gfx, "tapped");
}

void Display::showSwipedMessage() {
    drawGestureMessage(dc, gfx, "swiped");
}
