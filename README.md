This project is an advanced communication and control bridge for an Ardumower robotic lawn mower, 
based on an ESP32 gateway that connects the mower’s control board (Arduino DUE / PFOD system), MQTT (Home Assistant), WiFi, and optional Bluetooth (HC-05).

It enables full real-time telemetry, remote control, and configuration management of the mower through Home Assistant, while preserving compatibility with the original Ardumower firmware and PFOD menu system.

Key Concept

The system acts as a real-time middleware layer between:

- Ardumower (Arduino DUE + PFOD firmware)
- ESP32 (communication bridge)
- MQTT broker (Home Assistant integration)
- Mobile / Web dashboard (Home Assistant UI)

System Architecture

[ Ardumower DUE ]
        ↕ (Serial / PFOD protocol)
[ ESP32 Bridge ]
   ↕          ↕
WiFi (MQTT)   Bluetooth (optional HC-05)
   ↓
[ Home Assistant ]

Main Features

Real-time Telemetry

The mower continuously sends status updates (~every 3 seconds):

Status (WORKING / CHARGING / STATION)
State machine (DRIVE / MOW / HOME / STOP)
Battery voltage
Loop counter / CPU load
IMU data (Yaw, Pitch, Roll)
Error state
Mowing pattern

All data is published as a single JSON MQTT payload:
Mower/telemetry

Smart MQTT Bridge (ESP32)

The ESP32 handles:

Serial communication with Ardumower DUE
PFOD passthrough to mobile apps
MQTT publishing to Home Assistant
Command routing from Home Assistant → mower
Data parsing and cleanup of noisy serial streams

Includes:

robust packet filtering
RMSTA frame detection
JSON generation from CSV telemetry

Home Assistant Integration

Fully integrated into Home Assistant via MQTT sensors:

Real-time dashboard for mower status
Sensor attributes from JSON payload
Control buttons (Start / Stop / Home / Manual drive)
Live camera integration (optional)
Sensor fusion from onboard ESP32 sensors (AHT20 / BMP280)

Settings System (In Development)

A modular configuration system is being implemented:

Mow motor settings
Drive motor tuning
Safety parameters (bumper, limits, thresholds)
PFOD menu mirroring into Home Assistant UI

Workflow:

Home Assistant → MQTT → ESP32 → PFOD → Ardumower DUE → response back → MQTT → HA UI
