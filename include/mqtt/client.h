#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#ifndef MQTT_BROKER
#define MQTT_BROKER "broker.scorescrape.io"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 8883
#endif
#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 512
#endif

typedef void (*MqttMessageCallback)(const char* topic, const char* payload, unsigned int length);

class MqttClient {
public:
    MqttClient();
    ~MqttClient() = default;

    void setCallback(MqttMessageCallback cb);
    bool connect(const char* client_id);
    void disconnect();
    void loop();
    bool isConnected() { return mqtt_.connected(); }
    int getState() { return mqtt_.state(); }
    bool subscribe(const char* topic);
    bool publish(const char* topic, const char* payload);
    bool publish(const char* topic, const uint8_t* payload, unsigned int length);

private:
    static void onMessage(char* topic, byte* payload, unsigned int length);
    MqttMessageCallback callback_ = nullptr;
    WiFiClientSecure tls_;
    PubSubClient mqtt_;
};
