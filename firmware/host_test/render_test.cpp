// Renders the receipt on a PC and writes receipt_preview.pbm (1-bit) for inspection.
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "../weather_printer/receipt.h"
int main(int argc, char **argv) {
  GFXcanvas1 canvas(RECEIPT_W, RECEIPT_MAX_H);
  WeatherData w = {};
  strcpy(w.city, "Munich, Germany");
  w.cond = ICON_CLEAR; w.temp = 18.6f; w.high = 21.4f; w.low = 14.2f; w.feels = 17.8f;
  w.rainPct = 20; w.humidity = 61; w.windKmh = 12.4f; strcpy(w.windDir, "S");
  int hours[4] = {13, 16, 19, 22}; int conds[4] = {ICON_PARTLY, ICON_CLOUDS, ICON_RAIN, ICON_CLOUDS};
  float temps[4] = {19.2f, 18.1f, 16.8f, 15.9f}; int rain[4] = {10, 20, 65, 35};
  for (int i = 0; i < 4; i++) { w.fcHour[i] = hours[i]; w.fcCond[i] = conds[i]; w.fcTemp[i] = temps[i]; w.fcRain[i] = rain[i]; }
  if (argc > 1) { w.cond = atoi(argv[1]); w.temp = -3.4f; strcpy(w.windDir, "NW"); w.windKmh = 104.2f; w.humidity = 100; w.rainPct = 100; }
  int rows = drawReceipt(canvas, w, 10, 42, 7, 1, 11, 2026);
  FILE *f = fopen("receipt_preview.pbm", "wb");
  fprintf(f, "P4\n%d %d\n", RECEIPT_W, rows);
  fwrite(canvas.getBuffer(), 1, (RECEIPT_W / 8) * rows, f);  // PBM P4: MSB first, 1 = black
  fclose(f);
  long black = 0; for (int i = 0; i < 48 * rows; i++) black += __builtin_popcount(canvas.getBuffer()[i]);
  printf("rows=%d  black=%.1f%%  bytes=%d\n", rows, 100.0 * black / (RECEIPT_W * rows), 48 * rows);
}
