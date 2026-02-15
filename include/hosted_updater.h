#pragma once

// ESP-Hosted coprocessor (ESP32-C6) firmware updater.
// Reads the firmware binary from LittleFS and flashes it over SDIO.

#include <Arduino.h>

class HostedUpdater {
public:
    // Call after LittleFS is mounted and ESP-Hosted is initialized.
    // Returns true if the C6 was updated (caller should reboot).
    static bool updateIfNeeded();

private:
    static bool flashFromFile(const char* path);
};