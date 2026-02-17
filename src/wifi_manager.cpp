#include "wifi_manager.h"
#include "nvs_manager.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <HTTPClient.h>
#include <esp_eap_client.h>
#include <esp_wifi.h>

// NVS
static const char* NVS_NS = "wifi";

// SoftAP (single source of truth — display uses getPortalSSID())
static const char AP_SSID[] = "ScoreScrape-Setup";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_GW(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

// ScoreScrape API ping — returns 200 with "Pong!" body.
static const char* CAPTIVE_CHECK_URL  = "https://api.scorescrape.io/ping";
static const char* CAPTIVE_CHECK_RESPONSE = "Pong!";

// IP101GRI PHY on the ESP32-P4-WIFI6-POE-ETH (RMII, fixed pinout)
#ifndef ETH_PHY_TYPE
#define ETH_PHY_TYPE  ETH_PHY_IP101
#define ETH_PHY_ADDR  1
#define ETH_PHY_MDC   31
#define ETH_PHY_MDIO  52
#define ETH_PHY_POWER 51
#define ETH_CLK_MODE  EMAC_CLK_EXT_IN
#endif

volatile bool WiFiManager::eth_link_up_ = false;
volatile bool WiFiManager::eth_got_ip_  = false;


void WiFiManager::begin(void (*statusCallback)(const char*)) {
    NvsManager::instance().registerNamespace(NVS_NS);

    // LittleFS is already mounted by main.cpp (needed for C6 FW updater).
    File f = LittleFS.open("/index.html", "r");
    if (f) {
        portal_html_ = f.readString();
        f.close();
    } else {
        Serial.println("[NET] /index.html missing (run: pio run -t uploadfs)");
        portal_html_ = "<html><body><h2>Filesystem not flashed</h2>"
                        "<p>Run <code>pio run -t uploadfs</code></p></body></html>";
    }

    loadCredentials();
    initEthernet();

    // Let ethernet negotiate link + DHCP before we decide what to do.
    delay(2000);

    if (eth_got_ip_ && checkEthernetInternet()) {
        Serial.printf("[NET] Ethernet online (%s)\n", ETH.localIP().toString().c_str());
        state_     = WiFiState::CONNECTED;
        conn_type_ = ConnType::ETHERNET;
        hideAP();
        return;
    }

    // -----------------------------------------------------------------------
    // IMPORTANT: The ESP-Hosted SDIO link to the C6 coprocessor is fragile.
    // Calling WiFi.mode() tears down and re-initializes the hosted transport,
    // which can kill the link entirely. main.cpp sets WIFI_AP_STA once at
    // boot and we NEVER call WiFi.mode() again. All connection changes use
    // only WiFi.disconnect() / WiFi.begin() / WiFi.softAP() / softAPdisconnect().
    // -----------------------------------------------------------------------

    if (saved_ssid_.length() > 0) {
        Serial.printf("[NET] Trying saved creds: %s\n", saved_ssid_.c_str());
        state_ = WiFiState::CONNECTING;

        if (statusCallback) {
            String msg = "Attempting connection to \"" + saved_ssid_ + "\"";
            statusCallback(msg.c_str());
        }

        String err = saved_enterprise_
            ? connectEnterprise(saved_ssid_, saved_user_, saved_pass_)
            : connectWPA(saved_ssid_, saved_pass_);

        if (err.isEmpty()) {
            state_     = WiFiState::CONNECTED;
            conn_type_ = ConnType::WIFI;
            hideAP();
            Serial.printf("[NET] WiFi online (%s)\n", WiFi.localIP().toString().c_str());
            return;
        }
        Serial.println("[NET] Saved creds failed");
    }

    startPortal();
}


void WiFiManager::handlePortal() {
    if (server_) server_->handleClient();
    if (dns_)    dns_->processNextRequest();

    checkPendingConnection();

    // Deferred portal teardown — gives the phone time to poll /api/status
    // and render the success screen before the SoftAP disappears.
    if (portal_stop_at_ > 0 && millis() >= portal_stop_at_) {
        portal_stop_at_ = 0;
        stopPortal();
    }

    // Auto-detect ethernet coming online while portal is up
    static uint32_t next_eth_poll = 0;
    uint32_t now = millis();
    if (eth_got_ip_ && now >= next_eth_poll) {
        next_eth_poll = now + 5000;
        if (checkEthernetInternet()) {
            Serial.println("[NET] Ethernet detected, closing portal");
            state_     = WiFiState::CONNECTED;
            conn_type_ = ConnType::ETHERNET;
            stopPortal();
        }
    }
}

const char* WiFiManager::getPortalSSID() const {
    return AP_SSID;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED || eth_got_ip_;
}

String WiFiManager::getIP() const {
    if (conn_type_ == ConnType::ETHERNET)
        return ETH.localIP().toString();
    return WiFi.localIP().toString();
}


// -- Ethernet ---------------------------------------------------------------

void WiFiManager::initEthernet() {
    Network.onEvent(ethEventHandler);
    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO,
              ETH_PHY_POWER, ETH_CLK_MODE);
}

void WiFiManager::ethEventHandler(arduino_event_id_t event) {
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        ETH.setHostname("scorescrape");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        eth_link_up_ = true;
        Serial.println("[ETH] Link up");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        eth_got_ip_ = true;
        Serial.printf("[ETH] %s\n", ETH.localIP().toString().c_str());
        break;
    case ARDUINO_EVENT_ETH_LOST_IP:
        eth_got_ip_ = false;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        eth_link_up_ = false;
        eth_got_ip_  = false;
        Serial.println("[ETH] Link down");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        eth_link_up_ = false;
        eth_got_ip_  = false;
        break;
    default:
        break;
    }
}

bool WiFiManager::checkEthernetInternet() {
    if (!eth_got_ip_) return false;
    HTTPClient http;
    http.begin(CAPTIVE_CHECK_URL);
    http.setTimeout(4000);
    int code = http.GET();

    if (code == 200) {
        String response = http.getString();
        http.end();
        response.trim();
        return response == CAPTIVE_CHECK_RESPONSE;
    }

    http.end();
    return false;
}


// -- NVS credentials --------------------------------------------------------

void WiFiManager::loadCredentials() {
    auto& nvs = NvsManager::instance();
    saved_ssid_       = nvs.getString(NVS_NS, "ssid");
    saved_pass_       = nvs.getString(NVS_NS, "pass");
    saved_user_       = nvs.getString(NVS_NS, "user");
    saved_enterprise_ = nvs.getBool(NVS_NS, "enterprise", false);
}

void WiFiManager::saveCredentials(const String& ssid, const String& pass,
                                  const String& user, bool enterprise) {
    auto& nvs = NvsManager::instance();
    nvs.putString(NVS_NS, "ssid", ssid);
    nvs.putString(NVS_NS, "pass", pass);
    nvs.putString(NVS_NS, "user", user);
    nvs.putBool(NVS_NS, "enterprise", enterprise);
    saved_ssid_       = ssid;
    saved_pass_       = pass;
    saved_user_       = user;
    saved_enterprise_ = enterprise;
}


// -- WiFi connection --------------------------------------------------------
// CRITICAL: Never call WiFi.mode() in these functions. The ESP-Hosted SDIO
// link to the C6 coprocessor does not survive mode transitions. We stay in
// WIFI_AP_STA permanently and only use disconnect/begin/softAP.

static bool waitForAssociation(uint32_t timeout_ms) {
    uint32_t deadline = millis() + timeout_ms;
    while (WiFi.status() != WL_CONNECTED && millis() < deadline)
        delay(250);
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::connectWPA(const String& ssid, const String& pass,
                               uint32_t timeout_ms) {
    // No WiFi.mode() call — we're already in AP_STA.
    // Use disconnect(false) to avoid eraseAP RPC which can disrupt the hosted link.
    WiFi.disconnect(false);
    delay(100);

    Serial.printf("[NET] WiFi.begin('%s')...\n", ssid.c_str());
    if (pass.length() > 0)
        WiFi.begin(ssid.c_str(), pass.c_str());
    else
        WiFi.begin(ssid.c_str());

    if (!waitForAssociation(timeout_ms)) {
        Serial.printf("[NET] WPA association to '%s' timed out (status=%d)\n",
                      ssid.c_str(), WiFi.status());
        WiFi.disconnect(false);
        if (pass.length() > 0)
            return "Could not connect. Check that the password is correct.";
        return "Could not connect to the open network.";
    }

    if (!checkInternet()) {
        Serial.println("[NET] WPA connected but no internet (captive portal or restricted network)");
        WiFi.disconnect(false);
        return "Connected to WiFi, but the network has no internet access. "
               "It may require a captive portal login that this device cannot complete.";
    }

    return "";
}

String WiFiManager::connectEnterprise(const String& ssid, const String& user,
                                      const String& pass, uint32_t timeout_ms) {
    // No WiFi.mode() call — we're already in AP_STA.
    WiFi.disconnect(false);
    delay(100);

    // PEAP/MSCHAPv2 — identity and username are the same for most
    // corporate and eduroam deployments (email-style login).
    //
    // NOTE: The standard esp_eap_client_* functions are patched by
    // esp_hosted_eap_fix.c to forward RPCs to the C6 coprocessor.
    // Without that fix, they are 4-byte no-op stubs in the hosted library.
    esp_eap_client_set_identity((uint8_t*)user.c_str(), user.length());
    esp_eap_client_set_username((uint8_t*)user.c_str(), user.length());
    esp_eap_client_set_password((uint8_t*)pass.c_str(), pass.length());
    esp_eap_client_set_disable_time_check(true);

    esp_err_t err = esp_wifi_sta_enterprise_enable();
    if (err != ESP_OK) {
        Serial.printf("[NET] Enterprise enable failed: 0x%x\n", err);
        WiFi.disconnect(false);
        return "Enterprise (802.1X) WiFi is not supported by the current "
               "coprocessor firmware. A firmware update may be required.";
    }

    Serial.printf("[NET] WiFi.begin('%s') [enterprise PEAP]...\n", ssid.c_str());
    WiFi.begin(ssid.c_str());

    if (!waitForAssociation(timeout_ms)) {
        Serial.printf("[NET] Enterprise association to '%s' timed out (status=%d)\n",
                      ssid.c_str(), WiFi.status());
        esp_wifi_sta_enterprise_disable();
        WiFi.disconnect(false);
        return "Enterprise authentication failed. Check your username and password.";
    }

    if (!checkInternet()) {
        Serial.println("[NET] Enterprise connected but no internet (captive portal or restricted network)");
        esp_wifi_sta_enterprise_disable();
        WiFi.disconnect(false);
        return "Connected to WiFi, but the network has no internet access.";
    }

    return "";
}

bool WiFiManager::checkInternet() {
    HTTPClient http;
    http.begin(CAPTIVE_CHECK_URL);
    http.setTimeout(5000);
    int code = http.GET();

    if (code == 200) {
        String response = http.getString();
        http.end();
        response.trim();

        if (response == CAPTIVE_CHECK_RESPONSE) {
            Serial.println("[NET] Internet connectivity verified via ScoreScrape API");
            return true;
        } else {
            Serial.printf("[NET] Unexpected response from ping endpoint: '%s'\n", response.c_str());
            return false;
        }
    }

    http.end();
    Serial.printf("[NET] Ping endpoint returned code %d (expected 200)\n", code);
    return false;
}


// -- Captive portal ---------------------------------------------------------

void WiFiManager::startPortal() {
    Serial.println("[NET] Starting captive portal");

    // We're already in WIFI_AP_STA mode — just bring up the SoftAP.
    // No WiFi.mode() calls here to avoid killing the hosted link.
    WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
    delay(100);
    WiFi.softAP(AP_SSID);

    dns_ = new DNSServer();
    dns_->start(53, "*", AP_IP);

    server_ = new WebServer(80);
    server_->on("/",            HTTP_GET,  [this]() { serveRoot(); });
    server_->on("/index.html",  HTTP_GET,  [this]() { serveRoot(); });
    server_->on("/style.css",   HTTP_GET,  [this]() { serveFile("/style.css", "text/css"); });
    server_->on("/script.js",   HTTP_GET,  [this]() { serveFile("/script.js", "application/javascript"); });
    server_->on("/api/scan",    HTTP_GET,  [this]() { serveScan(); });
    server_->on("/api/connect", HTTP_POST, [this]() { serveConnect(); });
    server_->on("/api/status",  HTTP_GET,  [this]() { serveStatus(); });
    server_->onNotFound(                   [this]() { serveRedirect(); });
    server_->begin();

    state_ = WiFiState::PORTAL_ACTIVE;
    Serial.printf("[NET] AP \"%s\" on %s\n", AP_SSID,
                  WiFi.softAPIP().toString().c_str());
}

void WiFiManager::stopPortal() {
    if (server_) { server_->stop(); delete server_; server_ = nullptr; }
    if (dns_)    { dns_->stop();    delete dns_;    dns_    = nullptr; }
    hideAP();
    Serial.println("[NET] Portal stopped");
}

void WiFiManager::hideAP() {
    // Silence the SoftAP that the C6 broadcasts in AP_STA mode.
    // softAPdisconnect(true) zeros the SSID/password via esp_wifi_set_config,
    // then we belt-and-suspenders set ssid_hidden=1 via the low-level API
    // to ensure the C6 stops including the SSID in beacon frames.
    // We stay in AP_STA mode — do NOT call WiFi.mode() here.
    WiFi.softAPdisconnect(true);
    delay(100);
    wifi_config_t conf;
    if (esp_wifi_get_config(WIFI_IF_AP, &conf) == ESP_OK) {
        conf.ap.ssid_hidden = 1;
        conf.ap.max_connection = 0;
        esp_wifi_set_config(WIFI_IF_AP, &conf);
    }
}


// -- Pending connection (non-blocking) --------------------------------------
// serveConnect() kicks off WiFi.begin() and returns immediately so the phone
// gets an HTTP response right away. checkPendingConnection() runs each loop
// iteration to monitor WiFi.status() and do the internet check once associated.

void WiFiManager::checkPendingConnection() {
    if (!pending_connect_) return;

    if (WiFi.status() == WL_CONNECTED) {
        // Associated — now verify internet (blocks ~5s max, acceptable).
        if (checkInternet()) {
            saveCredentials(pending_ssid_, pending_pass_,
                            pending_user_, pending_enterprise_);
            conn_type_       = ConnType::WIFI;
            pending_connect_ = false;
            pending_result_  = "connected";
            pending_ip_      = WiFi.localIP().toString();
            Serial.printf("[NET] WiFi online (%s)\n", pending_ip_.c_str());
            return;
        }
        // Internet check failed
        Serial.println("[NET] Associated but no internet");
        if (pending_enterprise_) esp_wifi_sta_enterprise_disable();
        WiFi.disconnect(false);
        pending_connect_ = false;
        pending_result_  = "failed";
        pending_error_   = "Connected to WiFi, but the network has no internet access.";
        return;
    }

    if (millis() > pending_deadline_) {
        Serial.printf("[NET] Association timed out (status=%d)\n", WiFi.status());
        if (pending_enterprise_) esp_wifi_sta_enterprise_disable();
        WiFi.disconnect(false);
        pending_connect_ = false;
        pending_result_  = "failed";
        pending_error_   = pending_enterprise_
            ? "Enterprise authentication failed. Check your username and password."
            : "Could not connect. Check the password and try again.";
    }
}


// -- HTTP handlers ----------------------------------------------------------

void WiFiManager::serveRoot() {
    server_->send(200, "text/html", portal_html_);
}

void WiFiManager::serveFile(const char* path, const char* mime) {
    File f = LittleFS.open(path, "r");
    if (!f) { server_->send(404, "text/plain", "Not found"); return; }
    server_->streamFile(f, mime);
    f.close();
}

void WiFiManager::serveScan() {
    int n = WiFi.scanNetworks();
    String json;
    json.reserve(n * 80);
    json = "[";

    for (int i = 0; i < n; i++) {
        if (i) json += ',';
        wifi_auth_mode_t auth = WiFi.encryptionType(i);
        bool open = (auth == WIFI_AUTH_OPEN);
        bool ent  = (auth == WIFI_AUTH_WPA2_ENTERPRISE ||
                     auth == WIFI_AUTH_WPA3_ENTERPRISE);

        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\""
                ",\"rssi\":" + String(WiFi.RSSI(i)) +
                ",\"ch\":"   + String(WiFi.channel(i)) +
                ",\"open\":" + (open ? "true" : "false") +
                ",\"enterprise\":" + (ent ? "true" : "false") + "}";
    }
    json += "]";
    WiFi.scanDelete();
    server_->send(200, "application/json", json);
}

void WiFiManager::serveConnect() {
    if (!server_->hasArg("plain")) {
        server_->send(400, "application/json", "{\"ok\":false,\"msg\":\"No body\"}");
        return;
    }

    String body = server_->arg("plain");
    String ssid = extractJsonValue(body, "ssid");
    String pass = extractJsonValue(body, "pass");
    String user = extractJsonValue(body, "user");
    bool   ent  = extractJsonValue(body, "enterprise") == "true";

    if (ssid.isEmpty()) {
        server_->send(400, "application/json", "{\"ok\":false,\"msg\":\"Missing SSID\"}");
        return;
    }
    if (ent && user.isEmpty()) {
        server_->send(400, "application/json",
                      "{\"ok\":false,\"msg\":\"Username required for enterprise\"}");
        return;
    }

    // Set up enterprise EAP params if needed (these are non-blocking RPCs).
    if (ent) {
        esp_eap_client_set_identity((uint8_t*)user.c_str(), user.length());
        esp_eap_client_set_username((uint8_t*)user.c_str(), user.length());
        esp_eap_client_set_password((uint8_t*)pass.c_str(), pass.length());
        esp_eap_client_set_disable_time_check(true);

        esp_err_t err = esp_wifi_sta_enterprise_enable();
        if (err != ESP_OK) {
            Serial.printf("[NET] Enterprise enable failed: 0x%x\n", err);
            server_->send(200, "application/json",
                          "{\"ok\":false,\"msg\":\"Enterprise WiFi not supported by "
                          "current coprocessor firmware.\"}");
            return;
        }
    }

    // Kick off WiFi.begin() — non-blocking, returns immediately.
    WiFi.disconnect(false);
    delay(100);

    Serial.printf("[NET] WiFi.begin('%s')%s...\n", ssid.c_str(),
                  ent ? " [enterprise PEAP]" : "");

    if (ent || pass.isEmpty())
        WiFi.begin(ssid.c_str());
    else
        WiFi.begin(ssid.c_str(), pass.c_str());

    // Store pending state — checkPendingConnection() monitors from the main loop.
    pending_connect_    = true;
    pending_ssid_       = ssid;
    pending_pass_       = pass;
    pending_user_       = user;
    pending_enterprise_ = ent;
    pending_deadline_   = millis() + (ent ? 20000 : 15000);
    pending_result_     = "";
    pending_ip_         = "";
    pending_error_      = "";

    // Respond immediately so the phone's captive portal webview doesn't time out.
    server_->send(200, "application/json", "{\"status\":\"connecting\"}");
}

void WiFiManager::serveStatus() {
    if (pending_result_ == "connected") {
        server_->send(200, "application/json",
                      "{\"status\":\"connected\",\"ip\":\"" + pending_ip_ + "\"}");
        // Schedule portal teardown — give the phone time to render the success screen.
        state_ = WiFiState::CONNECTED;
        if (portal_stop_at_ == 0)
            portal_stop_at_ = millis() + 5000;
        return;
    }

    if (pending_result_ == "failed") {
        String err = pending_error_;
        err.replace("\"", "\\\"");
        pending_result_ = "";
        server_->send(200, "application/json",
                      "{\"status\":\"failed\",\"msg\":\"" + err + "\"}");
        return;
    }

    if (pending_connect_) {
        server_->send(200, "application/json", "{\"status\":\"connecting\"}");
        return;
    }

    server_->send(200, "application/json", "{\"status\":\"idle\"}");
}

void WiFiManager::serveRedirect() {
    server_->sendHeader("Location", "http://192.168.4.1/", true);
    server_->send(302, "text/plain", "");
}


// -- JSON helper ------------------------------------------------------------

String WiFiManager::extractJsonValue(const String& json, const char* key) {
    String needle = String("\"") + key + "\"";
    int ki = json.indexOf(needle);
    if (ki < 0) return "";

    int ci = json.indexOf(':', ki + needle.length());
    if (ci < 0) return "";

    int vi = ci + 1;
    while (vi < (int)json.length() && json[vi] == ' ') vi++;
    if (vi >= (int)json.length()) return "";

    if (json[vi] == '"') {
        int end = json.indexOf('"', vi + 1);
        return (end > vi) ? json.substring(vi + 1, end) : "";
    }

    // Bare value (bool, number)
    int end = vi;
    while (end < (int)json.length() && json[end] != ',' && json[end] != '}')
        end++;
    return json.substring(vi, end);
}
