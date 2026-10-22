#ifndef WIFI_H
#define WIFI_H

// Host stub of the ESP32 Wi-Fi API used by MqttPublisher.

#include <string>
#include <vector>

enum wl_status_t {
  WL_IDLE_STATUS,
  WL_NO_SSID_AVAIL,
  WL_SCAN_COMPLETED,
  WL_CONNECTED,
  WL_CONNECT_FAILED,
  WL_CONNECTION_LOST,
  WL_DISCONNECTED
};

enum wifi_mode_t { WIFI_OFF, WIFI_STA, WIFI_AP, WIFI_AP_STA };

// The transport PubSubClient writes through. Nothing to fake: the MQTT stub
// never touches the socket.
class Client {
public:
  virtual ~Client() = default;
};

class WiFiClient : public Client {};

class WiFiStub {
public:
  wl_status_t status() const;
  void mode(wifi_mode_t mode);
  void begin(const char *ssid, const char *password);
};

extern WiFiStub WiFi;

namespace fake {

// One call to WiFi.begin().
struct WifiConnectAttempt {
  std::string ssid;
  std::string password;
};

void resetWifi();
void setWifiStatus(wl_status_t status);
const std::vector<WifiConnectAttempt> &wifiConnectAttempts();

} // namespace fake

#endif
