# Printer reference notes

Source: `printer-manual-CSN-A2.pdf` (Cashino CSN-A2 user manual, v4.0, 70 pages).
The printer being bought is a GOOJPRT QR701, a *different* model. The manual is a
stand-in because these modules usually share the same ESC/POS firmware family.
Verify everything below on the real unit with its self-test page (hold FEED while
powering on) before relying on it.

## Hardware facts (manual pp. 4, 7-10)
- Paper 58 mm, print width 48 mm = 384 dots/line (8 dots/mm, 203 dpi). Max roll diameter 39 mm.
- Input power (VH): DC 5-9 V. Speed 60 mm/s; max 80 mm/s at 8.5 V. Higher voltage = faster and darker.
- TTL connector (CN4, 4-pin PH2.0): 1 = +3.3 V, 2 = TXD (printer out), 3 = RXD (printer in), 4 = GND.
  The logic level is 3.3 V, so it connects straight to an ESP32 with no divider.
  Power is a separate connector (J3: VH, GND).
- 6-pin variant (TTL/RS232 header): 1 GND, 2 NULL, 3 VH, 4 GND, 5 RXD, 6 TXD.
- Default baud rate for this family is 9600. Confirm on the self-test page.
- Status LED blinks: 1 = OK, 2 = mechanism/temperature low, 3 = no paper, 5 = overheated.
- Print head must stay under 65 C: avoid long continuous solid-black printing (p. 11).

## Commands we will use (decimal)
| Purpose | Command | Notes |
|---|---|---|
| Initialize | `27 64` (ESC @) | Send first, every job |
| Print + feed n lines | `27 100 n` (ESC d) | Feed at the end so the text clears the tear bar |
| Align | `27 97 n` (ESC a) | 0 left, 1 center, 2 right |
| Print mode | `27 33 n` (ESC !) | bit0 font B, bit3 bold, bit4 double-height, bit5 double-width |
| Char size | `29 33 n` (GS !) | up to 8x each way |
| Line spacing | `27 51 n` (ESC 3) | n x 0.125 mm, default 30 |
| Code page | `27 116 n` (ESC t) | 0 = CP437, 2 = CP850, 16 = WCP1252, 19 = CP858 (has the euro sign) |
| Raster image | `29 118 48 m xL xH yL yH data` (GS v 0) | see below |
| Density | `18 35 n` (DC2 #) | low 5 bits: density 50% + 5% x n; top 3 bits: break time |
| Heating params | `27 55 n1 n2 n3` (ESC 7) | see power notes |
| Paper/status | `27 118 n` (ESC v) / `29 114 1` (GS r) | printer replies with 1 byte |
| Test page | `18 84` (DC2 T) | |
| Barcode | `29 107 ...` (GS k) | CODE128 etc. (pp. 42-47) |

German text note: the default code page is CP437. Umlauts and sharp-s print
correctly on CP437/CP850/WCP1252 if you convert the UTF-8 text first.

## Custom graphics (for weather, transit, logos)
Use GS v 0 (manual p. 33):
- Width: 384 px max = 48 bytes per row. xL = 48, xH = 0.
- Height: any number of rows, yL + yH x 256 (max 4095 per command).
- Each byte holds 8 horizontal pixels, MSB on the left; bit 1 = black dot.
- Pipeline: render image -> grayscale -> dither (Floyd-Steinberg) -> 1-bit -> pack bytes -> send in chunks.
- Large images should be sent in bands of about 24-64 rows with a short pause between bands.
- NV (flash) logo: FS q defines, FS p prints (p. 35-38). Flash wear: write at most about 10 times a day.

## Power notes (manual p. 48)
`ESC 7 n1 n2 n3`: n1 = max simultaneously heated dots / 8 minus 1 (default 7 = 64 dots),
n2 = heating time (x10 us, default 80), n3 = heating interval (x10 us, default 2).
Lower n1 = lower peak current but slower on dark lines. It is the printer's own
brown-out control and can be set in firmware at startup.
`ESC v` status bit 3 flags supply voltage above 9.5 V; bit 6 flags head temperature above 60 C.
