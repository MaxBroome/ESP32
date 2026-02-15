#include "nvs_manager.h"
#include <nvs_flash.h>

NvsManager& NvsManager::instance() {
    static NvsManager inst;
    static bool inited = false;
    if (!inited) {
        inited = true;
        // Ensure NVS flash is initialized. On a fresh device the partition
        // may be empty, causing Preferences::begin() to log errors.
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            nvs_flash_init();
        }
    }
    return inst;
}

void NvsManager::registerNamespace(const char* ns) {
    // Avoid duplicates
    for (uint8_t i = 0; i < ns_count; i++) {
        if (strcmp(namespaces[i], ns) == 0) return;
    }
    if (ns_count < MAX_NAMESPACES) {
        namespaces[ns_count++] = ns;
        // Touch the namespace in RW mode so it exists before any read-only
        // access. Prevents Preferences "NOT_FOUND" log spam on first boot.
        Preferences prefs;
        prefs.begin(ns, false);
        prefs.end();
    }
}

// --- String ---
String NvsManager::getString(const char* ns, const char* key, const String& defaultVal) {
    Preferences prefs;
    prefs.begin(ns, true);  // read-only
    String val = prefs.isKey(key) ? prefs.getString(key, defaultVal) : defaultVal;
    prefs.end();
    return val;
}

bool NvsManager::putString(const char* ns, const char* key, const String& value) {
    Preferences prefs;
    prefs.begin(ns, false);
    bool ok = prefs.putString(key, value) > 0;
    prefs.end();
    return ok;
}

// --- Int ---
int32_t NvsManager::getInt(const char* ns, const char* key, int32_t defaultVal) {
    Preferences prefs;
    prefs.begin(ns, true);
    int32_t val = prefs.isKey(key) ? prefs.getInt(key, defaultVal) : defaultVal;
    prefs.end();
    return val;
}

bool NvsManager::putInt(const char* ns, const char* key, int32_t value) {
    Preferences prefs;
    prefs.begin(ns, false);
    bool ok = prefs.putInt(key, value) > 0;
    prefs.end();
    return ok;
}

// --- Bool ---
bool NvsManager::getBool(const char* ns, const char* key, bool defaultVal) {
    Preferences prefs;
    prefs.begin(ns, true);
    bool val = prefs.isKey(key) ? prefs.getBool(key, defaultVal) : defaultVal;
    prefs.end();
    return val;
}

bool NvsManager::putBool(const char* ns, const char* key, bool value) {
    Preferences prefs;
    prefs.begin(ns, false);
    bool ok = prefs.putBool(key, value) > 0;
    prefs.end();
    return ok;
}

// --- Remove / Clear ---
bool NvsManager::remove(const char* ns, const char* key) {
    Preferences prefs;
    prefs.begin(ns, false);
    bool ok = prefs.remove(key);
    prefs.end();
    return ok;
}

bool NvsManager::clearNamespace(const char* ns) {
    Preferences prefs;
    prefs.begin(ns, false);
    bool ok = prefs.clear();
    prefs.end();
    return ok;
}

void NvsManager::factoryReset() {
    Serial.println("[NVS] Factory reset — wiping all namespaces");
    for (uint8_t i = 0; i < ns_count; i++) {
        Serial.printf("[NVS]   Clearing: %s\n", namespaces[i]);
        clearNamespace(namespaces[i]);
    }
    Serial.println("[NVS] Factory reset complete");
}