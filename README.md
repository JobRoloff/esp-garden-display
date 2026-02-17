This repo is one part of the long runnign garden project where I try to build something that'll help me grow plants and be a nerd in the process.

Weilding cpp, I'm harassing myself with notifications on what my janky greenhouse is up to... Things like the latest sensor readings. 

## Getting Started

1. At thte project root, create a secretes.ini file and pupulate it the file with the following

```bash
[secrets]
wifi_ssid = "your wifi namme"
wifi_pass = "your wifi password"
```

2. Assuming you have a MQTT broker running and you're using a different host, port and topic, change the setup function in main.cpp

```bash
const char* MQTT_HOST = "192.168.1.66";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "pi-garden-light/oled";
```

3. Upload to the esp32

## Hardware
|item|img|
|-|-|
|Esp32-wroom| <img src="./public/controller_and_breakout.png" alt="esp32 wroom in a pin breakout" width="300"/>|
|Oled display|<img src="./public/oled_display.png" alt="esp32 wroom in a pin breakout" width="300"/>
