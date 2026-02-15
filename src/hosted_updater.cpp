#include "hosted_updater.h"
#include <LittleFS.h>

// The Arduino ESP32 core exposes these C functions for hosted coprocessor management.
// They're conditionally compiled when CONFIG_ESP_WIFI_REMOTE_ENABLED is set (which it is
// for ESP32-P4 builds with the esp_hosted component).
extern "C" {
    #include "esp32-hal-hosted.h"
}

static const char* FW_PATH = "/c6_fw.bin";
static const size_t CHUNK_SIZE = 1400;  // recommended by esp_hosted OTA docs

bool HostedUpdater::updateIfNeeded() {
    if (!hostedIsInitialized()) {
        Serial.println("[C6] ESP-Hosted not initialized, skipping FW check");
        return false;
    }

    if (!LittleFS.exists(FW_PATH)) {
        Serial.println("[C6] No firmware file on LittleFS, skipping");
        return false;
    }

    if (!hostedHasUpdate()) {
        Serial.println("[C6] Coprocessor firmware is up to date");
        return false;
    }

    // Version mismatch — host is newer than slave
    uint32_t hMaj, hMin, hPat, sMaj, sMin, sPat;
    hostedGetHostVersion(&hMaj, &hMin, &hPat);
    hostedGetSlaveVersion(&sMaj, &sMin, &sPat);
    Serial.printf("[C6] Host expects %lu.%lu.%lu, slave has %lu.%lu.%lu\n",
                  hMaj, hMin, hPat, sMaj, sMin, sPat);
    Serial.println("[C6] Updating coprocessor firmware...");

    if (flashFromFile(FW_PATH)) {
        Serial.println("[C6] Update complete, rebooting...");
        delay(500);
        ESP.restart();
        return true;  // unreachable, but for clarity
    }

    Serial.println("[C6] Update FAILED");
    return false;
}

bool HostedUpdater::flashFromFile(const char* path) {
    File f = LittleFS.open(path, "r");
    if (!f) {
        Serial.println("[C6] Failed to open firmware file");
        return false;
    }

    size_t total = f.size();
    Serial.printf("[C6] Firmware size: %u bytes\n", total);

    if (!hostedBeginUpdate()) {
        Serial.println("[C6] OTA begin failed");
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
            Serial.println("[C6] Read error");
            f.close();
            return false;
        }

        if (!hostedWriteUpdate(buf, got)) {
            Serial.printf("[C6] Write failed at offset %u\n", written);
            f.close();
            return false;
        }

        written += got;
        int pct = (written * 100) / total;
        if (pct / 10 != lastPct / 10) {
            Serial.printf("[C6] %d%%\n", pct);
            lastPct = pct;
        }
    }

    f.close();

    if (!hostedEndUpdate()) {
        Serial.println("[C6] OTA end/verify failed");
        return false;
    }

    if (!hostedActivateUpdate()) {
        Serial.println("[C6] OTA activate failed (may be OK on older FW)");
        // Not fatal — older firmwares don't support activate
    }

    return true;
}