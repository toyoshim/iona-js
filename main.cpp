// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <avr/interrupt.h>

#include "Arduino.h"
#include "DIPSW.h"
#include "Jamma.h"
#include "JVSIOClient.h"

namespace {

#if defined(PROTO)
const char id[] = "SEGA ENTERPRISES,LTD.compat;IONA-KVC-P0;ver1.01";
#else
const char id[] = "SEGA ENTERPRISES,LTD.compat;MP01-IONA-JS;ver1.10";
#endif
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
    jamma.Sync();
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
    jamma.SubCoin(data[1] - 1, data[3]);
    io.pushReport(JVSIO::kReportOk);
    break;
   case JVSIO::kCmdDriverOutput:
    gpout = data[2];
    io.pushReport(JVSIO::kReportOk);
    break;
  }
}

}  // namespace

int main() {
  JVSIODataClient data;
  JVSIOSenseClient sense;
  JVSIO::LedClient led;
  Serial.println("IONA JAMMA Standard Model - Ver 1.10");
  Serial.println(id);

  JVSIO io(&data, &sense, &led);
  io.begin();
  Serial.println("boot");

  for (;;)
    loop(io);
  return 0;
}
