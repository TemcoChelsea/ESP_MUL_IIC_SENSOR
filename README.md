# ESP_MUL_IIC_SENSOR
Multi-channel I2C sensor system based on ESP32-S3, compatible with various I2C
sensors such as temperature & humidity, CO₂, and pressure sensors.

## Features
- Two hardware I2C channels (`Wire` on GPIO8/9, `Wire1` on GPIO17/18; edit `channels[]` in `src/main.cpp`).
- Auto-scan of each channel; known sensors are detected by I2C address and initialised.
- Built-in drivers: SHT3x (temp/humidity), SCD4x (CO₂/temp/humidity), BMP280 (pressure/temp).
- Add a sensor by subclassing `I2CSensor` in `src/Sensors.h` and registering its address in `create()`.

## Build
PlatformIO: `pio run -t upload && pio device monitor`
