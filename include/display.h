#pragma once
#include <Arduino.h>

bool display_init(uint8_t i2cAddr = 0x3C);

// Convenience screens
void display_hello();

// Print 1–2 lines
void display_text(const char* line1, const char* line2 = nullptr);

// Print "label" on line 1 and "value" on line 2
void display_label_value(const char* label, int value);
void display_label_value_f(const char* label, float value, uint8_t decimals = 2);

// Optional: if you prefer Arduino String inputs
void display_label_value(const String& label, int value);
void display_label_value_f(const String& label, float value, uint8_t decimals = 2);
