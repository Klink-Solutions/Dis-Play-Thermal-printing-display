// Minimal stand-in for Arduino.h so Adafruit_GFX compiles on a PC.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define PROGMEM
#define pgm_read_byte(a) (*(const unsigned char *)(a))
#define pgm_read_word(a) (*(const unsigned short *)(a))
#define pgm_read_dword(a) (*(const unsigned long *)(a))
#define F(x) x
typedef bool boolean;
typedef uint8_t byte;
#include <algorithm>
using std::min;
using std::max;
#define _swap_int16_t(a,b) { int16_t t = a; a = b; b = t; }
#define radians(deg) ((deg) * 0.017453292519943295)
class __FlashStringHelper;
class String {
 public:
  String(const char *s = "") : s_(s) {}
  unsigned length() const { return strlen(s_); }
  char charAt(unsigned i) const { return s_[i]; }
  const char *c_str() const { return s_; }
 private:
  const char *s_;
};
static inline void delay(unsigned long) {}
