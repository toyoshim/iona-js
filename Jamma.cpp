// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "Jamma.h"

#include <avr/io.h>

Jamma::Jamma() {
  // Input, pull-up
  DDRA = 0x00;
  DDRB = 0x00;
  DDRC = 0x00;
  PORTA = 0xff;
  PORTB = 0xff;
  PORTC = 0xff;
  Initialize();
}

void Jamma::Initialize() {
  coin_count[0] = 0;
  coin_count[1] = 0;
  for (uint8_t i = 0; i < 4; ++i)
    sw[i] = 0;
}

void Jamma::Update(bool swap) {
  uint8_t pina = PINA;
  uint8_t pinb = PINB;
  uint8_t pinc = PINC;

  sw[0] =
      ((pinb & 0x02) ? 0 : 0x80) |
      ((pina & 0x01) ? 0 : 0x20) |
      ((pina & 0x02) ? 0 : 0x10) |
      ((pina & 0x04) ? 0 : 0x08) |
      ((pina & 0x08) ? 0 : 0x04);
  if (swap) {
    sw[0] |=
        ((pinb & 0x20) ? 0 : 0x02) |
        ((pinb & 0x04) ? 0 : 0x01);
  } else {
    sw[0] |=
        ((pinb & 0x04) ? 0 : 0x02) |
        ((pinb & 0x08) ? 0 : 0x01);
  }
  sw[1] =
      ((pinb & 0x40) ? 0 : 0x20) |
      ((pinb & 0x80) ? 0 : 0x10);
  if (swap) {
    sw[1] |=
        ((pinb & 0x08) ? 0 : 0x80) |
        ((pinb & 0x10) ? 0 : 0x40);
  } else {
    sw[1] |=
        ((pinb & 0x10) ? 0 : 0x80) |
        ((pinb & 0x20) ? 0 : 0x40);
  }
  sw[2] =
      ((pinc & 0x02) ? 0 : 0x80) |
      ((pina & 0x10) ? 0 : 0x20) |
      ((pina & 0x20) ? 0 : 0x10) |
      ((pina & 0x40) ? 0 : 0x08) |
      ((pina & 0x80) ? 0 : 0x04);
  if (swap) {
    sw[2] |=
        ((pinc & 0x20) ? 0 : 0x02) |
        ((pinc & 0x04) ? 0 : 0x01);
  } else {
    sw[2] |=
        ((pinc & 0x04) ? 0 : 0x02) |
        ((pinc & 0x08) ? 0 : 0x01);
  }
  sw[3] =
      ((pinc & 0x40) ? 0 : 0x20) |
      ((pinc & 0x80) ? 0 : 0x10);
  if (swap) {
    sw[3] |=
        ((pinc & 0x08) ? 0 : 0x80) |
        ((pinc & 0x10) ? 0 : 0x40);
  } else {
    sw[3] |=
        ((pinc & 0x10) ? 0 : 0x80) |
        ((pinc & 0x20) ? 0 : 0x40);
  }

  uint8_t csw1 = pinb & 1;
  uint8_t csw2 = pinc & 1;
  if (csw1 && !coin_sw[0])
    coin_count[0]++;
  if (csw2 && !coin_sw[1])
    coin_count[1]++;
  coin_sw[0] = csw1;
  coin_sw[1] = csw2;
}

uint8_t Jamma::GetSw(uint8_t index, bool rapid_mode, bool rapid_mask) {
  uint8_t result = sw[index];
  if (rapid_mode) switch (index) {
   case 0:
   case 2:
    if (!rapid_mask)
      result |= (sw[index + 1] >> 5) & 0x03;
    break;
   case 1:
   case 3:
    if (!rapid_mask)
      result |= (result << 3) & 0x80;
  }
  return result;
}

uint8_t Jamma::GetCoin(uint8_t index) {
  return coin_count[index];
}

void Jamma::SubCoin(int index, uint8_t sub) {
  coin_count[index] -= sub;
}
