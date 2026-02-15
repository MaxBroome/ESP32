#pragma once

// Hold the BOOT button at power-on to factory reset (wipes NVS, reboots).
// GPIO35 is shared with the Ethernet RMII bus on this board, so the button
// is only usable before ETH.begin() — call disable() once ethernet is up.

#include <Arduino.h>

class ResetButton {
public:
    void begin(uint8_t pin, uint32_t hold_ms = 5000);
    void check();
    void disable();
    bool waitForHold();  // blocks until hold completes or button released

private:
    uint8_t  pin_         = 0;
    uint32_t hold_ms_     = 5000;
    uint32_t press_start_ = 0;
    bool     pressed_     = false;
    bool     enabled_     = false;
};