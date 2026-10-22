#ifndef WIFI_CLIENT_SECURE_H
#define WIFI_CLIENT_SECURE_H

// Host stub of the ESP32 TLS transport. It keeps the CA certificate the
// firmware configured so the tests can check which transport a broker uses.

#include "WiFi.h"

class WiFiClientSecure : public WiFiClient {
public:
  void setCACert(const char *rootCA) { rootCertificate = rootCA; }
  const char *caCertificate() const { return rootCertificate; }

private:
  const char *rootCertificate = nullptr;
};

#endif
