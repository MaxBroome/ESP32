// ScoreScrape ESP32-P4
// Board: Waveshare ESP32-P4-WIFI6-POE-ETH (WiFi via SDIO ESP32-C6)
// Display: 1.47" 172x320 LCD (JD9853 + AXS5106L), landscape

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "board_config.h"
#include "display.h"
#include "touch_handler.h"
#include "app_state.h"
#include "nvs_manager.h"
#include "reset_button.h"
#include "wifi_manager.h"
#include "hosted_updater.h"
#include "provision_code.h"
#include "mqtt/provision.h"

extern "C" {
    #include "esp32-hal-hosted.h"
}

Display       display;
TouchHandler  touch;
AppState      appState;
ResetButton   resetBtn;
WiFiManager   wifiMgr;
MqttProvision mqttProvision;

static char last_onboarding_status[48] = {0};

// Start the MQTT provisioning flow and switch to the onboarding screen.
static void startOnboarding() {
    String code = ProvisionCode::getOrCreate();
    display.showOnboardingScreen(code.c_str());
    mqttProvision.begin(code.c_str());
    last_onboarding_status[0] = '\0';
    display.updateOnboardingStatus(mqttProvision.getStatusMessage());
    strncpy(last_onboarding_status, mqttProvision.getStatusMessage(),
            sizeof(last_onboarding_status) - 1);
    last_onboarding_status[sizeof(last_onboarding_status) - 1] = '\0';
    appState.setScreen(AppScreen::ONBOARDING);
}

// Wait for the ESP-Hosted SDIO link to the C6 coprocessor to come up.
// The C6 needs time to boot after the P4 resets it via the RESET pin.
static void waitForHostedLink(uint32_t timeout_ms = 8000) {
    uint32_t start = millis();
    while (!hostedIsInitialized() && (millis() - start) < timeout_ms) {
        delay(100);
    }
    if (hostedIsInitialized()) {
        uint32_t hMaj, hMin, hPat;
        hostedGetHostVersion(&hMaj, &hMin, &hPat);
        Serial.printf("[init] radio: esp-hosted %lu.%lu.%lu\n", hMaj, hMin, hPat);
    } else {
        Serial.println("[init] radio: FAILED — c6 not responding");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("\nScoreScrape v%s\n\n", FIRMWARE_VERSION);

    display.begin();
    display.showBootScreen("Starting...");

    // Register all NVS namespaces BEFORE the reset button check so that
    // factoryReset() actually knows what to wipe. Modules normally register
    // in their begin(), but that runs after the reset window.
    NvsManager::instance().registerNamespace("wifi");
    NvsManager::instance().registerNamespace("device");

    // Factory reset: hold BOOT button at power-on for 5 seconds.
    // This must happen before wifiMgr.begin() because ETH.begin() takes
    // over GPIO35 (shared with the RMII bus) and makes the pin unreadable.
    resetBtn.begin(PIN_RESET_BTN, 5000);
    resetBtn.waitForHold();
    resetBtn.disable();

    if (touch.begin()) Serial.println("[init] touch: ok");
    else               Serial.println("[init] touch: FAILED");

    // Mount LittleFS early so the C6 updater can access the firmware file.
    if (!LittleFS.begin(true)) {
        Serial.println("[init] fs: format + mount");
        LittleFS.format();
        LittleFS.begin();
    }

    display.showBootScreen("Starting radio...");
    // Initialize the ESP-Hosted SDIO link to the C6 coprocessor.
    // We go straight to AP_STA mode and NEVER change it again — the hosted
    // link does not survive WiFi.mode() transitions on the ESP32-P4.
    WiFi.mode(WIFI_AP_STA);
    waitForHostedLink();

    display.showBootScreen("Verifying radio firmware...");
    // If the hosted link is up, check if the C6 firmware needs updating.
    // This will reboot if an update is applied.
    HostedUpdater::updateIfNeeded();

    wifiMgr.begin([](const char* msg) { display.showBootScreen(msg); });

    // Already provisioned from a previous boot — go straight to home.
    if (wifiMgr.isConnected() && MqttProvision::hasBridgeId()) {
        Serial.printf("[init] bridge: %s (cached)\n", MqttProvision::getBridgeId().c_str());
        display.showHomeScreen();
        appState.setScreen(AppScreen::HOME);
    } else if (wifiMgr.isConnected()) {
        startOnboarding();
    } else {
        display.showConnectToNetworkScreen(wifiMgr.getPortalSSID());
        appState.setScreen(AppScreen::CONNECT_NETWORK);
    }
    Serial.println("[init] ready\n");
}

void loop() {
    if (wifiMgr.isPortalActive())
        wifiMgr.handlePortal();

    // When transitioning from portal to connected, show onboarding and start MQTT
    if (wifiMgr.isConnected() && appState.getScreen() == AppScreen::CONNECT_NETWORK) {
        startOnboarding();
    }

    if (appState.getScreen() == AppScreen::ONBOARDING) {
        mqttProvision.loop();
        const char* status = mqttProvision.getStatusMessage();
        if (status[0] && strcmp(status, last_onboarding_status) != 0) {
            display.updateOnboardingStatus(status);
            strncpy(last_onboarding_status, status, sizeof(last_onboarding_status) - 1);
            last_onboarding_status[sizeof(last_onboarding_status) - 1] = '\0';
        }
        if (mqttProvision.isProvisioned()) {
            mqttProvision.stop();
            display.showHomeScreen();
            appState.setScreen(AppScreen::HOME);
        }
    }

    if (appState.shouldRevertToHome()) {
        display.showHomeScreen();
        appState.setScreen(AppScreen::HOME);
    }

    if (appState.getScreen() == AppScreen::HOME) {
        GestureType gesture = touch.detectGesture();

        if (gesture == GestureType::SWIPE_RIGHT_TO_LEFT) {
            display.showSwipedMessage();
            appState.markTapped();
        }
        else if (gesture == GestureType::TAP) {
            TouchData td;
            touch.getTouchData(td);
            display.showTappedMessage();
            appState.markTapped();
        }
    }

    delay(10);
}
