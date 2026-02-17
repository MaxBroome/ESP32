#include "mqtt/provision.h"
#include "nvs_manager.h"

static MqttProvision* s_provision = nullptr;

static const char* NVS_NS     = "device";
static const char* NVS_BRIDGE = "bridge_id";

static String extractJson(const char* json, const char* key) {
    String n = String("\"") + key + "\"";
    int ki = String(json).indexOf(n);
    if (ki < 0) return "";
    int ci = String(json).indexOf(':', ki + n.length());
    if (ci < 0) return "";
    int vi = ci + 1;
    String s = json;
    while (vi < (int)s.length() && s[vi] == ' ') vi++;
    if (vi >= (int)s.length()) return "";
    if (s[vi] == '"') {
        int end = s.indexOf('"', vi + 1);
        return (end > vi) ? s.substring(vi + 1, end) : "";
    }
    int end = vi;
    while (end < (int)s.length() && s[end] != ',' && s[end] != '}') end++;
    return s.substring(vi, end);
}

// -- Static NVS helpers -------------------------------------------------------

bool MqttProvision::hasBridgeId() {
    return getBridgeId().length() > 0;
}

String MqttProvision::getBridgeId() {
    return NvsManager::instance().getString(NVS_NS, NVS_BRIDGE);
}

// -- Callbacks ----------------------------------------------------------------

void MqttProvision::onMessage(const char* topic, const char* payload, unsigned int length) {
    if (s_provision) s_provision->handleMessage(topic, payload, length);
}

// -- Lifecycle ----------------------------------------------------------------

void MqttProvision::begin(const char* code) {
    code_ = code;
    topic_ = String("devices/claim/") + code_;
    client_id_ = String("provision-") + code_;
    state_ = MqttProvisionState::CONNECTING;
    error_msg_[0] = '\0';
    subscribed_ = published_ = false;
    s_provision = this;
    strncpy(status_msg_, "Connecting to MQTT broker", sizeof(status_msg_) - 1);
    status_msg_[sizeof(status_msg_) - 1] = '\0';
    client_.setCallback(onMessage);
    Serial.printf("[MQTT] Provision: %s -> %s\n", client_id_.c_str(), topic_.c_str());
}

void MqttProvision::stop() {
    if (client_.isConnected()) {
        client_.disconnect();
        Serial.println("[MQTT] Disconnected");
    }
    s_provision = nullptr;
    state_ = MqttProvisionState::IDLE;
}

void MqttProvision::loop() {
    if (state_ == MqttProvisionState::IDLE ||
        state_ == MqttProvisionState::PROVISIONED ||
        state_ == MqttProvisionState::ERROR) return;

    if (!client_.isConnected()) {
        // Connection lost — need to re-subscribe after reconnecting.
        subscribed_ = false;
        if (millis() - last_connect_attempt_ >= connect_interval_ms_) {
            last_connect_attempt_ = millis();
            doConnect();
        }
        return;
    }

    client_.loop();

    // After a (re)connect, always subscribe before doing anything else.
    if (!subscribed_) {
        doSubscribe();
        return;
    }

    // First time through after subscribing: publish the register message.
    if (!published_) {
        doPublish();
        return;
    }

    // Otherwise we're in WAITING_ACK or REGISTERED — just keep pumping
    // client_.loop() so the provision message can arrive.
}

// -- Internal helpers ---------------------------------------------------------

void MqttProvision::doConnect() {
    strncpy(status_msg_, "Connecting to MQTT broker", sizeof(status_msg_) - 1);
    status_msg_[sizeof(status_msg_) - 1] = '\0';
    if (client_.connect(client_id_.c_str())) {
        strncpy(status_msg_, "Connected to MQTT broker", sizeof(status_msg_) - 1);
        status_msg_[sizeof(status_msg_) - 1] = '\0';
        Serial.println("[MQTT] Connected");
    } else {
        snprintf(error_msg_, sizeof(error_msg_), "Connect failed (rc=%d)", client_.getState());
        Serial.printf("[MQTT] %s\n", error_msg_);
    }
}

void MqttProvision::doSubscribe() {
    if (client_.subscribe(topic_.c_str())) {
        subscribed_ = true;
        strncpy(status_msg_, "Subscribed to topic", sizeof(status_msg_) - 1);
        status_msg_[sizeof(status_msg_) - 1] = '\0';
        Serial.printf("[MQTT] Subscribed %s\n", topic_.c_str());
    } else {
        Serial.println("[MQTT] Subscribe failed");
    }
}

void MqttProvision::doPublish() {
    String payload = "{\"type\":\"register\",\"firmware_version\":\"" FIRMWARE_VERSION "\"}";
    if (client_.publish(topic_.c_str(), payload.c_str())) {
        published_ = true;
        strncpy(status_msg_, "Published to topic", sizeof(status_msg_) - 1);
        status_msg_[sizeof(status_msg_) - 1] = '\0';
        Serial.printf("[MQTT] Published to %s\n", topic_.c_str());
    } else {
        Serial.println("[MQTT] Publish failed");
    }
}

void MqttProvision::handleMessage(const char* topic, const char* payload, unsigned int length) {
    (void)topic;
    (void)length;
    Serial.printf("[MQTT] Rx: %s\n", payload);

    String type = extractJson(payload, "type");

    // Ignore our own register echo.
    if (type == "register") return;

    // Provision message — save bridge_id and we're done.
    if (type == "provision") {
        String bridgeId = extractJson(payload, "bridge_id");
        if (bridgeId.length() > 0) {
            NvsManager::instance().registerNamespace(NVS_NS);
            if (NvsManager::instance().putString(NVS_NS, NVS_BRIDGE, bridgeId)) {
                state_ = MqttProvisionState::PROVISIONED;
                strncpy(status_msg_, "Provisioned", sizeof(status_msg_) - 1);
                status_msg_[sizeof(status_msg_) - 1] = '\0';
                Serial.printf("[MQTT] Provisioned bridge_id=%s\n", bridgeId.c_str());
            } else {
                Serial.println("[MQTT] Failed to save bridge_id to NVS");
            }
        } else {
            Serial.println("[MQTT] Provision message missing bridge_id");
        }
        return;
    }

    // Ack from server after register.
    if (type == "ack" && extractJson(payload, "status") == "registered") {
        state_ = MqttProvisionState::REGISTERED;
        strncpy(status_msg_, "Waiting for adoption", sizeof(status_msg_) - 1);
        status_msg_[sizeof(status_msg_) - 1] = '\0';
        Serial.println("[MQTT] Registered");
    }
}
