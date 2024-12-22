#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "time.h"

#define SS_PIN 5
MFRC522DriverPinSimple ss_pin(SS_PIN);  // Configurable SS pin
MFRC522DriverSPI driver(ss_pin);        // Create SPI driver
MFRC522 mfrc522(driver);                // Create MFRC522 instance

// Network settings
const char* ssid = "";       // Network login
const char* password = "";   // Network password
const char* serverUrl = "";  // Server address

// NTP settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;
const int daylightOffset_sec = 3600;

// PINs for LEDs
const int GREEN_LED = 12;
const int RED_LED = 4;
const int BLUE_LED = 2;

// Structure for storing UID and time
struct StudentRecord {
  String uid;
  String time;
};

// Buffer for storing data
std::vector<StudentRecord> buffer;
int BUFFER_SIZE = 200;
String macAddress = "";


// Task for reading RFID tags
void readRFID(void* pvParameters) {
  while (true) {
    if (buffer.size() < BUFFER_SIZE) {

      if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        delay(50);
        return;
      }

      // Read UID
      String uidStr = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidStr += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
      }
      uidStr.toUpperCase();

      // Get current time
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        char timeBuffer[30];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        String data = String(timeBuffer);
        Serial.println(data);

        // Store in buffer
        buffer.push_back({ uidStr, data });

        // Indicate success with green LED
        digitalWrite(GREEN_LED, HIGH);
        delay(1000);
        digitalWrite(GREEN_LED, LOW);

        // Display
        Serial.printf("UID: %s, Time: %s\n", uidStr, data);
      } else {
        Serial.println("Failed to get time");
      }
      delay(3000);
    } else {
      continue;
    }
  }
}

// Task for sending data to the server
void sendDataToServer(void* pvParameters) {
  while (true) {
    if (WiFi.status() == WL_CONNECTED && !buffer.empty()) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      // Prepare JSON
      String jsonData = "{\"mac\":\"" + macAddress + "\",\"uid\":\"" + buffer.front().uid + "\",\"time\":\"" + buffer.front().time + "\"}";

      // Send data
      int httpResponseCode = http.POST(jsonData);
      if (httpResponseCode == 200 || httpResponseCode == 500) {
        Serial.println("Data sent successfully");
        buffer.erase(buffer.begin());
        digitalWrite(BLUE_LED, HIGH);
        delay(1000);
        digitalWrite(BLUE_LED, LOW);
      } else {
        Serial.printf("Error sending data, response code: %d\n", httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi not connected, retrying...");
      WiFi.begin(ssid, password);
      delay(2000);
      if (WiFi.status() == WL_CONNECTED) {
        macAddress = WiFi.macAddress();
      }
    }
  }
}

void setup() {
  Serial.begin(9600);  // Початок послідовного зв'язку

  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  for (unsigned short int i = 0; i < 5 && WiFi.status() != WL_CONNECTED; i++) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    macAddress = WiFi.macAddress();
    Serial.printf("Connected to WiFi, MAC: %s\n", macAddress.c_str());
  } else {
    Serial.println("Failed to connect to WiFi");
  }

  // Configure time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Initialize RFID
  mfrc522.PCD_Init();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  // Create tasks
  xTaskCreate(readRFID, "readRFID", 4096, NULL, 1, NULL);
  xTaskCreate(sendDataToServer, "sendDataToServer", 4096, NULL, 1, NULL);
}

void loop() {
}
