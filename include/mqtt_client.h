#pragma once
#include <Arduino.h>

// Call once after WiFi is connected.
void mqtt_init(const char* host, uint16_t port, const char* topic);

// Call repeatedly in loop().
void mqtt_loop();

// Call after mqtt_loop() to refresh display with any received message (run from main loop, not callback).
void mqtt_pump_display();

// Optional helpers
bool mqtt_is_connected();
void mqtt_force_reconnect();
