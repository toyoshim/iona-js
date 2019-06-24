// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "Arduino.h"

SerialLibrary Serial;

namespace {

char U4ToHex(uint8_t val) {
  if (val < 10)
    return '0' + val;
  return 'a' + val - 10;
}

void delayU8x1usec(uint8_t x1us) {
  // At 16MHz, 1us = 16tau (PROTO: 8MHz, 1us = 8tau)
  asm (
      "mov r18, %0\n"  // 1t
      "1:\n"           // LOOP 1: 16t (PROTO: 8t)
#if defined(PROTO)
      "ldi r19, 1\n"   //  1t
      "nop\n"          //  1t
#else
      "ldi r19, 4\n"   //  1t
#endif
      "2:\n"           //  LOOP 2: 11t (PROTO: 2t)
      "dec r19\n"      //   1t
      "brne 2b\n"      //   2t (1t for not taken)
      "nop\n"          // 1t
      "dec r18\n"      // 1t
      "brne 1b\n"      // 2t (1t for not taken)
  :: "r" (x1us)
  : "r18", "r19");
}

}  // namespace

// Use PD1(TXD) for serial debug logging.
SerialLibrary::SerialLibrary() {
  // U2X, 117.647Kbps [+2.1%] (PROTO: 111.111Kbps [-5.6%])
  UBRRH = 0;
#if defined(PROTO)
  UBRRL = 8;
#else
  UBRRL = 16;
#endif
  UCSRA = 0x02;
  // TX enabled
  UCSRB = (UCSRB & 0x90) | 0x08;
  // 8-bits, non-parity, 1 stop-bit
  UCSRC = 0x86;

  // Output, High
  PORTD |= 0x02;
  DDRD |= 0x02;
}

void SerialLibrary::print(uint8_t val) {
#if !defined(PROTO)
  while (!(UCSRA & (1 << UDRE)));
  UDR = val;
#endif
}

void SerialLibrary::print(uint8_t val, enum Type type) {
  if (type == BIN) {
    for (int i = 0x80; i; i >>= 1)
      print((val & i) ? '1' : '0');
  } else if (type == HEX) {
    char hex[3];
    int i = 0;
    if (16 <= val)
      hex[i++] = U4ToHex(val >> 4);
    hex[i++] = U4ToHex(val & 0x0f);
    hex[i++] = 0;
    print(hex);
  }
}

void SerialLibrary::print(const char* val) {
  while (*val)
    print(*val++);
}

void SerialLibrary::println(const char* val) {
  print(val);
  print("\r\n");
}

void delayMicroseconds(uint32_t us) {
  if (us > 255) {
    Serial.println("delayMicroseconds > 255: not supported");
    return;
  }
  delayU8x1usec(us & 0xff);
}
