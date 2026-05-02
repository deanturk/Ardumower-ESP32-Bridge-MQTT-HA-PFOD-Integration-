//SEE config.h for all the setting
//WARNING to compile without error into tool set the Partition scheme to Huge APP

//Hardware wiring :

//TO RFID BOARD
// ESP-32    <--> PN5180 pin mapping:
// Vin       <--> 5V  if esp32 powered using USB YOU NEED TO SEND THE 5V TO PN5180
// 3.3V      <--> 3.3V
// GND       <--> GND
// SCLK, GPIO18   --> SCLK
// MISO, GPIO19  <--  MISO
// MOSI, GPIO23   --> MOSI
// SS, GPIO12     --> NSS (=Not SS -> active LOW)
// BUSY, GPIO13   <--  BUSY
// Reset, GPIO14  --> RST
//

//TO PCB1.3 if used :
// ESP-32    <--> pcb1.3 pin mapping:
// Vin       <--> 5v ON BT CONNECTOR
// GND       <--> GND ON BT CONNECTOR
// RX2       <--> TX ON BT CONNECTOR
// TX2       <--> RX ON BT CONNECTOR

#include "config.h"
#include "sensors.h"
#include "mqttHandler.h"
#include "CommBridge.h"
#include "otaHandler.h"

#include "PubSubClient.h"
#include <WiFi.h>

// Mrežni objekti
WiFiClient pfodClient;
WiFiClient mqttClient;
PubSubClient client(mqttClient);
WiFiServer TheServeur(8881);

// Časomer za senzorje
unsigned long lastSensorRead = 0;
const long sensorReadInterval = 1000 * 60 * 5;  // 5 minut

unsigned long next_test_connection = 0;
char line_receive[256];
byte mon_index = 0;
// bool debug = true; // Če je nimaš že v Config.cpp

void setup() {
  delay(500);
  Serial.begin(115200);

  // Komunikacija z DUE (Serial2)
  Serial2.begin(19200, SERIAL_8N1, 16, 17);

  // 1. Inicializacija senzorjev (AHT/BMP)
  setupSensors();

  // 2. Inicializacija WiFi povezave (pusti MODE_STA/MODE_AP logiko tukaj ali v CommBridge)
  initNetwork();

  // 3. Inicializacija MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(receivedCallback);

  // 4. Inicializacija WiFi strežnika za PFOD
  TheServeur.begin();
  TheServeur.setNoDelay(true);

  if (debug) Serial.println(">>> Sistem je pripravljen in teče! <<<");
}

void loop() {

  handleOTA();
  
  // A. Skrb za MQTT povezavo in poslušanje ukazov
  if (useMqtt) {
    if (!client.connected()) {
      mqttConnect();
    }
    client.loop();
  }

  // B. Skrb za prenos podatkov med DUE, WiFi in Bluetooth
  handleCommBridge();

  // C. Periodično branje senzorjev in pošiljanje v HA
  if (millis() - lastSensorRead > sensorReadInterval) {
    lastSensorRead = millis();
    handleSensorPublish();
  }
}

// Funkcija za vzpostavitev mreže (prestavljeno iz setup-a za preglednost)
void initNetwork() {
  if (MODE_STA) {
    if (debug) Serial.println("Start ESP32 Station mode");
    WiFi.mode(WIFI_STA);
    WiFi.config(ip, gateway, netmask);
    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 15) {
      delay(500);
      if (debug) Serial.print(".");
      retries++;
    }

    if (WiFi.status() != WL_CONNECTED) {
      MODE_AP = true;
      useMqtt = false;
    } else if (debug) {
      Serial.print("\nWiFi povezan! IP: ");
      Serial.println(WiFi.localIP());
    }
  }

  if (MODE_AP) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_ap, pw_ap);
    WiFi.softAPConfig(ip_ap, ip_ap, netmask_ap);
    if (debug) Serial.printf("Access Point Mode: %s\n", ssid_ap);
  }
}