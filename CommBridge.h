#ifndef COMMBRIDGE_H
#define COMMBRIDGE_H

#include <Arduino.h>
#include <WiFi.h>
#include "BluetoothSerial.h"

void setupCommBridge();
void handleCommBridge(); // To bo vsebovalo tvoj glavni loop za podatke

#endif