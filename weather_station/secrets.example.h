#ifndef WEATHER_STATION_SECRETS_EXAMPLE_H
#define WEATHER_STATION_SECRETS_EXAMPLE_H

#define WIFI_SSID "your-wifi-network"
#define WIFI_PASSWORD "your-wifi-password"

#define DEVICE_ID "station-01"
#define OTA_PASSWORD "replace-with-a-unique-ota-password"

// Printed by scripts/create-mqtt-password.sh in the platform repository.
#define MQTT_PASSWORD "replace-with-the-station-broker-password"

// Contents of certificates/ca.crt in the platform repository.
const char MQTT_CA_CERTIFICATE[] = R"(-----BEGIN CERTIFICATE-----
replace-with-the-contents-of-ca.crt
-----END CERTIFICATE-----
)";

// LAN brokers take {host, port}. The cloud broker also takes the username,
// the password and the CA certificate, which switch it to TLS. Use the Docker
// host's LAN IP, not localhost.
#define MQTT_HOSTS                                                        \
	{                                                                       \
		{"192.168.1.20", 1883},                                              \
		{"weather-station.mexicocentral.cloudapp.azure.com", 8883, DEVICE_ID,  \
		 MQTT_PASSWORD, MQTT_CA_CERTIFICATE},                                 \
	}

// Optional: how often the station reads its sensors, in
// milliseconds. Remove this line to keep the 10000 ms default.
#define READING_INTERVAL_MS 10000

#endif
