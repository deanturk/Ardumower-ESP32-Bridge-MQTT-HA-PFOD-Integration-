#include "sensors.h"

// Dejanska definicija objektov
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

void setupSensors() {
  // --- AHT20 ---
  Serial.println("AHT20 test");
  if (aht.begin()) {
    Serial.println("Found AHT20 sensor!");
  } else {
    Serial.println("Didn't find AHT20 sensor, check wiring and I2C address!");
  }

  // --- BMP280 ---
  Serial.println("BMP280 test");
  if (bmp.begin(0x76)) {
    Serial.println("Found BMP280 sensor at 0x76!");
  } else if (bmp.begin(0x77)) {
    Serial.println("Found BMP280 sensor at 0x77!");
  } else {
    Serial.println("Didn't find BMP280 sensor, check wiring and I2C address!");
  }
}

SensorData readSensors() {
  SensorData data;
  sensors_event_t humidity_event, temp_event;

  // Branje AHT20
  if (aht.getEvent(&humidity_event, &temp_event)) {
    data.aht_temp = temp_event.temperature;
    data.aht_hum = humidity_event.relative_humidity;
  } else {
    data.aht_temp = NAN;
    data.aht_hum = NAN;
  }

  // Branje BMP280
  data.bmp_temp = bmp.readTemperature();
  data.bmp_press = bmp.readPressure() / 100.0F;

  return data;
}