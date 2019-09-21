// Copyright 2019 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#if !defined(__Jamma_H__)
# define __Jamma_H__

#include <stdint.h>

class Jamma final {
 public:
  Jamma();
  void Initialize();
  void Update(bool swap);
  void Sync();
  uint8_t GetSw(uint8_t index, bool rapid_mode, bool rapid_mask);
  uint8_t GetCoin(uint8_t index);
  void SubCoin(uint8_t index, uint8_t sub);

 private:
  uint8_t sw[4];
  uint8_t current_coin_sw[2];
  uint8_t coin_sw[2];
  uint8_t coin_count[2];
};

#endif // __Jamma_H__
