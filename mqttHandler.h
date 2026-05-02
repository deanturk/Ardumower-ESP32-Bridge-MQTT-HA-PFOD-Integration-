#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "mqttHandler.h"
#include "config.h"

// Deklaracija zunanjih objektov (da jih vidi tudi .cpp)
extern PubSubClient client;
extern bool debug;
extern boolean useMqtt;
// extern unsigned long next_test_connection;   // deklarirano v mqttHandler.h
extern PubSubClient client;

// Funkcije
void setupMQTT();
void mqttLoop();
void mqttConnect();
void receivedCallback(char* topic, byte* payload, unsigned int length);
void esp32_Mqtt_sta(char* line_receive);
//void parsePfodMenu(char* line);
int csvSplit(String string);
void handleSensorPublish();

#endif