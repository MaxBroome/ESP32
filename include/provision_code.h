#pragma once

#include <Arduino.h>

// =============================================================================
// ProvisionCode — 6-char device claim code (A-Z, 0-9)
//
// Generated on first boot, stored in NVS. Factory reset wipes it and a new
// code is generated on next boot.
//
// Register the "device" namespace before factory reset check in main.
// =============================================================================

class ProvisionCode {
public:
    // Get existing code from NVS, or generate and store a new one.
    // Returns 6-char string, always uppercase alphanumeric.
    static String getOrCreate();
};
