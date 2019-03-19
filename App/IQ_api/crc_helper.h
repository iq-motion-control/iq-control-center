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

#ifndef CRC_HELPER_H
#define	CRC_HELPER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compute CRC word for a byte string.
 */
uint16_t MakeCrc(const uint8_t *data, int count);

/* Update a CRC accumulation with one data byte.
 */
uint16_t ByteUpdateCrc(uint16_t crc, uint8_t data);

/* Update a CRC accumulation with several data bytes.
 */
uint16_t ArrayUpdateCrc(uint16_t crc, const uint8_t *data, int count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
