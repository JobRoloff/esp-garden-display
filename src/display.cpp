#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "display.h"

// ---- OLED config ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool display_init(uint8_t i2cAddr) {
  if (!display.begin(SSD1306_SWITCHCAPVCC, i2cAddr)) {
    Serial.println("SSD1306 allocation/init failed. Check wiring/size/address.");
    return false;
  }
  display.clearDisplay();
  display.display();
  return true;
}

void display_hello() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hello");
  display.println("world");
  display.display();
}

void display_text(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  if (line1) display.println(line1);
  if (line2) display.println(line2);
  display.display();
}

void display_label_value(const char* label, int value) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", value);
  display_text(label, buf);
}

void display_label_value_f(const char* label, float value, uint8_t decimals) {
  char buf[32];
  // dtostrf: width=0 means “no minimum width”
  dtostrf(value, 0, decimals, buf);
  display_text(label, buf);
}

// String overloads (call the const char* versions)
void display_label_value(const String& label, int value) {
  display_label_value(label.c_str(), value);
}

void display_label_value_f(const String& label, float value, uint8_t decimals) {
  display_label_value_f(label.c_str(), value, decimals);
}
