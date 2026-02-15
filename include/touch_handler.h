#pragma once

#include <Wire.h>
#include "axs5106l.h"

enum class GestureType {
    NONE,
    TAP,
    SWIPE_RIGHT_TO_LEFT,
    SWIPE_LEFT_TO_RIGHT,
    SWIPE_TOP_TO_BOTTOM,
    SWIPE_BOTTOM_TO_TOP
};

class TouchHandler {
public:
    TouchHandler();
    
    bool begin();
    bool isTouched();
    void getTouchData(TouchData &td);
    GestureType detectGesture();
    
private:
    AXS5106L touch;
    bool last_touch_state;
    
    // Gesture detection state
    bool touch_in_progress;
    int16_t touch_start_x;
    int16_t touch_start_y;
    int16_t touch_last_x;
    int16_t touch_last_y;
    uint32_t touch_start_time;
    
    // Screen rotation: touch reports in portrait (172x320), display is landscape (320x172)
    // Rotation 1 = 90° CW: touch X becomes screen Y, touch Y becomes screen (320-X)
    void transformCoordinates(int16_t touch_x, int16_t touch_y, int16_t &screen_x, int16_t &screen_y);
    
    static const int16_t SWIPE_MIN_DISTANCE = 50;  // Minimum pixels for swipe
    static const uint32_t SWIPE_MAX_TIME = 500;    // Maximum ms for swipe
    static const int16_t TAP_MAX_MOVEMENT = 10;    // Maximum movement for tap
};