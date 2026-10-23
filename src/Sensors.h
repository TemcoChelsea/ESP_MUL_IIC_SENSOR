#pragma once
#include <Arduino.h>
#include <Wire.h>

// One measurement set; unsupported quantities stay NAN.
struct Reading {
  float temperature = NAN;  // degC
  float humidity = NAN;     // %RH
  float co2 = NAN;          // ppm
  float pressure = NAN;     // hPa
};

class I2CSensor {
 public:
  I2CSensor(TwoWire &bus, uint8_t addr) : _bus(bus), _addr(addr) {}
  virtual ~I2CSensor() {}
  virtual const char *name() const = 0;
  virtual bool begin() = 0;
  virtual bool read(Reading &out) = 0;
  uint8_t address() const { return _addr; }

 protected:
  TwoWire &_bus;
  uint8_t _addr;

  bool write(const uint8_t *d, size_t n) {
    _bus.beginTransmission(_addr);
    _bus.write(d, n);
    return _bus.endTransmission() == 0;
  }
  bool write16(uint16_t cmd) {
    uint8_t d[2] = {uint8_t(cmd >> 8), uint8_t(cmd)};
    return write(d, 2);
  }
  bool readBytes(uint8_t *d, size_t n) {
    if (_bus.requestFrom(_addr, (uint8_t)n) != n) return false;
    for (size_t i = 0; i < n; i++) d[i] = _bus.read();
    return true;
  }
  // Sensirion CRC-8 (poly 0x31, init 0xFF)
  static uint8_t crc8(const uint8_t *d, size_t n) {
    uint8_t c = 0xFF;
    for (size_t i = 0; i < n; i++) {
      c ^= d[i];
      for (int b = 0; b < 8; b++) c = (c & 0x80) ? (c << 1) ^ 0x31 : (c << 1);
    }
    return c;
  }
};

// Sensirion SHT3x: temperature & humidity (0x44 / 0x45)
class SHT3x : public I2CSensor {
 public:
  SHT3x(TwoWire &b, uint8_t a = 0x44) : I2CSensor(b, a) {}
  const char *name() const override { return "SHT3x"; }
  bool begin() override { return write16(0x30A2); }  // soft reset
  bool read(Reading &o) override {
    uint8_t d[6];
    if (!write16(0x2400)) return false;  // single shot, high repeatability
    delay(16);
    if (!readBytes(d, 6) || crc8(d, 2) != d[2] || crc8(d + 3, 2) != d[5]) return false;
    o.temperature = -45.0f + 175.0f * ((d[0] << 8) | d[1]) / 65535.0f;
    o.humidity = 100.0f * ((d[3] << 8) | d[4]) / 65535.0f;
    return true;
  }
};

// Sensirion SCD4x: CO2, temperature & humidity (0x62)
class SCD4x : public I2CSensor {
 public:
  SCD4x(TwoWire &b, uint8_t a = 0x62) : I2CSensor(b, a) {}
  const char *name() const override { return "SCD4x"; }
  bool begin() override {
    write16(0x3F86);  // stop periodic measurement (ignore if idle)
    delay(500);
    return write16(0x21B1);  // start periodic measurement
  }
  bool read(Reading &o) override {
    uint8_t d[9];
    if (!write16(0xE4B8)) return false;  // get data ready status
    delay(1);
    if (!readBytes(d, 3) || crc8(d, 2) != d[2] || (((d[0] << 8) | d[1]) & 0x07FF) == 0) return false;
    if (!write16(0xEC05)) return false;  // read measurement
    delay(1);
    if (!readBytes(d, 9) || crc8(d, 2) != d[2] || crc8(d + 3, 2) != d[5] || crc8(d + 6, 2) != d[8]) return false;
    o.co2 = (d[0] << 8) | d[1];
    o.temperature = -45.0f + 175.0f * ((d[3] << 8) | d[4]) / 65536.0f;
    o.humidity = 100.0f * ((d[6] << 8) | d[7]) / 65536.0f;
    return true;
  }
};

// Bosch BMP280: pressure & temperature (0x76 / 0x77)
class BMP280 : public I2CSensor {
 public:
  BMP280(TwoWire &b, uint8_t a = 0x76) : I2CSensor(b, a) {}
  const char *name() const override { return "BMP280"; }
  bool begin() override {
    uint8_t id;
    if (!regRead(0xD0, &id, 1) || id != 0x58) return false;
    uint8_t c[24];
    if (!regRead(0x88, c, 24)) return false;
    T1 = c[0] | c[1] << 8; T2 = (int16_t)(c[2] | c[3] << 8); T3 = (int16_t)(c[4] | c[5] << 8);
    P1 = c[6] | c[7] << 8; P2 = (int16_t)(c[8] | c[9] << 8); P3 = (int16_t)(c[10] | c[11] << 8);
    P4 = (int16_t)(c[12] | c[13] << 8); P5 = (int16_t)(c[14] | c[15] << 8); P6 = (int16_t)(c[16] | c[17] << 8);
    P7 = (int16_t)(c[18] | c[19] << 8); P8 = (int16_t)(c[20] | c[21] << 8); P9 = (int16_t)(c[22] | c[23] << 8);
    uint8_t cfg[2] = {0xF4, 0x27};  // temp/press x1, normal mode
    return write(cfg, 2);
  }
  bool read(Reading &o) override {
    uint8_t d[6];
    if (!regRead(0xF7, d, 6)) return false;
    int32_t adcP = ((int32_t)d[0] << 12) | (d[1] << 4) | (d[2] >> 4);
    int32_t adcT = ((int32_t)d[3] << 12) | (d[4] << 4) | (d[5] >> 4);
    int32_t v1 = ((((adcT >> 3) - ((int32_t)T1 << 1))) * T2) >> 11;
    int32_t v2 = (((((adcT >> 4) - (int32_t)T1) * ((adcT >> 4) - (int32_t)T1)) >> 12) * T3) >> 14;
    int32_t tFine = v1 + v2;
    o.temperature = ((tFine * 5 + 128) >> 8) / 100.0f;
    int64_t a = (int64_t)tFine - 128000;
    int64_t b = a * a * P6 + ((a * P5) << 17) + ((int64_t)P4 << 35);
    a = ((a * a * P3) >> 8) + ((a * P2) << 12);
    a = (((int64_t)1 << 47) + a) * P1 >> 33;
    if (a == 0) return false;
    int64_t p = 1048576 - adcP;
    p = (((p << 31) - b) * 3125) / a;
    a = ((int64_t)P9 * (p >> 13) * (p >> 13)) >> 25;
    b = ((int64_t)P8 * p) >> 19;
    p = ((p + a + b) >> 8) + ((int64_t)P7 << 4);
    o.pressure = p / 256.0f / 100.0f;
    return true;
  }

 private:
  uint16_t T1, P1;
  int16_t T2, T3, P2, P3, P4, P5, P6, P7, P8, P9;
  bool regRead(uint8_t reg, uint8_t *d, size_t n) {
    return write(&reg, 1) && readBytes(d, n);
  }
};
