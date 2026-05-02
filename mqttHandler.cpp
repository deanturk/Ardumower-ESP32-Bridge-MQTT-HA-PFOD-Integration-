#include "mqttHandler.h"
#include "config.h"
#include "sensors.h"
#include <Arduino.h>

extern boolean useMqtt;
extern unsigned long next_test_connection;
extern PubSubClient client;

extern HardwareSerial Serial2;  // Da lahko pošiljaš {ra}, {nf}, itd.
extern String SplitResult[];    // Rezultati razreza niza

// Prenos tvojih spremenljivk
String SplitResult[10];
extern const char* mower_name;
extern const char* mqtt_user;
extern const char* mqtt_pass;
extern const char* mqtt_id;
extern const char* mqtt_subscribeTopic1;

// Topic konstante (jih bova uporabila v esp32_Mqtt_sta)
extern const char* mqtt_statusTopic;
extern const char* mqtt_stateTopic;
extern const char* mqtt_tempTopic;
extern const char* mqtt_batteryTopic;
extern const char* mqtt_idleTopic;


int csvSplit(String string) {
  String tempString = "";
  int bufferIndex = 0;
  for (int i = 0; i < string.length(); ++i) {
    char c = string[i];
    if (c != ';') {
      tempString += c;
    } else {
      tempString += '\0';
      SplitResult[bufferIndex++] = tempString;
      tempString = "";
    }
  }
  SplitResult[bufferIndex++] = tempString;
  return bufferIndex;
}

void handleMqttConnection() {
  if (useMqtt && !client.connected() && millis() > next_test_connection) {
    next_test_connection = millis() + 5000;
    mqttConnect();
  }
  client.loop();
}

void mqttConnect() {
  while (!client.connected()) {
    Serial.print("Poskus MQTT povezave...");
    // Poskus povezave z imenom kosilnice
    if (client.connect(mower_name)) {
      Serial.println("povezan!");
      client.subscribe("Mower/COMMAND/#");  // Naročimo se na ukaze
    } else {
      Serial.print("napaka, rc=");
      Serial.print(client.state());
      Serial.println(" poskus čez 5 sekund");
      delay(5000);
    }
  }
}

void receivedCallback(char* topic, byte* payload, unsigned int payload_length) {  //data coming from mqtt

  if (debug) Serial.println("--- MQTT Sporocilo Prejeto ---");  // DODAJ TO VRSTICO
  if (debug) Serial.print("Topic: ");
  if (debug) Serial.println(topic);
  if (debug) Serial.print("Payload Length: ");  // DODAJ TO VRSTICO
  if (debug) Serial.println(payload_length);    // DODAJ TO VRSTICO

  //convert payload to string
  String payloadString = "";
  for (int i = 0; i < payload_length; i++) {
    payloadString = payloadString + String(((char)payload[i]));
  }
  if (debug) Serial.print("Payload String: ");  // DODAJ TO VRSTICO
  if (debug) Serial.println(payloadString);     // DODAJ TO VRSTICO


  if (payloadString.length() > 0) {
    //split payload
    int count = csvSplit(payloadString);
    for (int j = 0; j < count; ++j) {
      if (SplitResult[j].length() > 0) {
        if (debug) Serial.print("SplitResult[");  // DODAJ TO VRSTICO
        if (debug) Serial.print(j);
        if (debug) Serial.print("]: ");  // DODAJ TO VRSTICO
        if (debug) Serial.println(SplitResult[j]);
      }
    }
  }

  //   MOWER AUTO MODE
  if (SplitResult[0] == "START") {
    Serial2.println("{ra}");
  }
  if (SplitResult[0] == "STOP") {
    Serial2.println("{ro}");
  }
  if (SplitResult[0] == "HOME") {
    Serial2.println("{rh}");
  }
  if (SplitResult[0] == "PATTERN") {
    Serial2.println("{rp}");
  }

  if (SplitResult[0] == "STARTTIMER") {
    Serial2.println("{ya0`" + SplitResult[1] + "}");  //F("mowPatternCurr")
    Serial2.println("{ya1`" + SplitResult[2] + "}");  //F("laneUseNr")
    Serial2.println("{ya2`" + SplitResult[3] + "}");  //F("rollDir")
    Serial2.println("{ya3`" + SplitResult[4] + "}");  //F("whereToStart")
    Serial2.println("{ya4`" + SplitResult[5] + "}");  //F("areaToGo")
    Serial2.println("{ya5`" + SplitResult[6] + "}");  //F("actualLenghtByLane")
    Serial2.println("{rv}");
  }

  //    MOWER MANUAL CONTROL
  if (SplitResult[0] == "nf") {                                               // "nf" iz Home Assistant payload_press
    if (debug) Serial.println("Prejeto 'nf'. Posiljam ukaz NAPREJ na Due.");  // DODATNI DEBUG IZPIS
    Serial2.println("{nf}");                                                  // <--- ZAMENJAJ {f} Z DEJANSKIM UKAZOM ZA NAPREJ NA ARDUMOWERJU!
  }
  if (SplitResult[0] == "nb") {
    if (debug) Serial.println("Prejeto 'nb'. Posiljam ukaz NAZAJ na Due.");
    Serial2.println("{nb}");
  }
  if (SplitResult[0] == "nl") {
    if (debug) Serial.println("Prejeto 'nl'. Posiljam ukaz LEVO na Due.");
    Serial2.println("{nl}");
  }
  if (SplitResult[0] == "nr") {
    if (debug) Serial.println("Prejeto 'nr'. Posiljam ukaz DESNO na Due.");
    Serial2.println("{nr}");
  }
  if (SplitResult[0] == "nm") {  // "nm" je ukaz iz Ardumower kode
    if (debug) Serial.println("Prejeto 'nm'. Posiljam ukaz za preklop kosilnega motorja na Due.");
    Serial2.println("{nm}");  // Ardumower pričakuje "{nm}" za preklop
  }

  if (SplitResult[0] == "OBRNI") {
    if (debug) Serial.println("YOLO: Ukaz OBRNI prejet preko SplitResult! Pošiljam {y01} na DUE.");
    Serial2.println("{y01}");
  }

  // MOW MOTOR SETTINGS
  if (String(topic).startsWith("Mower/COMMAND/o")) {
    // Iz topica izluščimo ukaz (npr. iz "Mower/COMMAND/o05" dobimo "o05")
    String cmd = String(topic).substring(String(topic).lastIndexOf('/') + 1);

    // Sestavimo pfod niz in ga pošljemo na Serial2 (DUE)
    // Rezultat bo npr: {o05`180}
    Serial2.print("{");
    Serial2.print(cmd);
    if (payloadString.length() > 0) {
      Serial2.print("`" + payloadString);
    }
    Serial2.println("}");
  }
}

//  CONSOLE DATA  #RMSTA, status, state, tempDht, bat, loops, error, yaw, pitch, roll, pattern
void esp32_Mqtt_sta(char* line_receive) {

  Serial.println("\n==============================");
  Serial.println("📥 RAW INPUT FROM DUE:");
  Serial.println(line_receive);

  // ----------------------------------------------------
  // 1. varnostna kopija + cleanup
  // ----------------------------------------------------
  char tempBuffer[384];
  strncpy(tempBuffer, line_receive, sizeof(tempBuffer));
  tempBuffer[sizeof(tempBuffer) - 1] = '\0';

  // odstrani CR/LF
  for (int i = 0; tempBuffer[i]; i++) {
    if (tempBuffer[i] == '\r' || tempBuffer[i] == '\n') {
      tempBuffer[i] = '\0';
      break;
    }
  }

  // mora biti RMSTA
  if (strncmp(tempBuffer, "#RMSTA", 6) != 0) {
    Serial.println("❌ Not RMSTA packet - ignored");
    return;
  }

  // ----------------------------------------------------
  // 2. CSV split
  // ----------------------------------------------------
  char* values[11] = { 0 };

  int i = 0;
  char* token = strtok(tempBuffer, ",");

  while (token != NULL && i < 11) {
    values[i++] = token;
    token = strtok(NULL, ",");
  }

  Serial.println("📊 SPLIT RESULT:");
  for (int j = 0; j < 11; j++) {
    Serial.print("  [");
    Serial.print(j);
    Serial.print("] = ");
    Serial.println(values[j] ? values[j] : "NULL");
  }

  // zaščita
  if (i < 9) {
    Serial.println("❌ Not enough fields in RMSTA");
    return;
  }

  // ----------------------------------------------------
  // 3. JSON build
  // ----------------------------------------------------
  char json[300];

  snprintf(json, sizeof(json),
           "{\"#\":\"%s\",\"status\":\"%s\",\"state\":\"%s\",\"temp\":\"%s\",\"bat\":\"%s\","
           "\"loops\":\"%s\",\"error\":\"%s\",\"yaw\":\"%s\",\"pitch\":\"%s\","
           "\"roll\":\"%s\",\"pattern\":\"%s\"}",
           values[0] ? values[0] : "0",
           values[1] ? values[1] : "0",
           values[2] ? values[2] : "0",
           values[3] ? values[3] : "0",
           values[4] ? values[4] : "0",
           values[5] ? values[5] : "0",
           values[6] ? values[6] : "0",
           values[7] ? values[7] : "0",
           values[8] ? values[8] : "0",
           values[9] ? values[9] : "0",
           values[10] ? values[10] : "0");

  Serial.print("Fields count: ");
  Serial.println(i);

  // ----------------------------------------------------
  // 4. DEBUG JSON
  // ----------------------------------------------------
  Serial.println("📤 MQTT JSON OUTPUT:");
  Serial.println(json);

  // ----------------------------------------------------
  // 5. MQTT publish
  // ----------------------------------------------------
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/telemetry", mower_name);

  bool ok = client.publish(topic, json);

  if (ok) {
    Serial.println("✅ MQTT publish OK");
  } else {
    Serial.println("❌ MQTT publish FAILED");
  }

  Serial.println("==============================\n");
}


//   AHT in BMP senzor inside mower
void handleSensorPublish() {
  SensorData s = readSensors();
  extern const char* mower_name;
  char topic[100];

  if (!isnan(s.aht_temp)) {
    snprintf(topic, sizeof(topic), "%s/AHT_Temp", mower_name);
    client.publish(topic, String(s.aht_temp).c_str());

    snprintf(topic, sizeof(topic), "%s/AHT_Humidity", mower_name);
    client.publish(topic, String(s.aht_hum).c_str());

    if (debug) Serial.printf("AHT20 -> Temp: %.2f, Vlaga: %.2f\n", s.aht_temp, s.aht_hum);
  }

  if (!isnan(s.bmp_press)) {
    snprintf(topic, sizeof(topic), "%s/BMP_Pressure", mower_name);
    client.publish(topic, String(s.bmp_press).c_str());

    snprintf(topic, sizeof(topic), "%s/BMP_Temp", mower_name);
    client.publish(topic, String(s.bmp_temp).c_str());

    if (debug) Serial.printf("BMP280 -> Tlak: %.2f hPa, Temp: %.2f\n", s.bmp_press, s.bmp_temp);
  }
}
