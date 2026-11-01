// Sends the receipt through escpos.h into a memory sink, then decodes the byte stream the way a
// printer would and checks the result is identical to the canvas.
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <vector>
#include "../weather_printer/receipt.h"
#include "../weather_printer/escpos.h"
struct Sink : Print { std::vector<uint8_t> v; size_t write(uint8_t b) override { v.push_back(b); return 1; } };
int main() {
  GFXcanvas1 canvas(RECEIPT_W, RECEIPT_MAX_H);
  WeatherData w = {}; strcpy(w.city, "Munich"); w.cond = ICON_RAIN; w.temp = 7.5f; w.windKmh = 3; strcpy(w.windDir, "N");
  for (int i = 0; i < 4; i++) { w.fcHour[i] = 12 + 3 * i; w.fcCond[i] = i; w.fcTemp[i] = i; w.fcRain[i] = 10 * i; }
  int rows = drawReceipt(canvas, w, 1, 2, 3, 4, 5, 2026);
  Sink s; escposInit(s); escposHeating(s, 3, 80, 2); escposDensity(s, 0x0A);
  escposRaster(s, canvas.getBuffer(), 48, rows, 24, 0); escposFeed(s, 5);
  // decode
  std::vector<uint8_t> img; size_t i = 0; int bands = 0;
  while (i < s.v.size()) {
    if (s.v[i] == 0x1B && s.v[i+1] == 0x40) i += 2;
    else if (s.v[i] == 0x1B && s.v[i+1] == 0x37) i += 5;
    else if (s.v[i] == 0x12 && s.v[i+1] == 0x23) i += 3;
    else if (s.v[i] == 0x1B && s.v[i+1] == 0x64) i += 3;
    else if (s.v[i] == 0x1D && s.v[i+1] == 0x76 && s.v[i+2] == 0x30) {
      int wb = s.v[i+4] | s.v[i+5] << 8, n = s.v[i+6] | s.v[i+7] << 8; i += 8;
      img.insert(img.end(), s.v.begin() + i, s.v.begin() + i + wb * n); i += wb * n; bands++;
    } else { printf("UNKNOWN byte %02x at %zu\n", s.v[i], i); return 1; }
  }
  bool same = img.size() == (size_t)48 * rows && memcmp(img.data(), canvas.getBuffer(), img.size()) == 0;
  printf("rows=%d bands=%d stream=%zu bytes (~%.0f s at 9600 baud) roundtrip=%s\n", rows, bands, s.v.size(),
         s.v.size() * 10.0 / 9600, same ? "OK" : "MISMATCH");
  return same ? 0 : 1;
}
