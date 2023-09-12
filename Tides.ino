#include <WiFiClientSecure.h>
#include <epd2in13.h>
#include <epdif.h>
#include <epdpaint.h>
#include <fonts.h>
#include <SPI.h>
#include "env_config.h"
#include "weatherforecast.h"
#include "restartcounter.h"

#define DEBUG 1 // Set to 1 to turn on debug serial output
#include "dbg.h"
Dbg dbg;

enum Color
{
  Black = 0,
  White = 1
};

const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

unsigned char rgbPaint[16 * 250];

bool ConnectToWiFi(void)
{
  const char *ssid = CONFIG_SSID;
  const char *password = CONFIG_PASSWORD;
  int count = 0;

  dbg.print("connecting to ");
  dbg.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    dbg.print(".");
    if (++count > 60)
      return false;
  }
  dbg.println("\nWiFi connected");
  return true;
}


void ConnectToNoaa(
    WiFiClientSecure &client,
    int &hour,
    int &minute)
{
  const char *host = "api.tidesandcurrents.noaa.gov";
  const int httpsPort = 443;
  static const char noaa_rootCA_cert[] PROGMEM =
      "-----BEGIN CERTIFICATE-----\n"
      "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
      "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
      "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
      "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
      "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
      "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
      "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
      "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
      "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
      "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
      "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
      "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
      "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
      "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
      "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
      "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
      "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
      "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
      "MrY=\n"
      "-----END CERTIFICATE-----\n";

  client.setCACert(noaa_rootCA_cert);

  if (!client.connect(host, httpsPort))
  {
    dbg.println("connection failed");
    return;
  }

  const char url[] = "/api/prod/datagetter?product=predictions&application=Custom&date=today&datum=MLLW&station=" CONFIG_NOAA_STATION "&time_zone=lst_ldt&units=english&interval=hilo&format=csv";

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

  while (client.connected())
  {
    const int CCH = 128;
    char rgch[CCH];

    client.readBytesUntil('\n', rgch, CCH);
    if (rgch[0] == '\r')
    {
      // A blank line indicates the end of the HTTP headers
      break;
    }
    // Get current time from HTTP header
    if (0 == strncmp(rgch, "Date:", 5))
    {
      // Skip 5 spaces to get to the time
      int cSpaces = 0;
      char *pch = rgch + 10;
      while (*pch && cSpaces < 4)
      {
        if (*pch++ == ' ')
        {
          cSpaces++;
        }
      }
      hour = atoi(pch) + CONFIG_OFFSET_FROM_UTC;
      hour = (hour + 24) % 24;

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
    Paint &paint,
    int y,
    char *rgch,
    sFONT &font)
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

  // the current time as returned from the HTTP header
  int hourCurrent;
  int minuteCurrent;

  const int X_MARGIN = 2;
  int y = 5;
  sFONT &fontBig = Font20;
  sFONT &fontSmall = Font12;
  const size_t CCH = 30;
  char rgch[CCH];

  btStop(); // Turn off Bluetooth to save power

  int32_t restartsRemaining = restartcounter.Get();
  restartsRemaining--; // We just restarted, so decrement the count
  dbg.println(restartsRemaining);
  if (restartsRemaining > 0)
  {
    // Do nothing except remember the count.
    restartcounter.Set(restartsRemaining);
  }
  else
  {
    // The number of restarts we were waiting for has now happenned,
    // so fetch new data.

    if (!ConnectToWiFi())
    {
      goto exit;
    }
    ConnectToNoaa(client, /*out*/ hourCurrent, /*out*/ minuteCurrent);

    OpenWeatherMapForecastData forecasts[5];
    int forecastCount = GetWeatherForecasts(forecasts, sizeof forecasts / sizeof forecasts[0]);

    if (epd.Init(lut_full_update) != 0)
    {
      goto exit;
    }

    paint.Clear(Color::White);

    DrawCentered(paint, y, "Low Tides", Font16);
    y += Font16.Height + 6;

    while (client.connected())
    {
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

      if (fLow)
      {
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

    paint.DrawHorizontalLine(0, y - 3, paint.GetWidth(), Color::Black);
    y += 7;

    // Draw forecasts

    int hourPrev = 24; // Keep track of the previous hour seen

    for (int i = 0; i < forecastCount; i++)
    {
      time_t time = forecasts[i].observationTime;
      // convert to local time
      time += CONFIG_OFFSET_FROM_UTC * 3600;
      struct tm *timeInfo = gmtime(&time);
      int hour = timeInfo->tm_hour;

      if (hour < hourPrev) // if start of a new day
      {
        // Print day and time
        snprintf(rgch, CCH, "%s %i, %i %cm",
                 monthNames[timeInfo->tm_mon],
                 timeInfo->tm_mday,
                 (0 == hour % 12) ? 12 : hour % 12,
                 hour < 12 ? 'a' : 'p');
      }
      else
      {
        // Print time
        snprintf(rgch, CCH, "%i %cm",
                 (0 == hour % 12) ? 12 : hour % 12,
                 hour < 12 ? 'a' : 'p');
      }
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

      hourPrev = hour;
    }

    // Ordinarily hourCurrent == CONFIG_HOUR_TO_FETCH_DATA, but if we get a little off schedule
    // this will put us back on schedule.
    int hoursToSleep = 24 + CONFIG_HOUR_TO_FETCH_DATA - hourCurrent;
    restartsRemaining = hoursToSleep * 60 / CONFIG_MINUTES_PER_RESTART;
    restartcounter.Set(restartsRemaining);

#ifdef DEBUG
    snprintf(rgch, CCH, "%i %i:%02i", restartsRemaining, hourCurrent, minuteCurrent);
    sFONT &fontDbg = Font12;
    paint.DrawFilledRectangle(0, 250 - fontDbg.Height, strlen(rgch) * fontDbg.Width + 20, paint.GetHeight(), Color::White);
    paint.DrawStringAt(X_MARGIN, 250 - fontDbg.Height, rgch, &fontDbg, Color::Black);
#endif

    epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
  }

exit:

#if CONFIG_USE_TPL5110
  // Tell the TPL5110 we are done.
  pinMode(CONFIG_DONE_PIN, OUTPUT);
  digitalWrite(CONFIG_DONE_PIN, HIGH);
  // No more code should execute.
#else
  esp_sleep_enable_timer_wakeup(86400000000ull/*1 day*/);
  esp_deep_sleep_start();
#endif
}

void loop()
{
}
