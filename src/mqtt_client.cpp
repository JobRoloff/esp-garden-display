#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "mqtt_client.h"
#include "display.h"

// Payloads can be ~2KB; PubSubClient drops messages larger than this.
static const size_t kPayloadMax = 2048;

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

static const char* g_host = nullptr;
static uint16_t g_port = 1883;
static const char* g_topic = nullptr;

static char msgBuf[320];
// Deferred display: full payload copied here in callback; parsed and drawn in mqtt_pump_display().
static char pendingDisplayBuf[2048];
static volatile bool pendingDisplay = false;

// Use Print* so IDE/clang resolves Print::print/println (HardwareSerial inherits Print in Arduino core)
static Print* _log = reinterpret_cast<Print*>(&Serial);

// Parse JSON and show raw_latest temp + humidity; fallback to raw text.
static void showPayloadOnOled(const char* payload, size_t len) {
  StaticJsonDocument<2048> doc;
  DeserializationError err = deserializeJson(doc, payload, len);

  if (!err) {
    JsonObject raw = doc["raw_latest"];
    if (!raw.isNull()) {
      float temp = raw["temp"] | 0.0f;
      float hum = raw["humidity"] | 0.0f;
      char line1[24];
      char line2[24];
      snprintf(line1, sizeof(line1), "%.1f C", (double)temp);
      snprintf(line2, sizeof(line2), "%.1f %%", (double)hum);
      display_text(line1, line2);
      return;
    }
  }

  // Fallback: show first line(s) of raw payload (e.g. plain "line1\nline2" messages)
  const char* nl = strchr(payload, '\n');
  if (nl) {
    size_t line1Len = (size_t)(nl - payload);
    line1Len = line1Len >= 63 ? 63 : line1Len;
    char line1[64];
    char line2[64];
    memcpy(line1, payload, line1Len);
    line1[line1Len] = '\0';
    size_t rest = len - (line1Len + 1);
    rest = rest >= 63 ? 63 : rest;
    memcpy(line2, nl + 1, rest);
    line2[rest] = '\0';
    display_text(line1, line2);
  } else {
    char line1[64];
    size_t copy = len >= 63 ? 63 : len;
    memcpy(line1, payload, copy);
    line1[copy] = '\0';
    display_text(line1, nullptr);
  }
}

static volatile size_t pendingDisplayLen = 0;

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Log first chunk only (avoid huge Serial dump)
  size_t logLen = min((size_t)length, sizeof(msgBuf) - 1);
  memcpy(msgBuf, payload, logLen);
  msgBuf[logLen] = '\0';
  char logBuf[380];
  snprintf(logBuf, sizeof(logBuf), "[MQTT RX] topic=%s len=%u\n", topic, (unsigned)length);
  _log->print(logBuf);
  _log->print(msgBuf);
  if (length > logLen) _log->print(F("...\n"));
  else _log->print('\n');

  // Defer display to main loop; copy full payload so we can parse JSON (up to buffer size).
  size_t copyLen = min((size_t)length, sizeof(pendingDisplayBuf) - 1);
  memcpy(pendingDisplayBuf, payload, copyLen);
  pendingDisplayBuf[copyLen] = '\0';
  pendingDisplayLen = copyLen;
  pendingDisplay = true;
}

static bool connectMqtt() {
  if (!g_host || !g_topic) return false;

  mqtt.setServer(g_host, g_port);
  mqtt.setCallback(onMqttMessage);
  // Payloads are ~2KB; messages larger than buffer are dropped by PubSubClient
  mqtt.setBufferSize((unsigned int)kPayloadMax);

  String clientId = "esp32-oled-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  _log->print(F("Connecting to MQTT..."));
  if (mqtt.connect(clientId.c_str())) {
    _log->println(F("connected"));
    mqtt.subscribe(g_topic, 1); // QoS 1
    return true;
  }

  snprintf(msgBuf, sizeof(msgBuf), "failed rc=%d\n", mqtt.state());
  _log->print(msgBuf);
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

void mqtt_pump_display() {
  if (!pendingDisplay) return;
  pendingDisplay = false;
  size_t len = pendingDisplayLen;
  showPayloadOnOled(pendingDisplayBuf, len);
}

bool mqtt_is_connected() {
  return mqtt.connected();
}

void mqtt_force_reconnect() {
  if (mqtt.connected()) mqtt.disconnect();
  connectMqtt();
}
