#include "config.h"

bool debug = true;

// WIFI STATION
boolean MODE_STA = true;
const char *ssid = "xxxxxx";       // YOUR SSID
const char *password = "xxxxxxx";  // YOUR PASS
IPAddress ip(192, 168, 0, 0);
IPAddress gateway(192, 168, 0, 1);
IPAddress netmask(255, 255, 255, 0);

// WIFI AP
boolean MODE_AP = false;
const char *ssid_ap = "ROBOTMOWER";
const char *pw_ap = "";
IPAddress ip_ap(192, 168, 4, 1);
IPAddress gateway_ap(192, 168, 4, 0);
IPAddress netmask_ap(255, 255, 255, 0);

// MQTT
boolean useMqtt = true;
const char* mower_name = "Mower";
const char* mqtt_server = "192.168.0.11";
const uint16_t mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* mqtt_id = "Mower";

const char* mqtt_subscribeTopic1 = "/COMMAND/#";
const char* mqtt_tempTopic = "/Temp";
const char* mqtt_batteryTopic = "/Battery";
const char* mqtt_idleTopic = "/Idle";
const char* mqtt_statusTopic = "/Status";
const char* mqtt_stateTopic = "/State";
