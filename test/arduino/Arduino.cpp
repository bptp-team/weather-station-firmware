#include "Arduino.h"

#include <cstdarg>
#include <cstdio>
#include <map>

SerialStub Serial;
EspStub ESP;

namespace {

unsigned long currentTimeMs = 0;
std::map<int, int> analogValues;
std::map<int, int> digitalValues;
std::vector<std::string> events;
std::string serialText;

void record(const std::string &event) { events.push_back(event); }

std::string formatNumber(unsigned long number, int base) {
  char buffer[32];

  switch (base) {
  case HEX:
    snprintf(buffer, sizeof(buffer), "%lx", number);
    break;
  case OCT:
    snprintf(buffer, sizeof(buffer), "%lo", number);
    break;
  default:
    snprintf(buffer, sizeof(buffer), "%lu", number);
    break;
  }

  return buffer;
}

const char *attenuationName(adc_attenuation_t attenuation) {
  switch (attenuation) {
  case ADC_0db:
    return "ADC_0db";
  case ADC_2_5db:
    return "ADC_2_5db";
  case ADC_6db:
    return "ADC_6db";
  case ADC_11db:
    return "ADC_11db";
  default:
    return "UNKNOWN";
  }
}

} // namespace

String::String(int number) : value(std::to_string(number)) {}

String::String(unsigned int number, int base)
    : value(formatNumber(number, base)) {}

String::String(unsigned long number, int base)
    : value(formatNumber(number, base)) {}

String::String(float number, int decimalPlaces) {
  char buffer[64];

  snprintf(buffer, sizeof(buffer), "%.*f", decimalPlaces,
           static_cast<double>(number));
  value = buffer;
}

String::String(double number, int decimalPlaces) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%.*f", decimalPlaces, number);
  value = buffer;
}

void SerialStub::begin(unsigned long baudRate) {
  record("Serial.begin(" + std::to_string(baudRate) + ")");
}

int SerialStub::printf(const char *format, ...) {
  char buffer[512];
  va_list arguments;

  va_start(arguments, format);
  const int written = vsnprintf(buffer, sizeof(buffer), format, arguments);
  va_end(arguments);

  serialText += buffer;

  return written;
}

uint64_t EspStub::getEfuseMac() const { return 0x0011223344556677ULL; }

unsigned long millis() { return currentTimeMs; }

void delay(unsigned long durationMs) {
  record("delay(" + std::to_string(durationMs) + ")");
  currentTimeMs += durationMs;
}

void pinMode(int pin, int mode) {
  record("pinMode(" + std::to_string(pin) + "," +
         (mode == OUTPUT ? "OUTPUT" : "INPUT") + ")");
}

void digitalWrite(int pin, int value) {
  record("digitalWrite(" + std::to_string(pin) + "," +
         (value == HIGH ? "HIGH" : "LOW") + ")");
  digitalValues[pin] = value;
}

int analogRead(int pin) {
  record("analogRead(" + std::to_string(pin) + ")");
  return analogValues[pin];
}

void analogReadResolution(int bits) {
  record("analogReadResolution(" + std::to_string(bits) + ")");
}

void analogSetPinAttenuation(int pin, adc_attenuation_t attenuation) {
  record("analogSetPinAttenuation(" + std::to_string(pin) + "," +
         attenuationName(attenuation) + ")");
}

namespace fake {

void resetBoard() {
  currentTimeMs = 0;
  analogValues.clear();
  digitalValues.clear();
  events.clear();
  serialText.clear();
}

void setMillis(unsigned long newTimeMs) { currentTimeMs = newTimeMs; }

void setAnalogValue(int pin, int value) { analogValues[pin] = value; }

int digitalValue(int pin) { return digitalValues[pin]; }

const std::vector<std::string> &boardEvents() { return events; }

void clearBoardEvents() { events.clear(); }

std::string serialOutput() { return serialText; }

} // namespace fake
