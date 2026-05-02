#include "CommBridge.h"
#include "config.h"
#include "mqttHandler.h"  // Da pozna funkcijo esp32_Mqtt_sta
#include <WiFi.h>

// Povemo CommBridge-u, da ti objekti živijo v .ino datoteki
extern WiFiServer TheServeur;
extern WiFiClient pfodClient;
extern HardwareSerial Serial2;
// Če uporabljaš Bluetooth, odkomentiraj spodnjo vrstico
extern BluetoothSerial SerialBT;

// Ker si imel napako pri definiciji WIFIbuf, ga definiraj tukaj:
uint8_t WIFIbuf[my_bufferSize];
uint16_t inWiFI = 0;
extern char line_receive[256];
extern byte mon_index;

void handleCommBridge() {
  bool staReceived = false;
  static char mqttProcessingBuffer[256]; // Statičen buffer za varno obdelavo niza

  // 1. Upravljanje WiFi PFOD odjemalcev (Standardno)
  if (TheServeur.hasClient()) {
    if (!pfodClient || !pfodClient.connected()) {
      if (pfodClient) pfodClient.stop();
      pfodClient = TheServeur.available();
      if (debug) Serial.println("New WIFI client connected");
    } else {
      // Zavrni dodatne kliente, če je eden že povezan
      WiFiClient tempClient = TheServeur.available();
      tempClient.stop();
    }
  }

  // 2. Podatki iz PFOD (WiFi) -> DUE (Serial2)
  if (pfodClient && pfodClient.available()) {
    int availableWiFi = pfodClient.available();
    if (availableWiFi > my_bufferSize) availableWiFi = my_bufferSize;
    
    int bytesRead = pfodClient.read(WIFIbuf, availableWiFi);
    if (bytesRead > 0) {
      Serial2.write(WIFIbuf, bytesRead);
    }
  }

  // 3. Podatki iz DUE (Serial2) -> WiFi/BT in MQTT analiza
  if (Serial2.available()) {
    inWiFI = 0; // Resetiramo števec za paket, ki gre na WiFi

    while (Serial2.available()) {
      char aChar = Serial2.read();

      // Polnjenje bufferja za takojšnje pošiljanje na WiFi/BT (pfodApp)
      if (inWiFI < my_bufferSize - 1) {
        WIFIbuf[inWiFI++] = aChar;
      }

      // Polnjenje line_receive bufferja za MQTT analizo
      if (aChar == '\n') {
        line_receive[mon_index] = '\0'; // Zaključi niz
        
        // Če najdemo glavo statusa, si jo prekopiramo za kasnejšo obdelavo
        if (strncmp(line_receive, "#RMSTA", 6) == 0) {
          strncpy(mqttProcessingBuffer, line_receive, sizeof(mqttProcessingBuffer));
          staReceived = true; 
        }
        
        mon_index = 0; // Priprava na naslednjo vrstico
      } else {
        if (mon_index < 255) {
          line_receive[mon_index++] = aChar;
        }
      }
    }

    // A. TAKOJ pošlji nepoškodovane podatke na WiFi (Prioriteta!)
    if (pfodClient && inWiFI > 0) {
      pfodClient.write(WIFIbuf, inWiFI);
    }

    // B. Šele ko je serijski prenos končan, v miru obdelaj MQTT
    // To prepreči, da bi client.publish povzročil "smeti" na Serial2 vhodu
    if (staReceived) {
      esp32_Mqtt_sta(mqttProcessingBuffer);
      staReceived = false; // Ponastavi zastavico
    }
  }
}