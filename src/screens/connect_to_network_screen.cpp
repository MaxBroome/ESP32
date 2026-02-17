#include "screens/connect_to_network_screen.h"
#include "display_context.h"
#include "display_config.h"
#include "colors.h"

void drawConnectToNetworkScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* apSsid) {
    gfx->startWrite();

    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.clear();

    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(SCREEN_W / 2, 14, DisplayContext::FONT_SMALL, "Connect to a Network",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    dc.setColor(COLOR_DARK_GRAY, COLOR_BLACK);
    dc.fillRectangle(12, 26, SCREEN_W - 24, 1);

    const int ICON_SZ = 20;
    const int ICON_X = 12;
    const int ROW_H = 42;
    const int LABEL_Y_OFF = 2;
    const int DESC_Y_OFF = 24;
    const int GAP_ABOVE_OR = 22;
    const int GAP_BELOW_OR = 18;
    const int TEXT_X = ICON_X + ICON_SZ + 12;

    const int ROW1_Y = 38;
    const int WIFI_CX = ICON_X + ICON_SZ / 2;
    const int WIFI_CY = ROW1_Y + ICON_SZ - 2;

    dc.setColor(COLOR_BRAND, COLOR_BLACK);
    dc.fillCircle(WIFI_CX, WIFI_CY, 2);
    dc.drawCircle(WIFI_CX, WIFI_CY, 5);
    dc.drawCircle(WIFI_CX, WIFI_CY, 9);
    dc.drawCircle(WIFI_CX, WIFI_CY, 13);

    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(TEXT_X, ROW1_Y + LABEL_Y_OFF, DisplayContext::FONT_SMALL, "WiFi Setup",
        DisplayContext::TEXT_JUSTIFY_LEFT | DisplayContext::TEXT_JUSTIFY_TOP);
    dc.setColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    dc.drawText(TEXT_X, ROW1_Y + DESC_Y_OFF, DisplayContext::FONT_SMALL, apSsid ? apSsid : "",
        DisplayContext::TEXT_JUSTIFY_LEFT | DisplayContext::TEXT_JUSTIFY_TOP);

    const int OR_Y = ROW1_Y + ROW_H + GAP_ABOVE_OR;
    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(SCREEN_W / 2, OR_Y, DisplayContext::FONT_SMALL, "OR",
        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);

    const int ROW2_Y = OR_Y + GAP_BELOW_OR;
    dc.setColor(COLOR_BRAND, COLOR_BLACK);
    dc.drawLine(ICON_X + 2, ROW2_Y + ICON_SZ / 2, ICON_X + ICON_SZ - 2, ROW2_Y + ICON_SZ / 2);
    dc.fillRectangle(ICON_X, ROW2_Y + 4, 6, 10);
    dc.fillRectangle(ICON_X + ICON_SZ - 6, ROW2_Y + 4, 6, 10);

    dc.setColor(COLOR_WHITE, COLOR_BLACK);
    dc.drawText(TEXT_X, ROW2_Y + LABEL_Y_OFF, DisplayContext::FONT_SMALL, "Ethernet",
        DisplayContext::TEXT_JUSTIFY_LEFT | DisplayContext::TEXT_JUSTIFY_TOP);
    dc.setColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    dc.drawText(TEXT_X, ROW2_Y + DESC_Y_OFF, DisplayContext::FONT_SMALL, "Plug in cable",
        DisplayContext::TEXT_JUSTIFY_LEFT | DisplayContext::TEXT_JUSTIFY_TOP);

    gfx->endWrite();
}
