// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#if !defined(__Arduino_H__)
# define __Arduino_H__

#include <avr/io.h>

#include <stddef.h>
#include <stdint.h>

// This file provides minimum Arduino compatible interface to run JVSIO.h in a
// similar way how we do on Arduino Nano compatible platforms.

enum Type {
 BIN,
 HEX,
};

enum Mode {
  INPUT = 0,
  OUTPUT = 1
};

extern class SerialLibrary {
 public:
  SerialLibrary();
  void print(uint8_t val);
  void print(uint8_t val, enum Type type);
  void print(const char* val);
  void println(const char* val);
} Serial;

void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);

#endif  // __Arduino_H__
