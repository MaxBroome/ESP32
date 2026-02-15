#pragma once

// ScoreScrape icon drawing function
// Draws a simplified version of the checkmark icon

#include <Arduino_GFX_Library.h>

void drawIcon(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t size) {
    // Colors from the icon
    const uint16_t COLOR_DARK_BLUE = 0x444B;   // #444ce7
    const uint16_t COLOR_MED_BLUE = 0x6172;    // #6172f3
    const uint16_t COLOR_LIGHT_BLUE = 0xA4BC;  // #a4bcfd
    
    // Scale factor
    float scale = size / 100.0;
    
    // Helper function to draw a rotated rectangle (checkmark stroke)
    auto drawCheckStroke = [&](int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, int16_t thickness) {
        // Draw thick line by drawing multiple parallel lines
        for (int i = -thickness/2; i <= thickness/2; i++) {
            gfx->drawLine(x + x1 * scale + i, y + y1 * scale, 
                         x + x2 * scale + i, y + y2 * scale, color);
            gfx->drawLine(x + x1 * scale, y + y1 * scale + i, 
                         x + x2 * scale, y + y2 * scale + i, color);
        }
    };
    
    // Draw the main checkmark strokes (dark blue)
    // Long diagonal stroke (top-right to bottom-left)
    drawCheckStroke(65, 15, 30, 50, COLOR_DARK_BLUE, 8);
    drawCheckStroke(30, 50, 15, 65, COLOR_DARK_BLUE, 8);
    
    // Short diagonal stroke (bottom-left to middle)
    drawCheckStroke(15, 65, 30, 50, COLOR_DARK_BLUE, 8);
    drawCheckStroke(30, 50, 40, 40, COLOR_DARK_BLUE, 8);
    
    // Additional strokes for depth
    drawCheckStroke(50, 25, 35, 40, COLOR_DARK_BLUE, 7);
    drawCheckStroke(75, 35, 55, 55, COLOR_DARK_BLUE, 7);
    
    // Medium blue accents
    drawCheckStroke(25, 20, 35, 30, COLOR_MED_BLUE, 6);
    drawCheckStroke(35, 70, 25, 80, COLOR_MED_BLUE, 6);
    drawCheckStroke(45, 60, 35, 70, COLOR_MED_BLUE, 6);
    
    // Light blue dots/accents
    gfx->fillCircle(x + 80 * scale, y + 65 * scale, 5 * scale, COLOR_LIGHT_BLUE);
    gfx->fillCircle(x + 15 * scale, y + 55 * scale, 4 * scale, COLOR_LIGHT_BLUE);
    gfx->fillCircle(x + 40 * scale, y + 10 * scale, 4 * scale, COLOR_LIGHT_BLUE);
}
