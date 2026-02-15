#include "reset_button.h"
#include "nvs_manager.h"

void ResetButton::begin(uint8_t pin, uint32_t hold_ms) {
    pin_     = pin;
    hold_ms_ = hold_ms;
    enabled_ = true;
    pinMode(pin_, INPUT_PULLUP);
    Serial.printf("[RST] GPIO%d, hold %lums to factory reset\n", pin_, hold_ms_);
}

void ResetButton::disable() {
    enabled_ = false;
    pressed_ = false;
}

bool ResetButton::waitForHold() {
    if (!enabled_) return false;

    // Quick check — if button isn't pressed right now, don't block at all
    if (digitalRead(pin_) == HIGH) return false;

    Serial.println("[RST] Button held, waiting for threshold...");
    uint32_t start = millis();

    while (digitalRead(pin_) == LOW) {
        uint32_t elapsed = millis() - start;

        if (elapsed >= hold_ms_) {
            Serial.println("[RST] Factory reset triggered");
            NvsManager::instance().factoryReset();
            delay(200);
            ESP.restart();
            return true;  // unreachable, but keeps the compiler happy
        }

        // Log progress each second
        static uint32_t last_sec = 0;
        uint32_t sec = elapsed / 1000;
        if (sec > last_sec) {
            last_sec = sec;
            Serial.printf("[RST] %lu/%lums\n", elapsed, hold_ms_);
        }

        delay(10);
    }

    Serial.println("[RST] Released early, continuing boot");
    return false;
}

void ResetButton::check() {
    if (!enabled_) return;

    bool low = (digitalRead(pin_) == LOW);

    if (low && !pressed_) {
        pressed_ = true;
        press_start_ = millis();
    } else if (!low) {
        pressed_ = false;
        return;
    }

    if (!pressed_) return;

    uint32_t held = millis() - press_start_;
    if (held >= hold_ms_) {
        Serial.println("[RST] Factory reset triggered");
        NvsManager::instance().factoryReset();
        delay(200);
        ESP.restart();
    }
}