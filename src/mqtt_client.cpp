#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "mqtt_client.h"
#include "display.h"

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

static const char* g_host = nullptr;
static uint16_t g_port = 1883;
static const char* g_topic = nullptr;

static char msgBuf[256];

static void showPayloadOnOled(const char* payload) {
  // Expect "line1\nline2" (line2 optional)
  const char* nl = strchr(payload, '\n');

  if (!nl) {
    display_text(payload, nullptr);
    return;
  }

  char line1[64];
  char line2[64];

  size_t line1Len = (size_t)(nl - payload);
  line1Len = min(line1Len, sizeof(line1) - 1);
  memcpy(line1, payload, line1Len);
  line1[line1Len] = '\0';

  const char* second = nl + 1;
  strncpy(line2, second, sizeof(line2) - 1);
  line2[sizeof(line2) - 1] = '\0';

  display_text(line1, line2);
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  unsigned int n = min(length, (unsigned int)(sizeof(msgBuf) - 1));
  memcpy(msgBuf, payload, n);
  msgBuf[n] = '\0';

  Serial.printf("MQTT topic=%s len=%u payload=\n%s\n", topic, n, msgBuf);
  showPayloadOnOled(msgBuf);
}

static bool connectMqtt() {
  if (!g_host || !g_topic) return false;

  mqtt.setServer(g_host, g_port);
  mqtt.setCallback(onMqttMessage);

  String clientId = "esp32-oled-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("Connecting to MQTT...");
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("connected");
    mqtt.subscribe(g_topic, 1); // QoS 1
    return true;
  }

  Serial.printf("failed rc=%d\n", mqtt.state());
  return false;
}

void mqtt_init(const char* host, uint16_t port, const char* topic) {
  g_host = host;
  g_port = port;
  g_topic = topic;

  // MQTT stability improvement on ESP32
  WiFi.setSleep(false);

  // Try initial connect
  connectMqtt();
  display_text("MQTT", "waiting...");
}

void mqtt_loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // Don’t attempt MQTT if WiFi is down
    return;
  }

  if (!mqtt.connected()) {
    static uint32_t lastTry = 0;
    if (millis() - lastTry > 2000) {
      lastTry = millis();
      connectMqtt();
    }
    return;
  }

  mqtt.loop();
}

bool mqtt_is_connected() {
  return mqtt.connected();
}

void mqtt_force_reconnect() {
  if (mqtt.connected()) mqtt.disconnect();
  connectMqtt();
}
