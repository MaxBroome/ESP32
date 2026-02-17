#pragma once

#include "mqtt/client.h"

enum class MqttProvisionState {
    IDLE,
    CONNECTING,
    SUBSCRIBING,
    PUBLISHING,
    WAITING_ACK,
    REGISTERED,
    PROVISIONED,
    ERROR
};

class MqttProvision {
public:
    void begin(const char* code);
    void loop();
    void stop();

    MqttProvisionState getState() const { return state_; }
    const char* getError() const { return error_msg_; }
    const char* getStatusMessage() const { return status_msg_; }
    bool isRegistered() const { return state_ == MqttProvisionState::REGISTERED; }
    bool isProvisioned() const { return state_ == MqttProvisionState::PROVISIONED; }

    // Check NVS for an existing bridge_id (persists across reboots).
    static bool hasBridgeId();
    static String getBridgeId();

private:
    static void onMessage(const char* topic, const char* payload, unsigned int length);
    void doConnect();
    void doSubscribe();
    void doPublish();
    void handleMessage(const char* topic, const char* payload, unsigned int length);

    MqttClient client_;
    String code_, topic_, client_id_;
    MqttProvisionState state_ = MqttProvisionState::IDLE;
    char error_msg_[64] = {0};
    char status_msg_[48] = {0};
    uint32_t last_connect_attempt_ = 0;
    uint32_t connect_interval_ms_ = 5000;
    bool subscribed_ = false;
    bool published_ = false;
};
