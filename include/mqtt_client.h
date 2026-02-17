#pragma once
#include <Arduino.h>

// Call once after WiFi is connected.
void mqtt_init(const char* host, uint16_t port, const char* topic);

// Call repeatedly in loop().
void mqtt_loop();

// Optional helpers
bool mqtt_is_connected();
void mqtt_force_reconnect();
