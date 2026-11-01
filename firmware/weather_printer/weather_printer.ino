// Weather receipt printer - prototype
// XIAO ESP32-C3 + 58 mm TTL thermal printer (ESC/POS) + 1 NeoPixel status LED.
//
// Libraries (Arduino Library Manager): "Adafruit GFX Library" (also install Adafruit BusIO when
// asked) and "ArduinoJson" by Benoit Blanchon (v7). Board: XIAO_ESP32C3, USB CDC On Boot: Enabled.
//
// Flow: connect WiFi -> sync clock (NTP) -> fetch weather -> draw into a 384 px 1-bit canvas ->
// stream it to the printer as an ESC/POS raster image.
// Serial monitor (115200): send 'p' to print now, 's' to print a test strip.
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "receipt.h"
#include "escpos.h"

static GFXcanvas1 *canvasPtr = nullptr;  // ~31 KB, allocated in setup()
#define canvas (*canvasPtr)
static unsigned long lastAutoPrint = 0;
static bool lastButton = true;

// ---- status LED -----------------------------------------------------------------------------
enum LedState { LED_OFF, LED_BOOT, LED_BUSY, LED_OK, LED_ERROR };
static void led(LedState s) {
  uint8_t r = 0, g = 0, b = 0;
  switch (s) {
    case LED_BOOT:  b = NEO_BRIGHTNESS; break;                       // blue   : connecting
    case LED_BUSY:  r = NEO_BRIGHTNESS; g = NEO_BRIGHTNESS / 2; break; // amber  : fetching/printing
    case LED_OK:    g = NEO_BRIGHTNESS; break;                       // green  : ready / done
    case LED_ERROR: r = NEO_BRIGHTNESS; break;                       // red    : failed
    default: break;
  }
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(LED_PIN, r, g, b);
#else
  neopixelWrite(LED_PIN, r, g, b);
#endif
}

// ---- weather ---------------------------------------------------------------------------------
static int iconForWmo(int code) {  // WMO weather code -> icon
  if (code <= 1) return ICON_CLEAR;
  if (code == 2) return ICON_PARTLY;
  if (code == 3) return ICON_CLOUDS;
  if (code == 45 || code == 48) return ICON_MIST;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return ICON_RAIN;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return ICON_SNOW;
  if (code >= 95) return ICON_THUNDER;
  return ICON_CLOUDS;
}

static void compassFromDegrees(float deg, char out[3]) {
  static const char *const names[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  strcpy(out, names[((int)((deg + 22.5f) / 45.0f)) % 8]);
}

static bool fetchWeather(WeatherData &w, int hourNow) {
  char url[420];
  snprintf(url, sizeof url,
           "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,"
           "wind_speed_10m,wind_direction_10m"
           "&hourly=temperature_2m,precipitation_probability,weather_code"
           "&daily=temperature_2m_max,temperature_2m_min"
           "&timezone=%s&forecast_days=2",
           LATITUDE, LONGITUDE, TIMEZONE_NAME);

  WiFiClientSecure client;
  client.setInsecure();  // PROTOTYPE ONLY: skips certificate checks. Pin a CA bundle before shipping.
  HTTPClient http;
  http.setTimeout(15000);
  http.useHTTP10(true);  // avoids chunked encoding so the JSON stream parses cleanly
  if (!http.begin(client, url)) return false;
  int status = http.GET();
  if (status != 200) {
    Serial.printf("weather HTTP %d\n", status);
    http.end();
    return false;
  }

  JsonDocument filter;
  filter["current"] = true;
  filter["hourly"]["temperature_2m"] = true;
  filter["hourly"]["precipitation_probability"] = true;
  filter["hourly"]["weather_code"] = true;
  filter["daily"]["temperature_2m_max"] = true;
  filter["daily"]["temperature_2m_min"] = true;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();
  if (err) {
    Serial.printf("weather JSON: %s\n", err.c_str());
    return false;
  }

  memset(&w, 0, sizeof w);
  strlcpy(w.city, CITY_NAME, sizeof w.city);
  JsonObject cur = doc["current"];
  w.temp = cur["temperature_2m"];
  w.feels = cur["apparent_temperature"];
  w.humidity = cur["relative_humidity_2m"];
  w.windKmh = cur["wind_speed_10m"];
  compassFromDegrees(cur["wind_direction_10m"] | 0.0f, w.windDir);
  w.cond = iconForWmo(cur["weather_code"] | 3);
  w.high = doc["daily"]["temperature_2m_max"][0];
  w.low = doc["daily"]["temperature_2m_min"][0];
  w.rainPct = doc["hourly"]["precipitation_probability"][hourNow] | 0;
  for (int i = 0; i < 4; i++) {
    int idx = hourNow + 3 * (i + 1);  // hourly arrays start at 00:00 today and cover 48 h
    w.fcHour[i] = idx % 24;
    w.fcTemp[i] = doc["hourly"]["temperature_2m"][idx];
    w.fcRain[i] = doc["hourly"]["precipitation_probability"][idx] | 0;
    w.fcCond[i] = iconForWmo(doc["hourly"]["weather_code"][idx] | 3);
  }
  return true;
}

// ---- wifi + clock ------------------------------------------------------------------------------
static bool connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) delay(250);
  return WiFi.status() == WL_CONNECTED;
}

static bool syncClock() {
  configTzTime(TZ_POSIX, "pool.ntp.org", "time.google.com");
  struct tm t;
  return getLocalTime(&t, 10000);
}

// ---- printing ----------------------------------------------------------------------------------
static void printerSetup() {
  Serial1.begin(PRINTER_BAUD, SERIAL_8N1, PRINTER_RX_PIN, PRINTER_TX_PIN);
  delay(300);
  escposInit(Serial1);
  escposHeating(Serial1, HEAT_N1, 80, 2);
  escposDensity(Serial1, DENSITY);
}

static void printWeatherReceipt() {
  led(LED_BUSY);
  if (!connectWifi()) { Serial.println("WiFi failed"); led(LED_ERROR); return; }
  struct tm t;
  if (!getLocalTime(&t, 2000)) { Serial.println("no time"); led(LED_ERROR); return; }
  WeatherData w;
  if (!fetchWeather(w, t.tm_hour)) { led(LED_ERROR); return; }
  getLocalTime(&t, 2000);  // refresh seconds after the network round trip
  int rows = drawReceipt(canvas, w, t.tm_hour, t.tm_min, t.tm_sec, t.tm_mday, t.tm_mon + 1,
                         t.tm_year + 1900);
  Serial.printf("printing %d rows (%d bytes)\n", rows, rows * (RECEIPT_W / 8));
  escposInit(Serial1);
  escposHeating(Serial1, HEAT_N1, 80, 2);
  escposDensity(Serial1, DENSITY);
  escposRaster(Serial1, canvas.getBuffer(), RECEIPT_W / 8, rows, BAND_ROWS, BAND_DELAY_MS);
  escposFeed(Serial1, FEED_LINES);
  Serial1.flush();
  led(LED_OK);
}

// Hardware smoke test: a plain text line, then a striped bitmap. No WiFi needed.
static void printTestStrip() {
  escposInit(Serial1);
  Serial1.println("PRINTER TEST");
  canvas.fillScreen(0);
  for (int x = 0; x < RECEIPT_W; x += 16) canvas.fillRect(x, 0, 8, 40, 1);
  canvas.drawRect(0, 0, RECEIPT_W, 40, 1);
  escposRaster(Serial1, canvas.getBuffer(), RECEIPT_W / 8, 40, BAND_ROWS, BAND_DELAY_MS);
  escposFeed(Serial1, FEED_LINES);
}

// ---- Arduino entry points ------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  canvasPtr = new GFXcanvas1(RECEIPT_W, RECEIPT_MAX_H);
  led(LED_BOOT);
  printerSetup();
  if (BUTTON_PIN >= 0) pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!connectWifi()) { Serial.println("WiFi failed - check config.h"); led(LED_ERROR); return; }
  if (!syncClock()) { Serial.println("NTP failed"); led(LED_ERROR); return; }
  Serial.println("ready: 'p' = print weather, 's' = test strip");
  led(LED_OK);
  if (PRINT_ON_BOOT) printWeatherReceipt();
  lastAutoPrint = millis();
}

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'p') printWeatherReceipt();
    if (ch == 's') printTestStrip();
  }
  if (BUTTON_PIN >= 0) {
    bool pressed = digitalRead(BUTTON_PIN) == LOW;
    if (pressed && lastButton) { delay(30); if (digitalRead(BUTTON_PIN) == LOW) printWeatherReceipt(); }
    lastButton = !pressed;
  }
  if (PRINT_INTERVAL_MIN > 0 && millis() - lastAutoPrint > (unsigned long)PRINT_INTERVAL_MIN * 60000UL) {
    lastAutoPrint = millis();
    printWeatherReceipt();
  }
  delay(20);
}
