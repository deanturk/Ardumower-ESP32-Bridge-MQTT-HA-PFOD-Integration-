#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

// --- SPLOŠNE NASTAVITVE ---
#define VERSION "1.30_Modular"
#define my_bufferSize 1024
extern bool debug;

// --- WIFI NASTAVITVE (STATION MODE) ---
extern boolean MODE_STA;
extern const char* ssid;
extern const char* password;
extern IPAddress ip;
extern IPAddress gateway;
extern IPAddress netmask;

// --- WIFI NASTAVITVE (ACCESS POINT MODE) ---
extern boolean MODE_AP;
extern const char* ssid_ap;
extern const char* pw_ap;
extern IPAddress ip_ap;
extern IPAddress gateway_ap;
extern IPAddress netmask_ap;

// --- MQTT NASTAVITVE ---
extern boolean useMqtt;
extern const char* mower_name;
extern const char* mqtt_server;
extern const uint16_t mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_pass;
extern const char* mqtt_id;

// --- MQTT TOPICS ---
extern const char* mqtt_subscribeTopic1;
extern const char* mqtt_tempTopic;
extern const char* mqtt_batteryTopic;
extern const char* mqtt_idleTopic;
extern const char* mqtt_statusTopic;
extern const char* mqtt_stateTopic;

#endif