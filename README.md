# Tides
Tide and weather clock for an ESP32-based TTGO-T5 board with 2.13" epaper display.  
The code is designed to use a TPL5110 to ensure low-power. See schematic.jpg.
All the values you should need to tweak are in env_config.h (including an ifdef
in case you don't have a TPL5110).

This sketch requires 3 libraries:
1. "ESP8266 Weather Station", available from the Arduino Library Manager (or https://github.com/ThingPulse/esp8266-weather-station).  You will only need these 6 files: ESPHTTPClient.h,      ESPWiFi.h, OpenWeatherMapCurrent.cpp, OpenWeatherMapCurrent.h, OpenWeatherMapForecast.cpp, OpenWeatherMapForecast.h and can delete the rest.
2. "Json Streaming Parser" available from the Arduino Library Manager (or https://github.com/squix78/json-streaming-parser)
3. https://github.com/LilyGO/TTGO-Epape-T5-V1.8.git . Even though the code is not packaged as a true library, you can just copy it under your libraries directory and include it.
