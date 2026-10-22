#include "PubSubClient.h"

#include "WiFiClientSecure.h"

namespace {

struct HostConnectionBehavior {
  std::string host;
  bool succeeds;
};

bool connectSucceeds = true;
int loopCalls = 0;
std::vector<fake::MqttMessage> messages;
std::vector<std::string> clientIds;
std::vector<fake::MqttConnectAttempt> connectAttempts;
std::vector<HostConnectionBehavior> hostConnectionBehaviors;

std::string textOrEmpty(const char *text) {
  return text == nullptr ? "" : text;
}

std::string caCertificateOf(Client *transport) {
  const WiFiClientSecure *tlsTransport =
      dynamic_cast<const WiFiClientSecure *>(transport);
  return tlsTransport == nullptr ? ""
                                 : textOrEmpty(tlsTransport->caCertificate());
}

bool connectionSucceedsForHost(const std::string &host) {
  for (const HostConnectionBehavior &behavior : hostConnectionBehaviors) {
    if (behavior.host == host) {
      return behavior.succeeds;
    }
  }
  return connectSucceeds;
}

} // namespace

PubSubClient::PubSubClient(Client &client) : transport(&client) {}

PubSubClient &PubSubClient::setServer(const char *host, uint16_t port) {
  serverHost = host == nullptr ? "" : host;
  serverPort = port;
  return *this;
}

bool PubSubClient::connect(const char *clientId) {
  return connect(clientId, nullptr, nullptr);
}

bool PubSubClient::connect(const char *clientId, const char *username,
                           const char *password) {
  clientIds.push_back(textOrEmpty(clientId));
  connectAttempts.push_back({serverHost, textOrEmpty(clientId),
                             textOrEmpty(username), textOrEmpty(password),
                             caCertificateOf(transport)});
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
  connectAttempts.clear();
  hostConnectionBehaviors.clear();
}

void setMqttConnectSucceeds(bool succeeds) { connectSucceeds = succeeds; }

void setMqttConnectSucceedsForHost(const char *host, bool succeeds) {
  hostConnectionBehaviors.push_back(
      {host == nullptr ? "" : host, succeeds});
}

const std::vector<MqttMessage> &mqttMessages() { return messages; }

const std::vector<std::string> &mqttClientIds() { return clientIds; }

const std::vector<MqttConnectAttempt> &mqttConnectAttempts() {
  return connectAttempts;
}

int mqttLoopCalls() { return loopCalls; }

} // namespace fake
