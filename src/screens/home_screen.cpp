#include "screens/home_screen.h"
#include "display_context.h"
#include "display_config.h"
#include "colors.h"
#include "assets/icon_bitmap.h"

void drawHomeScreen(DisplayContext& dc, Arduino_GFX* gfx) {
    gfx->startWrite();

    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.clear();

    gfx->draw16bitRGBBitmap(8, 8, (uint16_t*)icon_bitmap, ICON_WIDTH, ICON_HEIGHT);

    dc.drawText(dc.getWidth() / 2, dc.getHeight() / 2, DisplayContext::FONT_LARGE, "testing",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    gfx->endWrite();
}
