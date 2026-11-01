// escpos.h - minimal ESC/POS driver for the thermal printer (TTL serial, 9600 8N1 by default).
// Commands are taken from the CSN-A2 manual (references/printer-notes.md). Verify on your own unit.
#pragma once
#include <Arduino.h>

// ESC @ : reset printer to defaults. Send once at startup and before each job.
static void escposInit(Print &p) {
  const uint8_t cmd[] = {0x1B, 0x40};
  p.write(cmd, sizeof cmd);
}

// ESC 7 n1 n2 n3 : heating parameters (manual p.48).
//   n1 = max dots heated at once is 8*(n1+1); lower = lower peak current, slower dark areas
//   n2 = heating time (x10 us), n3 = heating interval (x10 us)
static void escposHeating(Print &p, uint8_t n1, uint8_t n2, uint8_t n3) {
  const uint8_t cmd[] = {0x1B, 0x37, n1, n2, n3};
  p.write(cmd, sizeof cmd);
}

// DC2 # n : print density. Low 5 bits = density (0..31), high 3 bits = break time.
static void escposDensity(Print &p, uint8_t n) {
  const uint8_t cmd[] = {0x12, 0x23, n};
  p.write(cmd, sizeof cmd);
}

// ESC d n : print buffer and feed n lines.
static void escposFeed(Print &p, uint8_t lines) {
  const uint8_t cmd[] = {0x1B, 0x64, lines};
  p.write(cmd, sizeof cmd);
}

// GS v 0 : print a raster bitmap, sent in horizontal bands.
//   buf        1 bit per pixel, MSB = leftmost pixel, 1 = black (same layout as GFXcanvas1)
//   widthBytes bytes per row (384 px = 48)
//   rows       number of rows to print
//   bandRows   rows per GS v 0 command (24..64 is a good range)
//   bandDelay  ms to wait after each band so the printer can keep up
static void escposRaster(Print &p, const uint8_t *buf, int widthBytes, int rows, int bandRows,
                         int bandDelayMs) {
  for (int y = 0; y < rows; y += bandRows) {
    int n = min(bandRows, rows - y);
    const uint8_t hdr[] = {0x1D, 0x76, 0x30, 0x00,
                           (uint8_t)(widthBytes & 0xFF), (uint8_t)(widthBytes >> 8),
                           (uint8_t)(n & 0xFF),          (uint8_t)(n >> 8)};
    p.write(hdr, sizeof hdr);
    p.write(buf + (size_t)y * widthBytes, (size_t)n * widthBytes);
    p.flush();
    if (bandDelayMs > 0) delay(bandDelayMs);
  }
}
