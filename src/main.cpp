#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "display.h"
#include "mqtt_client.h"

static String WIFI_SSID_STR;
static String WIFI_PASS_STR;

#ifndef WIFI_SSID
  #define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS ""
#endif


static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID_STR.c_str(), WIFI_PASS_STR.c_str());

  Serial.print("Connecting to WiFi");

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    if (millis() - start > 20000) {
      Serial.println("\nWiFi timeout");
      return;
    }
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // Load secrets from build_flags macros
  WIFI_SSID_STR = String(WIFI_SSID);
  WIFI_PASS_STR = String(WIFI_PASS);

  // ---- I2C pins on ESP32 ----
  static const int SDA_PIN = 21;
  static const int SCL_PIN = 22;

  // MQTT broker (LAN IP of Mosquitto)
  const char* MQTT_HOST = "192.168.1.66";
  const uint16_t MQTT_PORT = 1883;
  const char* MQTT_TOPIC = "pi-garden-light/oled";

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  pinMode(SCL_PIN, INPUT_PULLUP);
  pinMode(SDA_PIN, INPUT_PULLUP);

  if (!display_init(0x3C)) {
    while (true) delay(1000);
  }

  display_hello();
  Serial.println("OLED OK");

  connectWiFi();

  // Start MQTT after WiFi is up
  mqtt_init(MQTT_HOST, MQTT_PORT, MQTT_TOPIC);
}

void loop() {
  // Reconnect WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Keep MQTT alive + process incoming messages
  mqtt_loop();
  delay(10);
}
