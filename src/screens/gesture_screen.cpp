#include "screens/gesture_screen.h"
#include "display_context.h"
#include "display_config.h"
#include "colors.h"

void drawGestureMessage(DisplayContext& dc, Arduino_GFX* gfx, const char* message) {
    gfx->startWrite();

    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.fillRectangle(0, (SCREEN_H - 60) / 2, SCREEN_W, 60);

    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(dc.getWidth() / 2, dc.getHeight() / 2, DisplayContext::FONT_LARGE, message,
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    gfx->endWrite();
}
