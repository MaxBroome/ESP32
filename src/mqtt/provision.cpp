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

static void setStatus(char* buf, size_t sz, const char* msg) {
    strncpy(buf, msg, sz - 1);
    buf[sz - 1] = '\0';
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
    setStatus(status_msg_, sizeof(status_msg_), "Connecting...");
    client_.setCallback(onMessage);
    Serial.printf("[mqtt] claim: %s\n", topic_.c_str());
}

void MqttProvision::stop() {
    if (client_.isConnected()) {
        client_.disconnect();
    }
    s_provision = nullptr;
    state_ = MqttProvisionState::IDLE;
}

void MqttProvision::loop() {
    if (state_ == MqttProvisionState::IDLE ||
        state_ == MqttProvisionState::PROVISIONED ||
        state_ == MqttProvisionState::ERROR) return;

    if (!client_.isConnected()) {
        subscribed_ = false;
        if (millis() - last_connect_attempt_ >= connect_interval_ms_) {
            last_connect_attempt_ = millis();
            doConnect();
        }
        return;
    }

    client_.loop();

    if (!subscribed_) {
        doSubscribe();
        return;
    }

    if (!published_) {
        doPublish();
        return;
    }
}

// -- Internal helpers ---------------------------------------------------------

void MqttProvision::doConnect() {
    setStatus(status_msg_, sizeof(status_msg_), "Connecting...");
    if (client_.connect(client_id_.c_str())) {
        setStatus(status_msg_, sizeof(status_msg_), "Registering device...");
        Serial.println("[mqtt] connected");
    } else {
        snprintf(error_msg_, sizeof(error_msg_), "connect failed (rc=%d)", client_.getState());
        Serial.printf("[mqtt] %s\n", error_msg_);
    }
}

void MqttProvision::doSubscribe() {
    if (client_.subscribe(topic_.c_str())) {
        subscribed_ = true;
    } else {
        Serial.println("[mqtt] subscribe failed");
    }
}

void MqttProvision::doPublish() {
    String payload = "{\"type\":\"register\",\"firmware_version\":\"" FIRMWARE_VERSION "\"}";
    if (client_.publish(topic_.c_str(), payload.c_str())) {
        published_ = true;
        Serial.println("[mqtt] registered");
    } else {
        Serial.println("[mqtt] publish failed");
    }
}

void MqttProvision::handleMessage(const char* topic, const char* payload, unsigned int length) {
    (void)topic;
    (void)length;

    String type = extractJson(payload, "type");

    if (type == "register") return;

    if (type == "provision") {
        String bridgeId = extractJson(payload, "bridge_id");
        if (bridgeId.length() > 0) {
            NvsManager::instance().registerNamespace(NVS_NS);
            if (NvsManager::instance().putString(NVS_NS, NVS_BRIDGE, bridgeId)) {
                state_ = MqttProvisionState::PROVISIONED;
                setStatus(status_msg_, sizeof(status_msg_), "Provisioned!");
                Serial.printf("[mqtt] bridge: %s\n", bridgeId.c_str());
            } else {
                Serial.println("[mqtt] failed to save bridge_id");
            }
        } else {
            Serial.println("[mqtt] provision msg missing bridge_id");
        }
        return;
    }

    if (type == "ack" && extractJson(payload, "status") == "registered") {
        state_ = MqttProvisionState::REGISTERED;
        setStatus(status_msg_, sizeof(status_msg_), "Waiting for adoption...");
        Serial.println("[mqtt] waiting for adoption");
    }
}
