// Your timezone offset from UTC (GMT)
#define CONFIG_OFFSET_FROM_UTC (-8)

// WiFi
#define CONFIG_SSID "Fuller"
#define CONFIG_PASSWORD "xxx"

// HTTP user-agent string
#define CONFIG_USER_AGENT "jasonful@hotmail.com"

// Look up the nearest NOOA station ID to you at https://tidesandcurrents.noaa.gov/tide_predictions.html
#define CONFIG_NOAA_STATION "9446671"

// 4:00 AM
#define HOUR_TO_FETCH_NEW_DATA 4

// This is the number of minutes between wakeups of the TPL5110
// as determined by the resistor connected its "Delay" pin.
#define CONFIG_MINUTES_PER_RESTART 120

// The GPIO pin connected to the TPL5110's "Done"
#define CONFIG_DONE_PIN 13

// See https://docs.thingpulse.com/how-tos/openweathermap-key/
#define OPEN_WEATHER_MAP_APP_ID "46a94823a9a46c33f6934e6c4c5be17f"

/*
Go to https://openweathermap.org/find?q= and search for a location. Go through the
result set and select the entry closest to the actual location you want to display 
data for. It'll be a URL like https://openweathermap.org/city/2657896. The number
at the end is what you assign to the constant below.
 */
#define OPEN_WEATHER_MAP_LOCATION_ID "5801595" // Longbranch, WA, US

/*
Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
*/
#define OPEN_WEATHER_MAP_LANGUAGE "en"

// Weather units
#define IS_METRIC false

// Define the range of hours in the day you want forecasts for.
// Note OpenWeatherMap only actually gives a forecast for every 3 hours.
#define CONFIG_ALLOWED_HOURS_START 6 // 6 AM
#define CONFIG_ALLOWED_HOURS_END 22  // 10 PM

// The code is designed for use with a TPL5110 chip to ensure low 
// power usage.  But if you don't care about power, or for testing
// purposes, you can use deep sleep mode instead.
#define USE_TPL5110 0