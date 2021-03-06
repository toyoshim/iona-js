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
#if defined(NO_DEBUG)
  DDRD &= ~0x12;
  PORTD |= 0x12;
#else
  DDRD &= ~0x10;
  PORTD |= 0x10;
#endif
  Initialize();
}

void Jamma::Initialize() {
  for (uint8_t i = 0; i < 2; ++i) {
    current_coin_sw[i] = 1;
    coin_sw[i] = 1;
    coin_count[i] = 0;
  }
  for (uint8_t i = 0; i < 5; ++i)
    sw[i] = 0;
}

void Jamma::DetectMode() {
  mode = (PINA & 0x30) ? Mode::kJamma : Mode::kMahjong;
  if (mode == Mode::kJamma) {
    DDRC = 0x00;
    PORTC = 0xff;
  } else {
    // Hi-Z w/o PU for bit7-4.
    DDRC = 0x00;
    PORTC = 0x0f;
  }
}

Jamma::Mode Jamma::GetMode() {
  return mode;
}

void Jamma::Update(bool swap) {
  uint8_t pina = PINA;
  uint8_t pinb = PINB;
  uint8_t pinc = PINC;
  uint8_t pind = PIND;

  uint8_t csw1 = pinb & 1;
  uint8_t csw2 = pinc & 1;
  current_coin_sw[0] &= csw1;
  current_coin_sw[1] &= csw2;

  sw[0] = (pind & 0x10) ? 0 : 0x80;
  sw[1] =
      ((pinb & 0x02) ? 0 : 0x80) |
      ((pind & 0x02) ? 0 : 0x40) |
      ((pina & 0x01) ? 0 : 0x20) |
      ((pina & 0x02) ? 0 : 0x10) |
      ((pina & 0x04) ? 0 : 0x08) |
      ((pina & 0x08) ? 0 : 0x04);

  if (swap) {
    sw[1] |=
#if defined(ALT_SWAP)
        ((pinb & 0x08) ? 0 : 0x02) |  // B2 -> B1
        ((pinb & 0x10) ? 0 : 0x01);   // B3 -> B2
#else
        ((pinb & 0x20) ? 0 : 0x02) |  // B4 -> B1
        ((pinb & 0x04) ? 0 : 0x01);   // B1 -> B2
#endif
  } else {
    sw[1] |=
        ((pinb & 0x04) ? 0 : 0x02) |  // B1
        ((pinb & 0x08) ? 0 : 0x01);   // B2
  }

  if (mode == Mode::kMahjong) {
    sw[2] = sw[3] = sw[4] = 0;
    return;
  }

  sw[2] =
      ((pinb & 0x40) ? 0 : 0x20) |    // B5
      ((pinb & 0x80) ? 0 : 0x10);     // B6
  if (swap) {
    sw[2] |=
#if defined(ALT_SWAP)
        ((pinb & 0x20) ? 0 : 0x80) |  // B4 -> B3
        ((pinb & 0x04) ? 0 : 0x40);   // B1 -> B4
#else
        ((pinb & 0x08) ? 0 : 0x80) |  // B2 -> B3
        ((pinb & 0x10) ? 0 : 0x40);   // B3 -> B4
#endif
  } else {
    sw[2] |=
        ((pinb & 0x10) ? 0 : 0x80) |  // B3
        ((pinb & 0x20) ? 0 : 0x40);   // B4
  }
  sw[3] =
      ((pinc & 0x02) ? 0 : 0x80) |
      ((pina & 0x10) ? 0 : 0x20) |
      ((pina & 0x20) ? 0 : 0x10) |
      ((pina & 0x40) ? 0 : 0x08) |
      ((pina & 0x80) ? 0 : 0x04);
  if (swap) {
    sw[3] |=
#if defined(ALT_SWAP)
        ((pinc & 0x08) ? 0 : 0x02) |  // B2 -> B1
        ((pinc & 0x10) ? 0 : 0x01);   // B3 -> B2
#else
        ((pinc & 0x20) ? 0 : 0x02) |  // B4 -> B1
        ((pinc & 0x04) ? 0 : 0x01);   // B1 -> B2
#endif
  } else {
    sw[3] |=
        ((pinc & 0x04) ? 0 : 0x02) |  // B1
        ((pinc & 0x08) ? 0 : 0x01);   // B2
  }
  sw[4] =
      ((pinc & 0x40) ? 0 : 0x20) |    // B5
      ((pinc & 0x80) ? 0 : 0x10);     // B6
  if (swap) {
    sw[4] |=
#if defined(ALT_SWAP)
        ((pinc & 0x20) ? 0 : 0x80) |  // B4 -> B3
        ((pinc & 0x04) ? 0 : 0x40);   // B1 -> B4
#else
        ((pinc & 0x08) ? 0 : 0x80) |  // B2 -> B3
        ((pinc & 0x10) ? 0 : 0x40);   // B3 -> B4
#endif
  } else {
    sw[4] |=
        ((pinc & 0x10) ? 0 : 0x80) |  // B3
        ((pinc & 0x20) ? 0 : 0x40);   // B4
  }
}

void Jamma::Sync() {
  for (uint8_t i = 0; i < 2; ++i) {
    if (!coin_sw[i] && current_coin_sw[i])
      coin_count[i]++;
    coin_sw[i] = current_coin_sw[i];
    current_coin_sw[i] = 1;
  }
}

uint8_t Jamma::GetSw(uint8_t index, bool rapid_mode, bool rapid_mask) {
  uint8_t result = sw[index];
  if (rapid_mode) switch (index) {
    case 0:
      break;
    case 1:
    case 3:
      if (!rapid_mask)
        result |= (sw[index + 1] >> 5) & 0x03;
      break;
    case 2:
    case 4:
      if (!rapid_mask)
        result |= (result << 3) & 0x80;
      break;
  }
  return result;
}

uint8_t Jamma::GetCoin(uint8_t index) {
  return coin_count[index];
}

void Jamma::SubCoin(uint8_t index, uint8_t sub) {
  if (index < 2)
    coin_count[index] -= sub;
}

void Jamma::AddCoin(uint8_t index, uint8_t add) {
  if (index < 2)
    coin_count[index] += add;
}

void Jamma::DriveOutput(uint8_t output) {
  if (mode != Mode::kMahjong)
    return;
  // Output low signal for the activated outputs.
  out = output & 0xf0;
  out = ((out >> 1) | (out << 3)) & 0xf0;
  DDRC = out;
}

