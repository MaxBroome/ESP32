#include "screens/boot_screen.h"
#include "display_context.h"
#include "display_config.h"
#include "colors.h"
#include "assets/logo_bitmap.h"
#include <cstring>

#define BOOT_STATUS_Y    ((SCREEN_H - LOGO_HEIGHT) / 2 + LOGO_HEIGHT + 10)
#define BOOT_STATUS_H    40

void drawBootScreen(DisplayContext& dc, Arduino_GFX* gfx, const char* status) {
    static bool logo_drawn = false;

    gfx->startWrite();

    if (!logo_drawn) {
        dc.setColor(COLOR_WHITE, COLOR_BLACK);
        dc.clear();
        int16_t logo_x = (SCREEN_W - LOGO_WIDTH) / 2;
        int16_t logo_y = (SCREEN_H - LOGO_HEIGHT) / 2;
        gfx->draw16bitRGBBitmap(logo_x, logo_y, (uint16_t*)logo_bitmap, LOGO_WIDTH, LOGO_HEIGHT);
        logo_drawn = true;
    }

    dc.setColor(COLOR_BLACK, COLOR_BLACK);
    dc.fillRectangle(0, BOOT_STATUS_Y, SCREEN_W, BOOT_STATUS_H);
    if (status && status[0]) {
        dc.setColor(COLOR_WHITE, COLOR_BLACK);
        int16_t w, h;
        dc.getTextDimensions(status, DisplayContext::FONT_SMALL, &w, &h);
        if (w > 280) {
            const char* p = strstr(status, " to \"");
            if (p && (size_t)(p - status) < 28) {
                size_t line1Len = p - status;
                char line1[32];
                if (line1Len >= sizeof(line1)) line1Len = sizeof(line1) - 1;
                memcpy(line1, status, line1Len);
                line1[line1Len] = '\0';

                const char* line2 = p + 1;
                size_t line2Len = strlen(line2);
                char line2Buf[40];
                if (line2Len >= sizeof(line2Buf)) {
                    memcpy(line2Buf, line2, sizeof(line2Buf) - 4);
                    memcpy(line2Buf + sizeof(line2Buf) - 4, "...", 4);
                    line2 = line2Buf;
                }

                dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + 14, DisplayContext::FONT_SMALL, line1,
                    DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
                dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + 30, DisplayContext::FONT_SMALL, line2,
                    DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
            } else {
                size_t len = strlen(status);
                int mid = (int)len / 2;
                const char* split = nullptr;
                for (const char* s = status + mid; s > status; s--) {
                    if (*s == ' ') { split = s + 1; break; }
                }
                if (!split) {
                    split = (mid > 0 && (size_t)mid < len) ? status + mid : status + len;
                }
                if (split > status && split <= status + len) {
                    char line1[36];
                    size_t line1Len = (split[-1] == ' ')
                        ? (size_t)(split - 1 - status)
                        : (size_t)(split - status);
                    if (line1Len >= sizeof(line1)) line1Len = sizeof(line1) - 1;
                    memcpy(line1, status, line1Len);
                    line1[line1Len] = '\0';

                    size_t line2Len = strlen(split);
                    const char* line2 = split;
                    char line2Buf[40];
                    if (line2Len >= sizeof(line2Buf)) {
                        memcpy(line2Buf, split, sizeof(line2Buf) - 4);
                        memcpy(line2Buf + sizeof(line2Buf) - 4, "...", 4);
                        line2 = line2Buf;
                    }

                    dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + 14, DisplayContext::FONT_SMALL, line1,
                        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
                    dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + 30, DisplayContext::FONT_SMALL, line2,
                        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
                } else {
                    dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + BOOT_STATUS_H / 2, DisplayContext::FONT_SMALL, status,
                        DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
                }
            }
        } else {
            dc.drawText(SCREEN_W / 2, BOOT_STATUS_Y + BOOT_STATUS_H / 2, DisplayContext::FONT_SMALL, status,
                DisplayContext::TEXT_JUSTIFY_CENTER | DisplayContext::TEXT_JUSTIFY_VCENTER);
        }
    }

    gfx->endWrite();
}
