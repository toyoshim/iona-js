// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <avr/interrupt.h>

#include "Arduino.h"
#include "Jamma.h"
#include "JVSIOClient.h"

namespace {

class DIPSW final {
 public:
  DIPSW() {
    DDRD &= ~0x78;
    PORTD |= 0x78;
    Update();
  }

  void Update() {
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
    swap = dipsw & 2;
  }

  void Sync() {
    if (!rapid)
      return;
    rapid_count++;
    if (rapid_count == rapid_mod)
      rapid_count = 0;
  }

  bool GetRapidMode() {
    return rapid;
  }

  bool GetRapidMask() {
    return rapid_count < rapid_th;
  }

  bool GetSwapMode() {
    return swap;
  }

 private:
  uint8_t Peek() {
    return ~(PIND >> 3) & 0x0f;
  }

  uint8_t dipsw = 0xff;
  bool rapid = false;
  uint8_t rapid_count = 0;
  uint8_t rapid_mod = 0;
  uint8_t rapid_th = 0;
  bool swap = false;
};

const char id[] = "SEGA ENTERPRISES,LTD.compat;MP01-IONA-JS;ver1.00";
uint8_t gpout = 0;

Jamma jamma;
DIPSW dipsw;

void loop(JVSIO& io) {
  uint8_t len;
  uint8_t* data = io.getNextCommand(&len);
  if (!data) {
    dipsw.Update();
    jamma.Update(dipsw.GetSwapMode());
    return;
  }

  switch (*data) {
   case JVSIO::kCmdIoId:
    io.pushReport(JVSIO::kReportOk);
    for (size_t i = 0; id[i]; ++i)
      io.pushReport(id[i]);
    io.pushReport(0x00);
    jamma.Initialize();
    break;
   case JVSIO::kCmdFunctionCheck:
    io.pushReport(JVSIO::kReportOk);

    io.pushReport(0x01);  // sw
    io.pushReport(0x02);  // players
    io.pushReport(0x0C);  // buttons
    io.pushReport(0x00);

    io.pushReport(0x03);  // analog inputs
    io.pushReport(0x08);  // channels
    io.pushReport(0x00);  // bits
    io.pushReport(0x00);

    io.pushReport(0x12);  // general purpose driver
    io.pushReport(0x08);  // slots
    io.pushReport(0x00);
    io.pushReport(0x00);

    io.pushReport(0x02);  // coin
    io.pushReport(0x02);  // slots
    io.pushReport(0x00);
    io.pushReport(0x00);

    io.pushReport(0x00);
    break;
   case JVSIO::kCmdSwInput:
    dipsw.Sync();
    io.pushReport(JVSIO::kReportOk);
    if (data[1] == 2 && data[2] == 2) {
      io.pushReport(0x00);
      bool mode = dipsw.GetRapidMode();
      bool mask = dipsw.GetRapidMask();
      io.pushReport(jamma.GetSw(0, mode, mask));
      io.pushReport(jamma.GetSw(1, mode, mask));
      io.pushReport(jamma.GetSw(2, mode, mask));
      io.pushReport(jamma.GetSw(3, mode, mask));
    } else {
      Serial.println("Err CmdSwInput");
    }
    break;
   case JVSIO::kCmdCoinInput:
    io.pushReport(JVSIO::kReportOk);
    if (data[1] == 2) {
      io.pushReport((0 << 6) | 0);
      io.pushReport(jamma.GetCoin(0));
      io.pushReport((0 << 6) | 0);
      io.pushReport(jamma.GetCoin(1));
    } else {
      Serial.println("Err CmdCoinInput");
    }
    break;
   case JVSIO::kCmdAnalogInput:
    io.pushReport(JVSIO::kReportOk);
    for (size_t channel = 0; channel < data[1]; ++channel) {
      io.pushReport(0x80);
      io.pushReport(0x00);
    }
    break;
   case JVSIO::kCmdCoinSub:
    if (data[1] < 2)
      jamma.SubCoin(data[1], data[3]);
    io.pushReport(JVSIO::kReportOk);
    break;
   case JVSIO::kCmdDriverOutput:
    gpout = data[2];
    io.pushReport(JVSIO::kReportOk);
    Serial.print("gpout: ");
    Serial.print(gpout, HEX);
    Serial.println("");
    break;
  }
}

}  // namespace

int main() {
  JVSIODataClient data;
  JVSIOSenseClient sense;
  JVSIO::LedClient led;
  Serial.println("IONA JAMMA Standard Model - Ver 1.00");
  Serial.println(id);

  JVSIO io(&data, &sense, &led);
  io.begin();
  Serial.println("boot");

  for (;;)
    loop(io);
  return 0;
}
