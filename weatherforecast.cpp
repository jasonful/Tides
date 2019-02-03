/**The MIT License (MIT)

Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>

#include <JsonListener.h>
#include <time.h>
#include "weatherforecast.h"
#include "env_config.h"


void PrintForecasts(OpenWeatherMapForecastData forecasts[], uint8_t forecastCount)
{
  Serial.printf("Found %d forecasts in this call\n", forecastCount);
  Serial.println("------------------------------------");
  time_t time;
  for (uint8_t i = 0; i < forecastCount; i++) {
    Serial.printf("---\nForecast number: %d\n", i);
    // {"dt":1527066000, uint32_t observationTime;
    time = forecasts[i].observationTime;
    Serial.printf("observationTime: %d, full date: %s", forecasts[i].observationTime, ctime(&time));
    // "main":{
    //   "temp":17.35, float temp;
    Serial.printf("temp: %f\n", forecasts[i].temp);
    //   "temp_min":16.89, float tempMin;
    Serial.printf("tempMin: %f\n", forecasts[i].tempMin);
    //   "temp_max":17.35, float tempMax;
    Serial.printf("tempMax: %f\n", forecasts[i].tempMax);
    //   "pressure":970.8, float pressure;
    Serial.printf("pressure: %f\n", forecasts[i].pressure);
    //   "sea_level":1030.62, float pressureSeaLevel;
    Serial.printf("pressureSeaLevel: %f\n", forecasts[i].pressureSeaLevel);
    //   "grnd_level":970.8, float pressureGroundLevel;
    Serial.printf("pressureGroundLevel: %f\n", forecasts[i].pressureGroundLevel);
    //   "humidity":97, uint8_t humidity;
    Serial.printf("humidity: %d\n", forecasts[i].humidity);
    //   "temp_kf":0.46
    // },"weather":[{
    //   "id":802, uint16_t weatherId;
    Serial.printf("weatherId: %d\n", forecasts[i].weatherId);
    //   "main":"Clouds", String main;
    Serial.printf("main: %s\n", forecasts[i].main.c_str());
    //   "description":"scattered clouds", String description;
    Serial.printf("description: %s\n", forecasts[i].description.c_str());
    //   "icon":"03d" String icon; String iconMeteoCon;
    Serial.printf("icon: %s\n", forecasts[i].icon.c_str());
    Serial.printf("iconMeteoCon: %s\n", forecasts[i].iconMeteoCon.c_str());
    // }],"clouds":{"all":44}, uint8_t clouds;
    Serial.printf("clouds: %d\n", forecasts[i].clouds);
    // "wind":{
    //   "speed":1.77, float windSpeed;
    Serial.printf("windSpeed: %f\n", forecasts[i].windSpeed);
    //   "deg":207.501 float windDeg;
    Serial.printf("windDeg: %f\n", forecasts[i].windDeg);
    // rain: {3h: 0.055}, float rain;
    Serial.printf("rain: %f\n", forecasts[i].rain);
    // },"sys":{"pod":"d"}
    // dt_txt: "2018-05-23 09:00:00"   String observationTimeText;
    Serial.printf("observationTimeText: %s\n", forecasts[i].observationTimeText.c_str());
  }
}


int GetWeatherForecasts(
  OpenWeatherMapForecastData forecasts[], 
  uint8_t forecastMax) 
{
  OpenWeatherMapForecast client;

  client.setMetric(CONFIG_WEATHER_IS_METRIC);
  client.setLanguage(CONFIG_OPEN_WEATHER_MAP_LANGUAGE);

  uint8_t allowedHours[24];
  uint8_t allowedHoursCount = CONFIG_ALLOWED_HOURS_END - CONFIG_ALLOWED_HOURS_START;
  for (int i = 0; i < allowedHoursCount; i++)
  {
    allowedHours[i] = (i + CONFIG_ALLOWED_HOURS_START - CONFIG_OFFSET_FROM_UTC + 24) % 24;
  }
  client.setAllowedHours(allowedHours, allowedHoursCount);

  uint8_t forecastCount = client.updateForecastsById(forecasts, CONFIG_OPEN_WEATHER_MAP_APP_ID, CONFIG_OPEN_WEATHER_MAP_LOCATION_ID, forecastMax);
  PrintForecasts(forecasts, forecastCount);
  return forecastCount;
}