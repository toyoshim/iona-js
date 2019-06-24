// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#if !defined(__JVSIOClient_H__)
# define __JVSIOClient_H__

#include "jvsio/JVSIO.h"

// Use PD0 as DATA+, and PD2 as DATA-. (PROTO: PD1 as DATA-)
// Receiving data is decoded by hardware, via RXD interface of PD0.
// Sending data is manually handled to make differential signal pairs.
// JVS relies on 115.2Kbps, 8-bits, non-parity, 1 start/stop-bit UART.
class JVSIODataClient final : public JVSIO::DataClient {
 public:
  JVSIODataClient();

 private:
  int available() override;
  void setMode(int mode) override;
  void startTransaction() override;
  void endTransaction() override;
  uint8_t read() override;
  void write(uint8_t data) override;
};

// Use PD7 for SENSE signal.
class JVSIOSenseClient final : public JVSIO::SenseClient {
 private:
  void begin() override;
  void set(bool ready) override;
};

#endif  // __JVSIOClient_H__
