#include "touch_handler.h"
#include "board_config.h"

TouchHandler::TouchHandler() 
    : touch(Wire, PIN_TP_RST, PIN_TP_INT), 
      last_touch_state(false),
      touch_in_progress(false),
      touch_start_x(0),
      touch_start_y(0),
      touch_last_x(0),
      touch_last_y(0),
      touch_start_time(0) {
}

bool TouchHandler::begin() {
    Wire.begin(PIN_TP_SDA, PIN_TP_SCL, 400000);
    return touch.begin();
}

bool TouchHandler::isTouched() {
    TouchData td;
    return touch.read(td);
}

void TouchHandler::getTouchData(TouchData &td) {
    touch.read(td);
}

void TouchHandler::transformCoordinates(int16_t touch_x, int16_t touch_y, int16_t &screen_x, int16_t &screen_y) {
    // Rotation 1 (90° CW): 
    // Touch native: 172x320 (portrait)
    // Screen: 320x172 (landscape)
    // Transform: screen_x = touch_y, screen_y = (172 - touch_x)
    screen_x = touch_y;
    screen_y = 172 - touch_x;
}

GestureType TouchHandler::detectGesture() {
    TouchData td;
    bool is_touched = touch.read(td);
    
    if (is_touched && td.count > 0) {
        int16_t touch_x = td.points[0].x;
        int16_t touch_y = td.points[0].y;
        
        // Transform to screen coordinates
        int16_t screen_x, screen_y;
        transformCoordinates(touch_x, touch_y, screen_x, screen_y);
        
        if (!touch_in_progress) {
            // Start tracking a new touch
            touch_in_progress = true;
            touch_start_x = screen_x;
            touch_start_y = screen_y;
            touch_last_x = screen_x;
            touch_last_y = screen_y;
            touch_start_time = millis();
        } else {
            // Update last position during touch
            touch_last_x = screen_x;
            touch_last_y = screen_y;
        }
        
        return GestureType::NONE;
    } else {
        // Touch released - determine gesture type
        if (touch_in_progress) {
            touch_in_progress = false;
            
            int16_t delta_x = touch_last_x - touch_start_x;
            int16_t delta_y = touch_last_y - touch_start_y;
            uint32_t duration = millis() - touch_start_time;
            
            int16_t abs_delta_x = abs(delta_x);
            int16_t abs_delta_y = abs(delta_y);
            
            // Check if it's a swipe (significant movement)
            if (duration <= SWIPE_MAX_TIME) {
                // Horizontal swipe (x movement > y movement)
                if (abs_delta_x > abs_delta_y && abs_delta_x >= SWIPE_MIN_DISTANCE) {
                    if (delta_x < 0) {
                        return GestureType::SWIPE_RIGHT_TO_LEFT;
                    } else {
                        return GestureType::SWIPE_LEFT_TO_RIGHT;
                    }
                }
                // Vertical swipe
                else if (abs_delta_y >= SWIPE_MIN_DISTANCE) {
                    if (delta_y < 0) {
                        return GestureType::SWIPE_BOTTOM_TO_TOP;
                    } else {
                        return GestureType::SWIPE_TOP_TO_BOTTOM;
                    }
                }
            }
            
            // If not a swipe, check if it's a tap (minimal movement)
            if (abs_delta_x <= TAP_MAX_MOVEMENT && abs_delta_y <= TAP_MAX_MOVEMENT) {
                return GestureType::TAP;
            }
        }
        
        return GestureType::NONE;
    }
}