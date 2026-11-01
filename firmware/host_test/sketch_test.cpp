// Compiles the real .ino against stubs and runs setup(): checks syntax, JSON parsing and the output stream.
#include <vector>
#include <cstdarg>
#include <time.h>
#include <Arduino.h>
#include <Print.h>
#define ESP_ARDUINO_VERSION_MAJOR 3
#define SERIAL_8N1 0
#define INPUT_PULLUP 2
#define LOW 0
static std::vector<uint8_t> g_printer;
struct HardwareSerial : Print {
  void begin(...) {}
  size_t write(uint8_t b) override { g_printer.push_back(b); return 1; }
  using Print::write;
  int available() { return 0; }
  int read() { return 0; }
  void printf(const char *f, ...) { va_list a; va_start(a, f); vprintf(f, a); va_end(a); }
  size_t println(const char *s) { printf("[serial] %s\n", s); return 0; }
};
static HardwareSerial Serial, Serial1;
static void pinMode(int, int) {}
static int digitalRead(int) { return 1; }
static unsigned long millis() { return 0; }
static void configTzTime(const char *, const char *, const char *) {}
static bool getLocalTime(struct tm *t, uint32_t = 0) { memset(t, 0, sizeof *t); t->tm_hour = 10; t->tm_min = 42; t->tm_sec = 7; t->tm_mday = 1; t->tm_mon = 10; t->tm_year = 126; return true; }
static void rgbLedWrite(int pin, int r, int g, int b) { printf("[led] pin %d rgb %d,%d,%d\n", pin, r, g, b); }
#include "../weather_printer/weather_printer.ino"
int main() {
  setup(); loop();
  // decode printer stream -> image
  std::vector<uint8_t> img; size_t i = 0;
  auto &v = g_printer;
  while (i < v.size()) {
    if (v[i] == 0x1B && v[i+1] == 0x40) i += 2;
    else if (v[i] == 0x1B && v[i+1] == 0x37) { printf("ESC 7 n1=%d n2=%d n3=%d\n", v[i+2], v[i+3], v[i+4]); i += 5; }
    else if (v[i] == 0x12 && v[i+1] == 0x23) { printf("density=0x%02X\n", v[i+2]); i += 3; }
    else if (v[i] == 0x1B && v[i+1] == 0x64) { printf("feed %d\n", v[i+2]); i += 3; }
    else if (v[i] == 0x1D && v[i+1] == 0x76 && v[i+2] == 0x30) {
      int wb = v[i+4] | v[i+5] << 8, n = v[i+6] | v[i+7] << 8; i += 8;
      img.insert(img.end(), v.begin() + i, v.begin() + i + wb * n); i += wb * n;
    } else { printf("UNKNOWN %02x at %zu\n", v[i], i); return 1; }
  }
  int rows = img.size() / 48;
  FILE *f = fopen("sketch_preview.pbm", "wb"); fprintf(f, "P4\n384 %d\n", rows); fwrite(img.data(), 1, img.size(), f); fclose(f);
  printf("decoded %d rows from %zu bytes\n", rows, v.size());
}
