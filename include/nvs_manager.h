#pragma once

// =============================================================================
// NVS Manager — Persistent key-value storage with factory reset support
//
// All modules that need non-volatile storage go through this class.
// Factory reset (triggered by long button hold) wipes ALL namespaces.
//
// Usage:
//   NvsManager& nvs = NvsManager::instance();
//   nvs.putString("wifi", "ssid", "MyNetwork");
//   String ssid = nvs.getString("wifi", "ssid");
// =============================================================================

#include <Arduino.h>
#include <Preferences.h>

class NvsManager {
public:
    static NvsManager& instance();

    // Read/write for any namespace + key
    String  getString(const char* ns, const char* key, const String& defaultVal = "");
    bool    putString(const char* ns, const char* key, const String& value);
    int32_t getInt(const char* ns, const char* key, int32_t defaultVal = 0);
    bool    putInt(const char* ns, const char* key, int32_t value);
    bool    getBool(const char* ns, const char* key, bool defaultVal = false);
    bool    putBool(const char* ns, const char* key, bool value);

    // Remove a single key from a namespace
    bool    remove(const char* ns, const char* key);

    // Clear a single namespace
    bool    clearNamespace(const char* ns);

    // Factory reset — wipes all known namespaces
    void    factoryReset();

    // Register a namespace so factoryReset() knows about it.
    // Call this at init time from each module that uses NVS.
    void    registerNamespace(const char* ns);

private:
    NvsManager() : ns_count(0) {}
    NvsManager(const NvsManager&) = delete;
    NvsManager& operator=(const NvsManager&) = delete;

    static constexpr uint8_t MAX_NAMESPACES = 16;
    const char* namespaces[MAX_NAMESPACES];
    uint8_t ns_count;
};