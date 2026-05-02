// Funkcije in dekalracije za senzorje

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

struct SensorData {
  float aht_temp;
  float aht_hum;
  float bmp_temp;
  float bmp_press;
};

void setupSensors();
SensorData readSensors();
void initSensors();

#endif