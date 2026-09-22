### Weather Station

<p align="justify">
    <img
        src="docs/espressif-systems.svg"
        width="50"
        height="50"
    />
    <img
        src="https://cdn.jsdelivr.net/gh/devicons/devicon@latest/icons/cplusplus/cplusplus-original.svg"
        width="50"
        height="50"
    />
</p>

## Local development

This project targets a classic **ESP32** board using the **Arduino** framework.
The **ESP32** connects to **Wi-Fi** and publishes sensor readings to a
**Mosquitto** broker on the local network. **Cloud services are not part of this
setup.**

### Arduino IDE setup

1. Install the **ESP32 board package** in **Arduino IDE** and select the board
    that matches the hardware.
2. Install these libraries through **Library Manager**:
    - `Adafruit BME280 Library`
    - `Adafruit Unified Sensor`
    - `PubSubClient`
3. Copy `weather_station/secrets.example.h` to
    `weather_station/secrets.h`.
4. Edit `weather_station/secrets.h` with the **Wi-Fi credentials**, the **LAN
    addresses** of the MQTT brokers, a unique **OTA password** and, optionally,
    the **reading interval**.
5. Select an ESP32 partition scheme that includes OTA partitions, such as
    **Default** for a board with enough flash.
6. Open `weather_station/weather_station.ino`, select the **ESP32** board and
    port, then upload the sketch by USB. This first USB upload installs OTA
    support on the board.

`secrets.h` is **ignored by Git**. **Do not commit Wi-Fi passwords** or other
local settings. Anything that changes from one station or test run to the next
belongs there, **not in the sketch**.

Example local configuration:

```cpp
#define WIFI_SSID "your-wifi-network"
#define WIFI_PASSWORD "your-wifi-password"
#define MQTT_HOSTS \
    { \
        {"192.168.1.20", 1883}, \
        {"mqtt.example.com", 1883}, \
    }
#define DEVICE_ID "station-01"
#define OTA_PASSWORD "replace-with-a-unique-ota-password"
#define READING_INTERVAL_MS 10000
```

### Uploading firmware over Wi-Fi

After the first USB upload, power the board with the same Wi-Fi network
available. The Arduino IDE should show a network port named after `DEVICE_ID`
once the station connects to Wi-Fi. Select that port and upload the sketch;
enter the value of `OTA_PASSWORD` when prompted.

OTA is authenticated but not encrypted. Use a unique password for each station
and restrict the device to a trusted network. Do not commit `secrets.h` or put
the real password in documentation. If the network port does not appear,
verify the board is connected to Wi-Fi, the selected partition scheme has OTA
partitions, and the computer and ESP32 are on the same LAN. The firmware must
still be compiled with the same board target and partition layout.

The Arduino CLI can upload through the discovered network port as well. List
available ports, including network ports, with `arduino-cli board list`, then
pass the reported port to `arduino-cli upload` using the same FQBN used for the
USB upload.

`MQTT_HOSTS` is an array of `{host, port}` objects. The sketch calculates its
size automatically, so add or remove broker entries directly in
`secrets.h`. Every reading is published independently to each broker that is
connected. If one broker is unavailable, the other continues normally; the
firmware does not queue or replay messages for a broker that was offline.

`READING_INTERVAL_MS` is **optional**: it sets how often the station reads its
sensors, in **milliseconds**. Leave it out and the sketch falls back to
`10000 ms`. Change it in `secrets.h` while testing so the shorter interval
**never reaches a commit**.

Each broker host must be the **Docker host's LAN IP** or a hostname resolvable
by the **ESP32**. **Do not use** `localhost`: from the **ESP32**, `localhost`
means the **ESP32 itself**. Each port must match the port exposed by its MQTT
broker. The firmware currently supports anonymous, unencrypted TCP
connections; TLS and broker credentials are not configured here.

### Tests

The firmware logic is **unit tested on the development machine**.
`test/arduino/` holds a **host stub** of the small slice of the `Arduino`,
`WiFi` and `PubSubClient` APIs the sketch calls, so the modules under
`weather_station/` can run **without an ESP32 attached**. Only **CMake** and a
**C++17 compiler** are needed:

```bash
cmake -S test -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`EnvironmentSensor` is **left out of the suite**: it halts the board when the
`BME280` is missing, so nothing useful is left to assert. It is covered by the
**firmware compilation** instead.

### MQTT topics

Each station publishes under its **stable device ID**:

| Topic | Meaning | Payload |
| ----- | ------- | ------- |
| `weather/<device-id>/airTemperature` | **Air temperature** | Celsius, decimal text |
| `weather/<device-id>/airPressure` | **Atmospheric pressure** | Pascals, decimal text |
| `weather/<device-id>/airHumidity` | **Relative humidity** | Percent, decimal text |
| `weather/<device-id>/daylight` | **Raw daylight** sensor value | Integer text |
| `weather/<device-id>/waterLevel` | **Raw water-level** sensor value | Integer text |
| `weather/<device-id>/airQuality` | **Raw air-quality** sensor value | Integer text |

For example, station `station-01` publishes to
`weather/station-01/airTemperature`.

### Verify messages

Subscribe from a computer on the **same network**. Replace the **host address**
and **port** if your **Mosquitto** Compose configuration uses different values:

```bash
mosquitto_sub -h 127.0.0.1 -p 1883 -t "weather/#" -v
```

Use the **Docker host's LAN IP** instead of `127.0.0.1` when subscribing from a
different computer. The current local broker allows **anonymous, unencrypted
connections** and should remain **restricted to a trusted development network**.

The firmware services **MQTT** continuously and schedules sensor readings every
**10 seconds**, so temporary **Wi-Fi** or **broker outages** can recover
**without rebooting** the **ESP32**.

## Continuous integration

Every **push** and **pull request** against `main` runs
`.github/workflows/weather-station.yaml`, with **two independent jobs**:

- **Unit tests** builds the suite described above and runs it through `CTest`.
- **Compile firmware** installs the **ESP32 core** and the three libraries,
    copies `secrets.example.h` over `secrets.h` and compiles the sketch for
    `esp32:esp32:esp32`. The example values **only have to compile**; they never
    reach a real network.
