#include "CommBridge.h"
#include "config.h"
#include "mqttHandler.h"  
#include <WiFi.h>

extern WiFiServer TheServeur;
extern WiFiClient pfodClient;
extern HardwareSerial Serial2;

extern BluetoothSerial SerialBT;     // uncomment if You use Bluetooth

uint8_t WIFIbuf[my_bufferSize];
uint16_t inWiFI = 0;
extern char line_receive[256];
extern byte mon_index;

void handleCommBridge() {
  bool staReceived = false;
  static char mqttProcessingBuffer[256]; // Statičen buffer za varno obdelavo niza

  // 1. Manage WiFi PFOD clients (Standard)
  if (TheServeur.hasClient()) {
    if (!pfodClient || !pfodClient.connected()) {
      if (pfodClient) pfodClient.stop();
      pfodClient = TheServeur.available();
      if (debug) Serial.println("New WIFI client connected");
    } else {
      WiFiClient tempClient = TheServeur.available();
      tempClient.stop();
    }
  }

  // 2. Data from PFOD (WiFi) -> DUE (Serial2)
  if (pfodClient && pfodClient.available()) {
    int availableWiFi = pfodClient.available();
    if (availableWiFi > my_bufferSize) availableWiFi = my_bufferSize;
    
    int bytesRead = pfodClient.read(WIFIbuf, availableWiFi);
    if (bytesRead > 0) {
      Serial2.write(WIFIbuf, bytesRead);
    }
  }

  // 3. Data from DUE (Serial2) -> WiFi/BT in MQTT analize
  //    DUE sends some 10 data to console starts with #RMSTA: STATE, STATUS, BATVOLT etc 
  if (Serial2.available()) {
    inWiFI = 0; // Reset counter for packet 
    
    while (Serial2.available()) {
      char aChar = Serial2.read();

      // Fill bufferj to send on WiFi/BT (pfodApp)
      if (inWiFI < my_bufferSize - 1) {
        WIFIbuf[inWiFI++] = aChar;
      }

      // Fill line_receive buffer for MQTT analize
      if (aChar == '\n') {
        line_receive[mon_index] = '\0';
        
        if (strncmp(line_receive, "#RMSTA", 6) == 0) {
          strncpy(mqttProcessingBuffer, line_receive, sizeof(mqttProcessingBuffer));
          staReceived = true; 
        }
        
        mon_index = 0; // Prepare for next line data
      } else {
        if (mon_index < 255) {
          line_receive[mon_index++] = aChar;
        }
      }
    }

    // A. Sending data to WiFi 
    if (pfodClient && inWiFI > 0) {
      pfodClient.write(WIFIbuf, inWiFI);
    }

    // B. Only after the serial transmission is complete, process MQTT in peace
    // This prevents client.publish from causing "garbage" on the Serial2 input
    if (staReceived) {
      esp32_Mqtt_sta(mqttProcessingBuffer);
      staReceived = false; // Ponastavi zastavico
    }
  }
}
