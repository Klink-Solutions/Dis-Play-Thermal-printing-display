// config.h - everything you are likely to change. Don't commit real WiFi passwords.
#pragma once

// ---- WiFi (prototype: hardcoded; later replaced by a WiFiManager captive portal) ----
#define WIFI_SSID "your-wifi-name"
#define WIFI_PASS "your-wifi-password"

// ---- Location + time zone ----
#define CITY_NAME "Munich, Germany"   // ASCII only (the GFX fonts have no umlauts)
#define LATITUDE  48.1351
#define LONGITUDE 11.5820
#define TIMEZONE_NAME "Europe%2FBerlin"                 // URL-encoded, used by the weather API
#define TZ_POSIX      "CET-1CEST,M3.5.0,M10.5.0/3"      // used by the ESP clock (handles DST)

// ---- Pins (XIAO ESP32-C3) ----
#define PRINTER_TX_PIN 21   // ESP TX -> printer RXD   (D6)
#define PRINTER_RX_PIN 20   // ESP RX <- printer TXD   (D7)
#define LED_PIN         2   // NeoPixel data (D0). GPIO2 is a boot-strapping pin: keep a ~330 ohm
                            // series resistor and don't pull it low externally.
#define BUTTON_PIN     -1   // set to a GPIO (button to GND) to print on press; -1 = disabled

// ---- Printer ----
#define PRINTER_BAUD   9600 // QR701/CSN-A2 family default; check the self-test page
#define BAND_ROWS      24   // rows per GS v 0 command
#define BAND_DELAY_MS  0    // raise (e.g. 15) if you raise the baud rate and see garbage
#define HEAT_N1        3    // ESC 7 n1: max dots heated at once = 8*(n1+1). 7 = printer default
                            // (fastest, highest peak current); 3 = half the peak current.
#define DENSITY        0x0A // DC2 # n (0..31): darker = higher. Lower if the head runs hot.
#define FEED_LINES     6    // blank lines after the receipt so it clears the tear bar

// ---- Behaviour ----
#define PRINT_ON_BOOT      1  // 1 = print once after boot (good for testing, wastes paper on resets)
#define PRINT_INTERVAL_MIN 0  // >0 = also print automatically every N minutes
#define NEO_BRIGHTNESS     20 // 0..255, keep low
