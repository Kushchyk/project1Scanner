# RFID-based Scanner

## Project Overview
This project implements an RFID-based attendance system using ESP32 microcontrollers. The system scans RFID tags, records the attendance data with timestamps, and transmits the data to a server for logging and storage.

## Features
- **RFID Tag Scanning**: Detects and reads RFID tags using the MFRC522 module.
- **Real-Time Data Logging**: Captures the UID of the RFID tag and logs the current time.
- **Server Communication**: Sends attendance records to a server via HTTP.
- **Offline Buffering**: Stores records locally when the server is unavailable, ensuring no data loss.
- **WiFi Connectivity**: Uses WiFi to transmit data to the server.
- **LED Indicators**: Visual feedback for successful scans and errors.

## Hardware Requirements
- **ESP32 Development Board**
- **MFRC522 RFID Reader Module**
- **RFID Tags**
- **LEDs** (Green, Red and Blue, which is built-in for feedback)
- **Resistors**
- **Breadboard and Jumper Wires**
