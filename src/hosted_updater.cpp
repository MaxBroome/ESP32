#include "hosted_updater.h"
#include <LittleFS.h>
#include <Preferences.h>

// The Arduino ESP32 core exposes these C functions for hosted coprocessor management.
// They're conditionally compiled when CONFIG_ESP_WIFI_REMOTE_ENABLED is set (which it is
// for ESP32-P4 builds with the esp_hosted component).
extern "C" {
    #include "esp32-hal-hosted.h"
}

static const char* FW_PATH = "/c6_fw.bin";
static const size_t CHUNK_SIZE = 1400;
static const char* NVS_NS = "c6ota";
static const char* NVS_KEY = "fwsize";

bool HostedUpdater::updateIfNeeded() {
    if (!hostedIsInitialized()) {
        Serial.println("[init] c6: skipped (no radio)");
        return false;
    }

    if (!LittleFS.exists(FW_PATH)) {
        Serial.println("[init] c6: no firmware on fs");
        return false;
    }

    bool versionMismatch = hostedHasUpdate();

    // Also check if the firmware binary on LittleFS changed since last flash.
    // This catches rebuilds with the same version but different content (e.g.
    // enabling CONFIG_ESP_WIFI_ENTERPRISE_SUPPORT in a custom C6 build).
    File fw = LittleFS.open(FW_PATH, "r");
    size_t fileSize = fw ? fw.size() : 0;
    if (fw) fw.close();

    Preferences prefs;
    prefs.begin(NVS_NS, true);
    size_t lastFlashedSize = prefs.getUInt(NVS_KEY, 0);
    prefs.end();

    bool binaryChanged = (fileSize > 0 && fileSize != lastFlashedSize);

    if (!versionMismatch && !binaryChanged) {
        Serial.println("[init] c6: firmware current");
        return false;
    }

    uint32_t hMaj, hMin, hPat, sMaj, sMin, sPat;
    hostedGetHostVersion(&hMaj, &hMin, &hPat);
    hostedGetSlaveVersion(&sMaj, &sMin, &sPat);
    Serial.printf("[ota]  c6: host=%lu.%lu.%lu slave=%lu.%lu.%lu%s\n",
                  hMaj, hMin, hPat, sMaj, sMin, sPat,
                  binaryChanged ? " (binary changed)" : "");
    Serial.println("[ota]  c6: updating...");

    if (flashFromFile(FW_PATH)) {
        Preferences p;
        p.begin(NVS_NS, false);
        p.putUInt(NVS_KEY, fileSize);
        p.end();

        Serial.println("[ota]  c6: complete, rebooting");
        delay(500);
        ESP.restart();
        return true;
    }

    Serial.println("[ota]  c6: FAILED");
    return false;
}

bool HostedUpdater::flashFromFile(const char* path) {
    File f = LittleFS.open(path, "r");
    if (!f) {
        Serial.println("[ota]  c6: cannot open file");
        return false;
    }

    size_t total = f.size();
    Serial.printf("[ota]  c6: %u bytes\n", total);

    if (!hostedBeginUpdate()) {
        Serial.println("[ota]  c6: begin failed");
        f.close();
        return false;
    }

    uint8_t buf[CHUNK_SIZE];
    size_t written = 0;
    int lastPct = -1;

    while (written < total) {
        size_t toRead = min(CHUNK_SIZE, total - written);
        size_t got = f.read(buf, toRead);
        if (got == 0) {
            Serial.println("[ota]  c6: read error");
            f.close();
            return false;
        }

        if (!hostedWriteUpdate(buf, got)) {
            Serial.printf("[ota]  c6: write failed @ %u\n", written);
            f.close();
            return false;
        }

        written += got;
        int pct = (written * 100) / total;
        if (pct / 10 != lastPct / 10) {
            Serial.printf("[ota]  c6: %d%%\n", pct);
            lastPct = pct;
        }
    }

    f.close();

    if (!hostedEndUpdate()) {
        Serial.println("[ota]  c6: verify failed");
        return false;
    }

    if (!hostedActivateUpdate()) {
        // Not fatal — older firmwares don't support activate
    }

    return true;
}
