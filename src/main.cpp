// Multi-channel I2C sensor system for ESP32-S3.
// Two hardware I2C buses (Wire / Wire1) are scanned at startup; any known
// sensor found by address is initialised and polled periodically.
#include <Arduino.h>
#include <Wire.h>
#include "Sensors.h"

struct Channel {
  TwoWire *bus;
  int sda, scl;
};

// Adjust pins to match your board wiring.
static Channel channels[] = {
    {&Wire, 8, 9},
    {&Wire1, 17, 18},
};

static const size_t MAX_SENSORS = 16;
static I2CSensor *sensors[MAX_SENSORS];
static int sensorChannel[MAX_SENSORS];
static size_t sensorCount = 0;

static I2CSensor *create(TwoWire &bus, uint8_t addr) {
  switch (addr) {
    case 0x44: case 0x45: return new SHT3x(bus, addr);
    case 0x62: return new SCD4x(bus, addr);
    case 0x76: case 0x77: return new BMP280(bus, addr);
    default: return nullptr;
  }
}

static void scanChannel(int idx) {
  TwoWire &bus = *channels[idx].bus;
  for (uint8_t addr = 1; addr < 127 && sensorCount < MAX_SENSORS; addr++) {
    bus.beginTransmission(addr);
    if (bus.endTransmission() != 0) continue;
    I2CSensor *s = create(bus, addr);
    if (!s) {
      Serial.printf("CH%d: unknown device at 0x%02X\n", idx, addr);
      continue;
    }
    if (s->begin()) {
      Serial.printf("CH%d: %s at 0x%02X\n", idx, s->name(), addr);
      sensorChannel[sensorCount] = idx;
      sensors[sensorCount++] = s;
    } else {
      Serial.printf("CH%d: %s at 0x%02X init failed\n", idx, s->name(), addr);
      delete s;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  for (size_t i = 0; i < sizeof(channels) / sizeof(channels[0]); i++) {
    channels[i].bus->begin(channels[i].sda, channels[i].scl, 100000);
    scanChannel(i);
  }
  Serial.printf("%u sensor(s) ready\n", (unsigned)sensorCount);
}

void loop() {
  for (size_t i = 0; i < sensorCount; i++) {
    Reading r;
    if (!sensors[i]->read(r)) {
      Serial.printf("CH%d %s 0x%02X: read failed\n", sensorChannel[i], sensors[i]->name(), sensors[i]->address());
      continue;
    }
    Serial.printf("CH%d %s 0x%02X:", sensorChannel[i], sensors[i]->name(), sensors[i]->address());
    if (!isnan(r.temperature)) Serial.printf(" T=%.2fC", r.temperature);
    if (!isnan(r.humidity)) Serial.printf(" RH=%.1f%%", r.humidity);
    if (!isnan(r.co2)) Serial.printf(" CO2=%.0fppm", r.co2);
    if (!isnan(r.pressure)) Serial.printf(" P=%.2fhPa", r.pressure);
    Serial.println();
  }
  delay(5000);
}
