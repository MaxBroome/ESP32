#include "display_context.h"
#include "font_manager.h"

DisplayContext::DisplayContext(Arduino_GFX* gfx) 
    : _gfx(gfx), _fgColor(0xFFFF), _bgColor(0x0000),
      _doubleBufferEnabled(false), _isDrawingToBuffer(false),
      _backBuffer(nullptr), _bufferWidth(0), _bufferHeight(0) {
}

void DisplayContext::clear() {
    _gfx->fillScreen(_bgColor);
}

void DisplayContext::setColor(uint16_t foreground, uint16_t background) {
    _fgColor = foreground;
    _bgColor = background;
    _gfx->setTextColor(foreground, background);
}

void DisplayContext::drawText(int16_t x, int16_t y, Font font, const char* text, uint8_t justification) {
    const GFXfont* gfxFont = getFontForSize(font);
    _gfx->setFont(gfxFont);
    _gfx->setTextColor(_fgColor);
    
    int16_t finalX, finalY;
    calculateTextPosition(x, y, text, gfxFont, justification, &finalX, &finalY);
    
    _gfx->setCursor(finalX, finalY);
    _gfx->print(text);
}

void DisplayContext::fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    _gfx->fillRect(x, y, width, height, _fgColor);
}

void DisplayContext::fillCircle(int16_t x, int16_t y, int16_t radius) {
    _gfx->fillCircle(x, y, radius, _fgColor);
}

void DisplayContext::drawCircle(int16_t x, int16_t y, int16_t radius) {
    _gfx->drawCircle(x, y, radius, _fgColor);
}

void DisplayContext::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    _gfx->drawLine(x1, y1, x2, y2, _fgColor);
}

void DisplayContext::drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    _gfx->drawRect(x, y, width, height, _fgColor);
}

void DisplayContext::drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h) {
    _gfx->draw16bitRGBBitmap(x, y, bitmap, w, h);
}

void DisplayContext::setClip(int16_t x, int16_t y, int16_t width, int16_t height) {
    // Arduino_GFX clipping via writeAddrWindow during startWrite
    // This is a simplified implementation - full clipping would require
    // checking bounds on every draw operation
}

void DisplayContext::clearClip() {
    // Reset clipping region
}

void DisplayContext::enableDoubleBuffer(bool enable) {
    // For now, double buffering is handled by drawing operations
    // being batched between startWrite/endWrite calls
    _doubleBufferEnabled = enable;
}

void DisplayContext::swapBuffers() {
    // Flush any pending operations
    if (_doubleBufferEnabled) {
        _gfx->flush();
    }
}

int16_t DisplayContext::getWidth() const {
    return _gfx->width();
}

int16_t DisplayContext::getHeight() const {
    return _gfx->height();
}

void DisplayContext::getTextDimensions(const char* text, Font font, int16_t* width, int16_t* height) {
    const GFXfont* gfxFont = getFontForSize(font);
    _gfx->setFont(gfxFont);
    
    int16_t x1, y1;
    uint16_t w, h;
    _gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    *width = w;
    *height = h;
}

const GFXfont* DisplayContext::getFontForSize(Font font) {
    switch (font) {
        case FONT_SMALL:      return FontManager::small();
        case FONT_MEDIUM:     return FontManager::body();
        case FONT_LARGE:      return FontManager::heading();
        case FONT_XLARGE:     return FontManager::display();
        case FONT_MONO:       return FontManager::mono();
        case FONT_MONO_SMALL: return FontManager::monoSmall();
        default:              return FontManager::body();
    }
}

void DisplayContext::calculateTextPosition(int16_t x, int16_t y, const char* text, 
                                           const GFXfont* font, uint8_t justification,
                                           int16_t* outX, int16_t* outY) {
    _gfx->setFont(font);
    
    int16_t x1, y1;
    uint16_t w, h;
    _gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    // Horizontal justification
    if (justification & TEXT_JUSTIFY_CENTER) {
        *outX = x - (w / 2) - x1;
    } else if (justification & TEXT_JUSTIFY_RIGHT) {
        *outX = x - w - x1;
    } else {
        *outX = x - x1;
    }
    
    // Vertical justification
    if (justification & TEXT_JUSTIFY_VCENTER) {
        *outY = y - (h / 2) - y1;
    } else if (justification & TEXT_JUSTIFY_BOTTOM) {
        *outY = y - h - y1;
    } else {
        *outY = y - y1;
    }
}

void DisplayContext::initDoubleBuffer() {
    // Not needed with hardware-accelerated approach
}

void DisplayContext::freeDoubleBuffer() {
    // Not needed with hardware-accelerated approach
}