#ifndef DISPLAY_CONTEXT_H
#define DISPLAY_CONTEXT_H

#include <Arduino_GFX_Library.h>

/**
 * DisplayContext - MonkeyC-style drawing API
 * 
 * Simplified drawing interface inspired by Garmin's MonkeyC.
 * Makes common operations like centered text trivial.
 * 
 * Features:
 * - Double buffering for flicker-free updates
 * - Automatic text centering and justification
 * - Clipping regions
 * - Color management
 */
class DisplayContext {
public:
    // Text justification constants (MonkeyC style)
    static constexpr uint8_t TEXT_JUSTIFY_LEFT    = 0x01;
    static constexpr uint8_t TEXT_JUSTIFY_CENTER  = 0x02;
    static constexpr uint8_t TEXT_JUSTIFY_RIGHT   = 0x04;
    static constexpr uint8_t TEXT_JUSTIFY_VCENTER = 0x08;
    static constexpr uint8_t TEXT_JUSTIFY_TOP     = 0x10;
    static constexpr uint8_t TEXT_JUSTIFY_BOTTOM  = 0x20;

    // Font size constants (semantic)
    enum Font {
        FONT_SMALL,
        FONT_MEDIUM,
        FONT_LARGE,
        FONT_XLARGE,
        FONT_MONO,
        FONT_MONO_SMALL
    };

    DisplayContext(Arduino_GFX* gfx);

    // Core drawing methods (MonkeyC style)
    void clear();
    void setColor(uint16_t foreground, uint16_t background);
    
    void drawText(int16_t x, int16_t y, Font font, const char* text, uint8_t justification);
    void fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    void fillCircle(int16_t x, int16_t y, int16_t radius);
    void drawCircle(int16_t x, int16_t y, int16_t radius);
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    void drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    
    // Clipping (MonkeyC style)
    void setClip(int16_t x, int16_t y, int16_t width, int16_t height);
    void clearClip();
    
    // Double buffering (prevents flicker)
    void enableDoubleBuffer(bool enable);
    void swapBuffers();  // Call this to display everything at once
    
    // Utility methods
    int16_t getWidth() const;
    int16_t getHeight() const;
    void getTextDimensions(const char* text, Font font, int16_t* width, int16_t* height);

    // Direct GFX access for advanced operations
    Arduino_GFX* getGfx() { return _gfx; }

private:
    Arduino_GFX* _gfx;
    uint16_t _fgColor;
    uint16_t _bgColor;
    bool _doubleBufferEnabled;
    bool _isDrawingToBuffer;
    
    // Buffer for double buffering
    uint16_t* _backBuffer;
    int16_t _bufferWidth;
    int16_t _bufferHeight;

    const GFXfont* getFontForSize(Font font);
    void calculateTextPosition(int16_t x, int16_t y, const char* text, 
                              const GFXfont* font, uint8_t justification,
                              int16_t* outX, int16_t* outY);
    void initDoubleBuffer();
    void freeDoubleBuffer();
};

#endif // DISPLAY_CONTEXT_H