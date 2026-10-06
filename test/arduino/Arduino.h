#ifndef ARDUINO_H
#define ARDUINO_H

// Host stub of the small slice of the Arduino/ESP32 core the firmware uses.
// It exists so the sketch's logic can be compiled and unit tested on a
// development machine; it is never uploaded to the board.

#include <cstdint>
#include <string>
#include <vector>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

enum NumberBase { BIN = 2, OCT = 8, DEC = 10, HEX = 16 };

// ESP32 ADC input attenuation settings.
enum adc_attenuation_t { ADC_0db, ADC_2_5db, ADC_6db, ADC_11db };

// Arduino's own string type, backed by std::string.
class String {
public:
  String() = default;
  String(const char *text) : value(text == nullptr ? "" : text) {}
  explicit String(int number);
  explicit String(unsigned int number, int base = DEC);
  explicit String(unsigned long number, int base = DEC);
  String(float number, int decimalPlaces);
  String(double number, int decimalPlaces);

  const char *c_str() const { return value.c_str(); }
  const std::string &text() const { return value; }

  String &operator+=(const String &other) {
    value += other.value;
    return *this;
  }

private:
  std::string value;
};

inline String operator+(const String &left, const String &right) {
  String joined(left);
  joined += right;
  return joined;
}

// Serial port stub: every printed line is kept in memory for the tests to
// inspect instead of being written to a UART.
class SerialStub {
public:
  void begin(unsigned long baudRate);
  int printf(const char *format, ...);
};

extern SerialStub Serial;

// Board information stub.
class EspStub {
public:
  uint64_t getEfuseMac() const;
};

extern EspStub ESP;

unsigned long millis();
void delay(unsigned long durationMs);
void pinMode(int pin, int mode);
void digitalWrite(int pin, int value);
int analogRead(int pin);
void analogReadResolution(int bits);
void analogSetPinAttenuation(int pin, adc_attenuation_t attenuation);

// Controls the fake board from a test.
namespace fake {

// Clears the clock, the pins, the recorded events and the serial output.
void resetBoard();

void setMillis(unsigned long currentTimeMs);
void setAnalogValue(int pin, int value);
int digitalValue(int pin);

// Every board call the firmware made, in order, e.g. "digitalWrite(25,HIGH)".
const std::vector<std::string> &boardEvents();
void clearBoardEvents();

std::string serialOutput();

} // namespace fake

#endif
