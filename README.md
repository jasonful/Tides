# Tides
Tide and weather clock for an ESP32-based TTGO-T5 board with epaper display.  
The code is designed to use a TPL5110 to ensure low-power. See schematic.jpg.
All the values you should need to tweak are in env_config.h (including an ifdef
in case you don't have a TPL5110).

This requires the code from https://github.com/LilyGO/TTGO-Epape-T5-V1.8.git . Even though the code is not packaged as a true library, I just copied it to my libraries directory.

You also need the "ESP8266 Weather Station" library, which you can get from the Arduino Library Manager (or https://github.com/ThingPulse/esp8266-weather-station)
