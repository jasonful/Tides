#include <WiFiClientSecure.h>
#include <epd2in13.h>
#include <epdif.h>
#include <epdpaint.h>
#include <fonts.h>
#include <SPI.h>
#include "env_config.h"
#include "weatherforecast.h"
#include "restartcounter.h"

#define DEBUG
#include <Dbg.h>
Dbg dbg;

enum Color {
  Black = 0,
  White = 1
};

unsigned char rgbPaint[16*250];

void ConnectToWiFi(void)
{
  const char* ssid = CONFIG_SSID;
  const char* password = CONFIG_PASSWORD;

  dbg.print("connecting to ");
  dbg.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    dbg.print(".");
  }
  dbg.println("\nWiFi connected");
}


void ConnectToNoaa (
  WiFiClientSecure& client,
  int& hour,
  int& minute
)
{
  const char* host = "tidesandcurrents.noaa.gov";
  const int httpsPort = 443;
  static const char noaa_rootCA_cert[] PROGMEM= 
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n" ;

  client.setCACert(noaa_rootCA_cert);

  if (!client.connect(host, httpsPort)) {
    dbg.println("connection failed");
    return;
  }

  const char url[] = "/api/datagetter?product=predictions&application=Custom&date=today&datum=MLLW&station=" CONFIG_NOAA_STATION "&time_zone=lst_ldt&units=english&interval=hilo&format=csv";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: " CONFIG_USER_AGENT "\r\n" +
               "Connection: close\r\n\r\n");
  /* 
    Returned text looks like:
    Date Time, Prediction, Type
    2018-12-25 00:54,-3.304,L
    2018-12-25 08:06,16.223,H
    2018-12-25 13:54,7.533,L
    2018-12-25 18:35,13.596,H  
  */

  while (client.connected()) {
    const int CCH = 128;
    char rgch[CCH];

    client.readBytesUntil('\n', rgch, CCH);
    if (rgch[0] == '\r') {
      // Done reading headers
      break;
    }
    if (0 == strncmp(rgch, "Date:", 5))
    {
      // Skip 5 spaces to get to the time
      int cSpaces = 0;
      char *pch = rgch + 10;
      while (*pch && cSpaces < 4)
      {
        if (*pch++ == ' ') {
          cSpaces++;
        }
      }
      hour = atoi(pch) + CONFIG_OFFSET_FROM_UTC;
      if (hour < 0)
        hour += 24;
      if (hour > 23)
        hour -= 24;
      // Now find the minute
      while (*pch++ != ':')
        ;
      minute = atoi(pch);
    }
  }

  // Skip "Date Time, Prediction, Type" column headers
  client.readStringUntil('\n');
}



void DrawCentered(
  Paint& paint,
  int y,
  char* rgch,
  sFONT& font)
{
  paint.DrawStringAt((paint.GetWidth() - font.Width * strlen(rgch)) / 2, y, rgch, &font, Color::Black);
}


void setup() 
{
  WiFiClientSecure client;
  Epd epd;
  Paint paint(rgbPaint, 122, 250);
  // Store a value in nonvolatile storage indicating how many TPL5110-induced
  // wakeups should happen before we fetch new data.
  RestartCounter restartcounter;
  
  // the current time (hour only) as returned from the HTTP header
  int hourCurrent; 
  int minuteCurrent; 

  const int X_MARGIN = 2;
  int y = 5;
  sFONT& fontBig = Font20;
  sFONT& fontSmall = Font12;
  const size_t CCH = 30;
  char rgch[CCH];
  
  int32_t restartsRemaining = restartcounter.Get() - 1;
  if (restartsRemaining > 0) {
    // Do nothing except remember the count.
    restartcounter.Set(--restartsRemaining);
  } 
  else 
  {
    // The number of restarts we were waiting for has now happenned,
    // so fetch new data.

    ConnectToWiFi();

    ConnectToNoaa(client, /*out*/hourCurrent, /*out*/minuteCurrent);

    OpenWeatherMapForecastData forecasts[5];
    int forecastCount = GetWeatherForecasts(forecasts, sizeof forecasts / sizeof forecasts[0]);
    
    if (epd.Init(lut_full_update) != 0) {
        goto exit;
    }
    
    paint.Clear(Color::White);

    DrawCentered(paint, y, "Low Tides", Font16);
    y += Font16.Height + 6; 

    while (client.connected()) {    
      long year = client.parseInt();
      client.read(); // '-'
      long month = client.parseInt();
      client.read(); // '-'
      long day = client.parseInt();
      long hour = client.parseInt(); 
      long minute = client.parseInt(); 
      long level = client.parseInt();
      client.readBytesUntil(',', rgch, CCH);
      bool fLow = ('L' == client.read());
      client.readBytesUntil('\n', rgch, CCH);
    
      if (fLow) {
        // Draw time
        snprintf(rgch, CCH, "%i:%02i %cM", (0 == hour % 12) ? 12 : hour % 12, minute, hour < 12 ? 'A' : 'P');
        DrawCentered(paint, y, rgch, fontBig);
        y += fontBig.Height; 

        // Draw level
        snprintf(rgch, CCH, "%i ft.", level);
        DrawCentered(paint, y, rgch, fontSmall);
        y += fontSmall.Height + 8; 
      }
    } 

    paint.DrawHorizontalLine(0, y-3, paint.GetWidth(), Color::Black);
    y += 7;

    // Draw forecasts

    int hourPrev = 0; // Keep track of the previous hour seen..
    bool fTomorrow  = false; // ...so we can see if we roll into tomorrow

    for (int i=0; i < forecastCount; i++)
    {
      time_t time = forecasts[i].observationTime;
      struct tm* timeInfo = gmtime(&time);
      int hour = timeInfo->tm_hour + CONFIG_OFFSET_FROM_UTC;
      hour = (hour + 24) % 24;

      if (!fTomorrow) {
        if (hour < hourPrev) 
          fTomorrow = true;
        else 
          hourPrev = hour; 
      }

      // Draw time
      snprintf(rgch, CCH, "%i %cm %s", 
                (0 == hour % 12) ? 12 : hour % 12, 
                hour < 12 ? 'a' : 'p',
                fTomorrow ? "Tomorrow" : "");
      DrawCentered(paint, y, rgch, fontSmall);
      y += fontSmall.Height;       
      
      // Draw temp and weather
      snprintf(rgch, CCH, "%i  %s", 
                (int)(forecasts[i].temp + 0.5),
                forecasts[i].description.c_str());
      paint.DrawStringAt(X_MARGIN, y, rgch, &fontSmall, Color::Black);
      // Hack to draw a degree symbol using a period
      paint.DrawStringAt(X_MARGIN + fontSmall.Width * (strchr(rgch, ' ') - rgch), y - fontSmall.Height / 2, ".", &fontSmall, Color::Black);

      y += fontSmall.Height + 4; 
    }

#ifdef DEBUG
    snprintf(rgch, CCH, "%i %i:%02i", restartsRemaining, hourCurrent, minuteCurrent);
    sFONT& fontDbg = Font12;
    paint.DrawFilledRectangle(0, 250 - fontDbg.Height, strlen(rgch) * fontDbg.Width + 20, paint.GetHeight(), Color::White);
    paint.DrawStringAt(X_MARGIN, 250 - fontDbg.Height, rgch, &fontDbg, Color::Black);
#endif

    epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();

    // Ordinarily hourCurrent == CONFIG_HOUR_TO_FETCH_DATA, but if we get a little off schedule
    // this will put us back on schedule.
    int hoursToSleep = 24 + CONFIG_HOUR_TO_FETCH_DATA - hourCurrent;
    restartsRemaining = hoursToSleep * 60 / CONFIG_MINUTES_PER_RESTART;
    restartcounter.Set(restartsRemaining);
  } 

exit:

#if CONFIG_USE_TPL5110
  // Tell the TPL5110 we are done.
  pinMode(CONFIG_DONE_PIN, OUTPUT);
  digitalWrite(CONFIG_DONE_PIN, HIGH);
  // No more code should execute.
#else
  esp_sleep_enable_timer_wakeup(15 * 1000000);
  esp_deep_sleep_start();
#endif
}


void loop() 
{
}
