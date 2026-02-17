#include "screens/onboarding_screen.h"
#include "display_context.h"
#include "display_config.h"
#include "colors.h"

#define ONBOARDING_STATUS_H 24

void drawOnboardingScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* code) {
    gfx->startWrite();

    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.clear();

    dc.setColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    dc.drawText(dc.getWidth() / 2, 24, DisplayContext::FONT_SMALL, "scorescrape.io/dashboard",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    dc.setColor(COLOR_BRAND, COLOR_BLACK);
    dc.drawText(dc.getWidth() / 2, dc.getHeight() / 2, DisplayContext::FONT_XLARGE, code,
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    gfx->endWrite();
}

void drawOnboardingStatus(Arduino_Canvas* canvas, const char* msg) {
    if (!canvas || !msg) return;

    DisplayContext dc_status(canvas);
    dc_status.setColor(COLOR_BLACK, COLOR_BLACK);
    dc_status.fillRectangle(0, 0, SCREEN_W, ONBOARDING_STATUS_H);

    dc_status.setColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    dc_status.drawText(SCREEN_W / 2, ONBOARDING_STATUS_H / 2, DisplayContext::FONT_SMALL, msg,
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    canvas->flush();
}
