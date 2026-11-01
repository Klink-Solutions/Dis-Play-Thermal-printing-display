# Weather receipt printer - firmware prototype

XIAO ESP32-C3 -> 58 mm TTL thermal printer. The ESP joins WiFi, fetches weather from Open-Meteo
(free, no API key), draws the receipt into a 384x~626 px 1-bit canvas (Adafruit GFX) and streams it
to the printer as an ESC/POS raster image (`GS v 0`). The layout is a port of `designer/app.js`.

## Arduino IDE setup
1. Boards Manager: install **esp32 by Espressif**. Select board **XIAO_ESP32C3**, set **USB CDC On Boot: Enabled**.
2. Library Manager: install **Adafruit GFX Library** (accept "Install all" so Adafruit BusIO comes too)
   and **ArduinoJson** by Benoit Blanchon (version 7.x).
3. Open `weather_printer/weather_printer.ino` (the folder name must stay `weather_printer`).
4. Edit `config.h`: WiFi name/password, and `PRINT_ON_BOOT` etc.
5. Upload (hold BOOT while plugging in if the port doesn't appear). Serial monitor at 115200:
   `s` prints a test strip (no WiFi needed), `p` prints the weather receipt.

## Wiring
| XIAO ESP32-C3 | Printer / part |
|---|---|
| GPIO21 (D6, TX) | printer RXD (TTL connector pin 3) |
| GPIO20 (D7, RX) | printer TXD (TTL connector pin 2) |
| GND | printer GND (must be common) |
| GPIO2 (D0) | NeoPixel DIN via ~330 ohm |
Printer power comes from a separate 5 V supply (see earlier notes); do not draw it through the XIAO.

## First-run checklist
1. Printer self-test page (hold FEED while powering on): confirm baud rate; set `PRINTER_BAUD`.
2. Run `s` (test strip). Garbage or nothing = baud, TX/RX swapped, or missing common ground.
3. Run `p`.

## Known limitations / things to check on real hardware
- **Slow at 9600 baud**: a full receipt is ~30 KB = about 32 s, and the head may pause between bands
  (faint lines). Raise the printer's baud rate if the unit allows it; then set `PRINTER_BAUD` and
  `BAND_DELAY_MS`. The CSN-A2 manual has no baud-setting command; the QR701 may differ.
- **Boot noise**: GPIO21/20 are the chip's default UART0 pins, so the ROM boot log (115200 baud) may
  print a few junk characters on every reset. If that bothers you, move the printer to other pins.
- **GPIO2 is a boot strapping pin**: keep the NeoPixel line free of strong pull-downs.
- `client.setInsecure()` skips TLS certificate checks (prototype only).
- Text is ASCII-only (the GFX fonts have no umlauts or degree sign; the degree ring is drawn by hand).
- The weather parsing and the print stream were tested on a PC against a *sample* Open-Meteo response;
  the live API call, WiFi, LED and printer output have not been run on real hardware.

## Preview without hardware
`host_test/` compiles the same drawing code on a PC (needs g++ and a clone of Adafruit-GFX-Library).
`host_test/sketch_preview.png` is what the printer receives for the sample data.
