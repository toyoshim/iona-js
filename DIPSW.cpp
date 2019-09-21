// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <avr/io.h>

#include "Arduino.h"
#include "DIPSW.h"

DIPSW::DIPSW() {
  DDRD &= ~0x78;
  PORTD |= 0x78;
  Update();
}

void DIPSW::Update() {
  uint8_t old = dipsw;
  dipsw = Peek();
  if (old == dipsw)
    return;
  Serial.print("DIPSW: ");
  Serial.print(dipsw, BIN);
  Serial.println("");
  rapid = dipsw & 1;
  switch (dipsw & 0x0c) {
   case 0x00:  // __00 - 12
    rapid_mod = 5;
    rapid_th = 2;
    break;
   case 0x08:  // __01 - 15
    rapid_mod = 4;
    rapid_th = 2;
    break;
   case 0x04:  // __10 - 20
    rapid_mod = 3;
    rapid_th = 1;
    break;
   case 0x0c:  // __11 - 30
    rapid_mod = 2;
    rapid_th = 1;
    break;
  }
#if !defined(NO_SWAP)
  swap = dipsw & 2;
#endif
}

void DIPSW::Sync() {
  if (!rapid)
    return;
  rapid_count++;
  if (rapid_count == rapid_mod)
    rapid_count = 0;
}

bool DIPSW::GetRapidMode() {
  return rapid;
}

bool DIPSW::GetRapidMask() {
  return rapid_count < rapid_th;
}

bool DIPSW::GetSwapMode() {
  return swap;
}

uint8_t DIPSW::Peek() {
  return ~(PIND >> 3) & 0x0f;
}
