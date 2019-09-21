// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#if !defined(__DIPSW_H__)
# define __DIPSW_H__

#include <stdint.h>

class DIPSW final {
 public:
  DIPSW();
  void Update();
  void Sync();
  bool GetRapidMode();
  bool GetRapidMask();
  bool GetSwapMode();

 private:
  uint8_t Peek();

  uint8_t dipsw = 0xff;
  bool rapid = false;
  uint8_t rapid_count = 0;
  uint8_t rapid_mod = 0;
  uint8_t rapid_th = 0;
  bool swap = false;
};

#endif // __DIPSW_H__
