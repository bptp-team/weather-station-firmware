#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include <vector>

#include "WeatherReading.h"

struct MqttBrokerConfig {
  const char *host;
  uint16_t port;
};

class MqttPublisher {
 public:
  MqttPublisher(const char *wifiSsid, const char *wifiPassword,
                const MqttBrokerConfig *mqttBrokers, size_t mqttBrokerCount,
                const char *deviceId);

  MqttPublisher(const MqttPublisher &) = delete;
  MqttPublisher &operator=(const MqttPublisher &) = delete;

  void begin();
  void maintainConnection();
  void publishReading(const WeatherReading &reading);

 private:
  static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
  static const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

  struct BrokerState {
    explicit BrokerState(const MqttBrokerConfig &config)
        : config(config), mqttClient(wifiClient) {}

    MqttBrokerConfig config;
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    unsigned long lastMqttAttemptMs = 0;
  };

  std::vector<BrokerState> brokers;

  const char *wifiSsid;
  const char *wifiPassword;
  const char *deviceId;

  bool wifiWasConnected = false;
  bool wifiFailureWasLogged = false;
  unsigned long lastWifiAttemptMs = 0;

  void connectToWifi();
  void connectToMqtt(BrokerState &broker);

  String topicFor(const char *measurement) const;
  bool publishText(BrokerState &broker, const char *measurement,
                   const String &payload);
};

#endif
