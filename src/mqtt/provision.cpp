#include "mqtt/provision.h"

static MqttProvision* s_provision = nullptr;

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

void MqttProvision::onMessage(const char* topic, const char* payload, unsigned int length) {
    if (s_provision) s_provision->handleMessage(topic, payload, length);
}

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

void MqttProvision::loop() {
    if (state_ == MqttProvisionState::IDLE || state_ == MqttProvisionState::REGISTERED ||
        state_ == MqttProvisionState::ERROR) return;
    if (!client_.isConnected()) {
        if (millis() - last_connect_attempt_ >= connect_interval_ms_) {
            last_connect_attempt_ = millis();
            doConnect();
        }
        return;
    }
    client_.loop();
    if (state_ == MqttProvisionState::CONNECTING && client_.isConnected()) {
        state_ = MqttProvisionState::SUBSCRIBING;
        subscribed_ = false;
    }
    if (state_ == MqttProvisionState::SUBSCRIBING && !subscribed_) doSubscribe();
    if (state_ == MqttProvisionState::SUBSCRIBING && subscribed_) {
        state_ = MqttProvisionState::PUBLISHING;
        published_ = false;
    }
    if (state_ == MqttProvisionState::PUBLISHING && !published_) doPublish();
    if (state_ == MqttProvisionState::PUBLISHING && published_) state_ = MqttProvisionState::WAITING_ACK;
}

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
    (void)length;
    String type = extractJson(payload, "type");
    if (type == "register") return;
    if (type == "ack" && extractJson(payload, "status") == "registered") {
        state_ = MqttProvisionState::REGISTERED;
        strncpy(status_msg_, "Waiting for adoption", sizeof(status_msg_) - 1);
        status_msg_[sizeof(status_msg_) - 1] = '\0';
        Serial.println("[MQTT] Registered");
    }
}
