/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "crc_helper.h"

// Compute CRC word for a byte string.
uint16_t MakeCrc(const uint8_t *data, int count) {

  uint16_t crc = 0xffff;

  uint16_t i;
  for(i = 0; i < count; i++) {
    crc = ByteUpdateCrc(crc, data[i]);
  }
  return crc;
}

// Update a CRC accumulation with one data byte.
uint16_t ByteUpdateCrc(uint16_t crc, uint8_t data) {

  uint16_t x = (crc >> 8) ^ data;
  x ^= x >> 4;

  crc = (crc << 8) ^ (x << 12) ^ (x <<5) ^ x;
  return crc;
}

// Update a CRC accumulation with several data bytes.
uint16_t ArrayUpdateCrc(uint16_t crc, const uint8_t *data, int count) {

  uint16_t i;
  for(i = 0; i < count; i++) {
    crc = ByteUpdateCrc(crc, data[i]);
  }
  return crc;
}
