#pragma once

// Connectivity manager: Ethernet + WiFi (WPA2-Personal & Enterprise) + captive portal.
//
// On boot, tries connections in order of reliability:
//   1. Wired ethernet (IP101GRI over RMII, always-on if cable present)
//   2. Saved WiFi credentials from NVS
//   3. SoftAP captive portal for user provisioning
//
// The SoftAP is only torn down after verifying actual internet reachability
// on whichever interface comes up first. If ethernet gets plugged in while
// the portal is running, it auto-detects and shuts down the AP.

#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ETH.h>

enum class ConnType  { NONE, WIFI, ETHERNET };
enum class WiFiState { IDLE, CONNECTING, CONNECTED, PORTAL_ACTIVE };

class WiFiManager {
public:
    void begin();
    void handlePortal();

    WiFiState getState()  const { return state_; }
    ConnType  connType()  const { return conn_type_; }
    bool      isConnected() const;
    String    getIP() const;
    bool      isPortalActive() const { return state_ == WiFiState::PORTAL_ACTIVE; }

private:
    WiFiState state_     = WiFiState::IDLE;
    ConnType  conn_type_ = ConnType::NONE;

    WebServer* server_ = nullptr;
    DNSServer* dns_    = nullptr;
    String portal_html_;

    // NVS-backed credentials
    String saved_ssid_;
    String saved_pass_;
    String saved_user_;
    bool   saved_enterprise_ = false;

    // Set from the ethernet event callback (runs on a different FreeRTOS task)
    static volatile bool eth_link_up_;
    static volatile bool eth_got_ip_;

    void loadCredentials();
    void saveCredentials(const String& ssid, const String& pass,
                         const String& user, bool enterprise);

    // Return empty String on success, or a user-facing error message on failure.
    String connectWPA(const String& ssid, const String& pass, uint32_t timeout_ms = 15000);
    String connectEnterprise(const String& ssid, const String& user,
                             const String& pass, uint32_t timeout_ms = 20000);
    bool checkInternet();
    bool checkEthernetInternet();

    void initEthernet();
    static void ethEventHandler(arduino_event_id_t event);

    void startPortal();
    void stopPortal();

    void serveRoot();
    void serveFile(const char* path, const char* mime);
    void serveScan();
    void serveConnect();
    void serveRedirect();

    static String extractJsonValue(const String& json, const char* key);
};