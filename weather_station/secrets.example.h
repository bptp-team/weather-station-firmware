#ifndef WEATHER_STATION_SECRETS_EXAMPLE_H
#define WEATHER_STATION_SECRETS_EXAMPLE_H

#define WIFI_SSID "your-wifi-network"
#define WIFI_PASSWORD "your-wifi-password"

// Use the Docker host's LAN IP, not localhost. Add or remove entries as
// needed; the sketch calculates the broker count from this array.
#define MQTT_HOSTS                                                        \
	{                                                                       \
		{"192.168.1.20", 1883},                                              \
		{"mqtt.example.com", 1883},                                         \
	}
#define DEVICE_ID "station-01"

// Optional: how often the station reads its sensors, in
// milliseconds. Remove this line to keep the 10000 ms default.
#define READING_INTERVAL_MS 10000

#endif
