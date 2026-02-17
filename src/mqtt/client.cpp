#include "mqtt/client.h"

static MqttClient* s_client_for_cb = nullptr;

void MqttClient::onMessage(char* topic, byte* payload, unsigned int length) {
    if (s_client_for_cb && s_client_for_cb->callback_) {
        char* buf = (char*)malloc(length + 1);
        if (buf) {
            memcpy(buf, payload, length);
            buf[length] = '\0';
            s_client_for_cb->callback_(topic, buf, length);
            free(buf);
        }
    }
}

MqttClient::MqttClient() : mqtt_(tls_) {
    tls_.setInsecure();
    mqtt_.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt_.setBufferSize(MQTT_BUFFER_SIZE);
}

void MqttClient::setCallback(MqttMessageCallback cb) {
    callback_ = cb;
    s_client_for_cb = this;
    mqtt_.setCallback(callback_ ? onMessage : nullptr);
}

bool MqttClient::connect(const char* client_id) { return mqtt_.connect(client_id); }
void MqttClient::disconnect() { mqtt_.disconnect(); }
void MqttClient::loop() { mqtt_.loop(); }
bool MqttClient::subscribe(const char* topic) { return mqtt_.subscribe(topic); }
bool MqttClient::publish(const char* topic, const char* payload) { return mqtt_.publish(topic, payload); }
bool MqttClient::publish(const char* topic, const uint8_t* payload, unsigned int length) {
    return mqtt_.publish(topic, payload, length);
}
