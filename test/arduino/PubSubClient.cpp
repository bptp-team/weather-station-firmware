#include "PubSubClient.h"

namespace {

struct HostConnectionBehavior {
  std::string host;
  bool succeeds;
};

bool connectSucceeds = true;
int loopCalls = 0;
std::vector<fake::MqttMessage> messages;
std::vector<std::string> clientIds;
std::vector<HostConnectionBehavior> hostConnectionBehaviors;

bool connectionSucceedsForHost(const std::string &host) {
  for (const HostConnectionBehavior &behavior : hostConnectionBehaviors) {
    if (behavior.host == host) {
      return behavior.succeeds;
    }
  }
  return connectSucceeds;
}

} // namespace

PubSubClient::PubSubClient(WiFiClient &) {}

PubSubClient &PubSubClient::setServer(const char *host, uint16_t port) {
  serverHost = host == nullptr ? "" : host;
  serverPort = port;
  return *this;
}

bool PubSubClient::connect(const char *clientId) {
  clientIds.push_back(clientId == nullptr ? "" : clientId);
  isConnected = connectionSucceedsForHost(serverHost);
  return isConnected;
}

bool PubSubClient::connected() { return isConnected; }

bool PubSubClient::loop() {
  loopCalls++;
  return isConnected;
}

bool PubSubClient::publish(const char *topic, const char *payload) {
  if (!isConnected) {
    return false;
  }

  messages.push_back({serverHost,
                      topic == nullptr ? "" : topic,
                      payload == nullptr ? "" : payload});
  return true;
}

namespace fake {

void resetMqtt() {
  connectSucceeds = true;
  loopCalls = 0;
  messages.clear();
  clientIds.clear();
  hostConnectionBehaviors.clear();
}

void setMqttConnectSucceeds(bool succeeds) { connectSucceeds = succeeds; }

void setMqttConnectSucceedsForHost(const char *host, bool succeeds) {
  hostConnectionBehaviors.push_back(
      {host == nullptr ? "" : host, succeeds});
}

const std::vector<MqttMessage> &mqttMessages() { return messages; }

const std::vector<std::string> &mqttClientIds() { return clientIds; }

int mqttLoopCalls() { return loopCalls; }

} // namespace fake
