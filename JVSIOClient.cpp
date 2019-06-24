// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <avr/interrupt.h>

#include "Arduino.h"
#include "JVSIOClient.h"

namespace {

constexpr uint8_t rx_buf_size = 128;
uint8_t rx_buf[rx_buf_size];
volatile uint8_t rx_wt_ptr = 0;
volatile uint8_t rx_rd_ptr = 0;
volatile uint8_t rx_size = 0;

ISR(USART_RXC_vect) {
  if (UCSRA & (1 << UPE))
    return;
  uint8_t data = UDR;
  if (rx_size != (rx_buf_size - 1)) {
    rx_buf[rx_wt_ptr] = data;
    rx_wt_ptr = (rx_wt_ptr + 1) % rx_buf_size;
    rx_size++;
  }
}

}  // namespace

JVSIODataClient::JVSIODataClient() {
  // U2X, 117.647Kbps [+2.1%] (PROTO: 111.111Kbps [-5.6%])
  UBRRH = 0;
#if defined(PROTO)
  UBRRL = 8;
#else
  UBRRL = 16;
#endif
  UCSRA = 0x02;
  // RX enabled, interrupt ON
  UCSRB = (UCSRB & 0x48) | 0x90;
  // 8-bits, non-parity, 1 stop-bit
  UCSRC = 0x86;

#if defined(PROTO)
  // PD1 Input
  DDRD &= ~0x02;
  PORTD &= ~0x02;
#else
  // PD2 Input
  DDRD &= ~0x04;
  PORTD &= ~0x04;
#endif
  // PD0 Output (but masked during RX being enabled), Pull-up
  DDRD |= 0x01;
  PORTD |= 0x01;

  sei();
}

int JVSIODataClient::available() {
  return rx_size;
}

void JVSIODataClient::setMode(int mode) {
  uint8_t oldSREG = SREG;
  cli();
  if (mode == INPUT) {
#if defined(PROTO)
    DDRD &= ~0x02;
#else
    DDRD &= ~0x04;
#endif
    UCSRB |= 0x90;   // RX enabled, interrupt ON
  } else {
#if defined(PROTO)
    DDRD |= 0x02;
#else
    DDRD |= 0x04;
#endif
    UCSRB &= ~0x90;  // RX disabled, interrupt OFF
  }
  SREG = oldSREG;
}

void JVSIODataClient::startTransaction() {
  cli();
}

void JVSIODataClient::endTransaction() {
  sei();
}

uint8_t JVSIODataClient::read() {
  uint8_t oldSREG = SREG;
  cli();
  uint8_t data = 0;
  if (rx_size) {
    data = rx_buf[rx_rd_ptr];
    rx_rd_ptr = (rx_rd_ptr + 1) % rx_buf_size;
    rx_size--;
  }
  SREG = oldSREG;
  return data;
}

void JVSIODataClient::write(uint8_t data) {
  // 138t for each bit. (PROTO: 69t)
  asm (
    "rjmp 4f\n"

   // Spends 134t = 8 + 1 + 3 x N - 1 + 2 + 4; N = 40 (PROTO: 65t; N = 17)
   "1:\n"
    "brcs 2f\n"      // 2t (1t for not taken)
    "nop\n"          // 1t
    "cbi 0x12, 0\n"  // 2t
#if defined(PROTO)
    "sbi 0x12, 1\n"  // 2t
#else
    "sbi 0x12, 2\n"  // 2t
#endif
    "rjmp 3f\n"      // 2t (1 + 1 + 2 + 2 + 2)
   "2:\n"
    "sbi 0x12, 0\n"  // 2t
#if defined(PROTO)
    "cbi 0x12, 1\n"  // 2t
#else
    "cbi 0x12, 2\n"  // 2t
#endif
    "rjmp 3f\n"      // 2t (2 + 2 + 2 + 2)
   "3:\n"
#if defined(PROTO)
    "ldi r19, 17\n"  // 1t
#else
    "ldi r19, 40\n"  // 1t
#endif
   "2:\n"
    "dec r19\n"      // 1t
    "brne 2b\n"      // 2t (1t for not taken)
    "nop\n"          // 1t
    "nop\n"          // 1t
    "ret\n"          // 4t

   // Sends Start, bit 0, ..., bit 7, Stop
   "4:\n"
    "mov r18, %0\n"
    // Start bit
    "sec\n"         // 1t
    "rcall 1b\n"    // 3t
    "clc\n"         // 1t
    "rcall 1b\n"    // 3t
    // Bit 0
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 1
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 2
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 3
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 4
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 5
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 6
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Bit 7
    "ror r18\n"     // 1t
    "rcall 1b\n"    // 3t
    // Stop bit
    "sec\n"         // 1t
    "rcall 1b\n"    // 3t
   :: "r" (data)
   : "r18", "r19");
}

void JVSIOSenseClient::begin() {
  TCCR2 = 0x19;    // CTC mode, toggle on matching
  OCR2 = 0;        // Match on every cycles
  DDRD |= 0x80;    // Output enabled
  PORTD &= ~0x80;  // Prepare low output
}

void JVSIOSenseClient::set(bool ready) {
  if (ready)
    TCCR2 &= ~0x30;
  else
    TCCR2 |= 0x10;
}
