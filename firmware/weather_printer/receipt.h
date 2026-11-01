// receipt.h - draws the weather receipt into a 384-px-wide 1-bit canvas.
// Layout is a port of exportReceiptPng() in designer/app.js.
// Pure drawing code (no WiFi / printer), so it also compiles on a PC for preview.
#pragma once
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <stdio.h>
#include <string.h>
#include "icons.h"

#define RECEIPT_W 384          // 48 mm printable width at 203 dpi
#define RECEIPT_MAX_H 640      // canvas height; the real height is returned by drawReceipt()

enum WeatherIcon { ICON_CLEAR = 0, ICON_PARTLY, ICON_CLOUDS, ICON_RAIN, ICON_SNOW, ICON_THUNDER, ICON_MIST };

static const char *const ICON_NAMES[] = {"Clear",  "Partly Cloudy", "Clouds", "Rain",
                                         "Snow",   "Thunderstorm",  "Mist"};

struct WeatherData {
  char city[32];
  int cond;            // WeatherIcon
  float temp, high, low, feels;
  int rainPct, humidity;
  float windKmh;
  char windDir[3];
  // next four 3-hour steps
  int fcHour[4];
  int fcCond[4];
  float fcTemp[4];
  int fcRain[4];
};

// Black = 1 (matches ESC/POS raster, where a set bit prints a dot).
#define BLACK 1

// --- small helpers -----------------------------------------------------------------------
static void drawIconScaled(Adafruit_GFX &c, int icon, int x, int y, int scale) {
  for (int row = 0; row < 16; row++) {
    uint16_t bits = (pgm_read_byte(&WEATHER_ICONS[icon][row * 2]) << 8) |
                    pgm_read_byte(&WEATHER_ICONS[icon][row * 2 + 1]);
    for (int col = 0; col < 16; col++)
      if (bits & (0x8000 >> col)) c.fillRect(x + col * scale, y + row * scale, scale, scale, BLACK);
  }
}

static void rule(Adafruit_GFX &c, int y, int thickness = 2) {
  for (int i = 0; i < thickness; i++) c.drawFastHLine(18, y + i, RECEIPT_W - 36, BLACK);
}

static int textWidth(Adafruit_GFX &c, const char *s) {
  int16_t x1, y1;
  uint16_t w, h;
  c.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return (int)w + x1;  // advance incl. left bearing
}

static void textAt(Adafruit_GFX &c, const GFXfont *f, int x, int baseline, const char *s) {
  c.setFont(f);
  c.setTextColor(BLACK);
  c.setCursor(x, baseline);
  c.print(s);
}

static void textCentered(Adafruit_GFX &c, const GFXfont *f, int cx, int baseline, const char *s) {
  c.setFont(f);
  textAt(c, f, cx - textWidth(c, s) / 2, baseline, s);
}

static void textRight(Adafruit_GFX &c, const GFXfont *f, int rx, int baseline, const char *s) {
  c.setFont(f);
  textAt(c, f, rx - textWidth(c, s), baseline, s);
}

// The GFX fonts only contain ASCII, so the degree sign is drawn as a small ring.
// Draws "<value><ring>C" and returns its total width. align: 0 = left at x, 1 = centered on x.
static int drawTemp(Adafruit_GFX &c, const GFXfont *f, int x, int baseline, float v, int align,
                    int ringR) {
  char num[12];
  snprintf(num, sizeof num, "%.1f", v);
  c.setFont(f);
  int wNum = textWidth(c, num);
  int wC = textWidth(c, "C");
  int ringW = ringR * 2 + 4;
  int total = wNum + ringW + wC;
  int x0 = align == 1 ? x - total / 2 : x;
  textAt(c, f, x0, baseline, num);
  // ring sits at the top-right of the digits (cap height ~ ringR * 3.5)
  int capH = (f == &FreeSansBold24pt7b) ? 34 : 13;
  int cy = baseline - capH + ringR + 1;
  int cx = x0 + wNum + 2 + ringR;
  c.drawCircle(cx, cy, ringR, BLACK);
  if (ringR >= 4) c.drawCircle(cx, cy, ringR - 1, BLACK);
  textAt(c, f, x0 + wNum + ringW, baseline, "C");
  return total;
}

static void metric(Adafruit_GFX &c, int cx, int top, const char *label, const char *value) {
  textCentered(c, &FreeSansBold9pt7b, cx, top + 17, label);
  textCentered(c, &FreeSans9pt7b, cx, top + 40, value);
}

// --- the receipt ---------------------------------------------------------------------------
// Returns the number of rows actually used (print only this many).
static int drawReceipt(GFXcanvas1 &c, const WeatherData &w, int hh, int mm, int ss, int dd, int mo,
                       int yyyy) {
  char buf[40];
  c.fillScreen(0);

  // Clock
  snprintf(buf, sizeof buf, "%02d:%02d:%02d", hh, mm, ss);
  textCentered(c, &FreeMonoBold24pt7b, RECEIPT_W / 2, 58, buf);

  // Location + date
  textAt(c, &FreeSansBold9pt7b, 18, 90, w.city);
  snprintf(buf, sizeof buf, "%02d.%02d.%04d", dd, mo, yyyy);
  textRight(c, &FreeSans9pt7b, 366, 90, buf);
  rule(c, 105);

  // Current conditions: big icon + temperature + high/low
  drawIconScaled(c, w.cond, 20, 118, 8);
  drawTemp(c, &FreeSansBold24pt7b, 160, 176, w.temp, 0, 5);
  textAt(c, &FreeSans9pt7b, 162, 200, ICON_NAMES[w.cond]);
  textAt(c, &FreeSansBold9pt7b, 162, 226, "HIGH");
  textAt(c, &FreeSansBold9pt7b, 263, 226, "LOW");
  drawTemp(c, &FreeSans9pt7b, 162, 246, w.high, 0, 3);
  drawTemp(c, &FreeSans9pt7b, 263, 246, w.low, 0, 3);
  rule(c, 262);

  // Details, 2 x 2 grid
  int top1 = 270, top2 = 322;
  int cxL = 18 + (RECEIPT_W - 36) / 4, cxR = 18 + 3 * (RECEIPT_W - 36) / 4;
  textCentered(c, &FreeSansBold9pt7b, cxL, top1 + 17, "Feels like");
  drawTemp(c, &FreeSans9pt7b, cxL, top1 + 40, w.feels, 1, 3);
  snprintf(buf, sizeof buf, "%d%%", w.rainPct);
  metric(c, cxR, top1, "Rain", buf);
  snprintf(buf, sizeof buf, "%d%%", w.humidity);
  metric(c, cxL, top2, "Humidity", buf);
  snprintf(buf, sizeof buf, "%.1f km/h %s", w.windKmh, w.windDir);
  metric(c, cxR, top2, "Wind", buf);
  rule(c, 376);

  // Forecast
  textAt(c, &FreeSansBold9pt7b, 18, 402, "NEXT 12 HOURS - 3H STEPS");
  int y = 412;
  for (int i = 0; i < 4; i++) {
    c.drawFastHLine(18, y, RECEIPT_W - 36, BLACK);  // thin separator
    snprintf(buf, sizeof buf, "%02d:00", w.fcHour[i]);
    textAt(c, &FreeSansBold9pt7b, 24, y + 27, buf);
    drawIconScaled(c, w.fcCond[i], 96, y + 4, 2);
    drawTemp(c, &FreeSans9pt7b, 220, y + 27, w.fcTemp[i], 1, 3);
    snprintf(buf, sizeof buf, "%d%%", w.fcRain[i]);
    textCentered(c, &FreeSans9pt7b, 320, y + 27, buf);
    y += 40;
  }
  c.drawFastHLine(18, y, RECEIPT_W - 36, BLACK);
  return y + 2;  // footer banner removed for now
}
