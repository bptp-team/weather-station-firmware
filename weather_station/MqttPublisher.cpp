#include "MqttPublisher.h"
#include "Logger.h"

const char *MQTT_LOG_SOURCE = "MQTT";

MqttPublisher::MqttPublisher(const char *wifiSsid, const char *wifiPassword,
                             const MqttBrokerConfig *mqttBrokers,
                             size_t mqttBrokerCount,
                             const char *deviceId)
    : wifiSsid(wifiSsid),
      wifiPassword(wifiPassword),
      deviceId(deviceId) {
  brokers.reserve(mqttBrokerCount);
  for (size_t brokerIndex = 0; brokerIndex < mqttBrokerCount; brokerIndex++) {
    brokers.emplace_back(mqttBrokers[brokerIndex]);
  }
}

void MqttPublisher::begin() {
  for (BrokerState &broker : brokers) {
    broker.mqttClient.setServer(broker.config.host, broker.config.port);
  }
  connectToWifi();
}

void MqttPublisher::maintainConnection() {
  const unsigned long currentTimeMs = millis();
  const auto wifiStatus = WiFi.status();
  const bool isWifiConnected = wifiStatus == WL_CONNECTED;
  const bool hasWifiConnectionFailed = wifiStatus == WL_CONNECT_FAILED;
  const bool isWifiRetryDue =
      currentTimeMs - lastWifiAttemptMs >= WIFI_RETRY_INTERVAL_MS;

  if (isWifiConnected) {
    if (!wifiWasConnected) {
      logEvent(INFO, MQTT_LOG_SOURCE, "Connected to Wi-Fi");
      wifiWasConnected = true;
    }
    wifiFailureWasLogged = false;
  } else {
    if (wifiWasConnected) {
      logEvent(WARNING, MQTT_LOG_SOURCE, "Wi-Fi connection lost");
      wifiWasConnected = false;
    } else if (hasWifiConnectionFailed && !wifiFailureWasLogged) {
      logEvent(WARNING, MQTT_LOG_SOURCE, "Wi-Fi connection failed");
      wifiFailureWasLogged = true;
    }

    if (isWifiRetryDue) {
      connectToWifi();
    }
    return;
  }

  for (BrokerState &broker : brokers) {
    const bool isMqttConnected = broker.mqttClient.connected();
    const bool isMqttRetryDue = currentTimeMs - broker.lastMqttAttemptMs >=
                                MQTT_RETRY_INTERVAL_MS;

    if (!isMqttConnected) {
      if (isMqttRetryDue) {
        connectToMqtt(broker);
      }
      continue;
    }

    broker.mqttClient.loop();
  }
}

void MqttPublisher::publishReading(const WeatherReading &reading) {
  for (BrokerState &broker : brokers) {
    if (!broker.mqttClient.connected()) {
      continue;
    }

    publishText(broker, "airTemperature",
                String(reading.temperatureCelsius, 2));
    publishText(broker, "airPressure", String(reading.pressurePascals, 2));
    publishText(broker, "airHumidity",
                String(reading.relativeHumidityPercent, 2));
    publishText(broker, "daylight", String(reading.daylightRaw));
    publishText(broker, "waterLevel", String(reading.waterLevelRaw));
    publishText(broker, "airQuality", String(reading.airQualityRaw));
  }
}

void MqttPublisher::connectToWifi() {
  lastWifiAttemptMs = millis();
  wifiFailureWasLogged = false;
  logEvent(INFO, MQTT_LOG_SOURCE, "Connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);
}

void MqttPublisher::connectToMqtt(BrokerState &broker) {
  broker.lastMqttAttemptMs = millis();
  String clientId = String(deviceId) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  logEvent(INFO, MQTT_LOG_SOURCE, "Connecting to MQTT broker");
  if (broker.mqttClient.connect(clientId.c_str())) {
    logEvent(INFO, MQTT_LOG_SOURCE, "Connected to MQTT broker");
  } else {
    logEvent(WARNING, MQTT_LOG_SOURCE, "MQTT connection failed");
  }
}

String MqttPublisher::topicFor(const char *measurement) const {
  return String("weather/") + deviceId + "/" + measurement;
}

bool MqttPublisher::publishText(BrokerState &broker, const char *measurement,
                                const String &payload) {
  String topic = topicFor(measurement);
  return broker.mqttClient.publish(topic.c_str(), payload.c_str());
}
