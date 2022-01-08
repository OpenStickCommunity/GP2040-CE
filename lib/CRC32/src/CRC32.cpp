//
// Copyright (c) 2013 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//

#include "CRC32.h"

static const uint32_t crc32_table[] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

CRC32::CRC32() {
	reset();
}

void CRC32::reset() {
	_state = ~0L;
}

void CRC32::update(const uint8_t &data) {
	// via http://forum.arduino.cc/index.php?topic=91179.0
	uint8_t tbl_idx = 0;

	tbl_idx = _state ^ (data >> (0 * 4));
	_state = *(uint32_t *)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4);
	tbl_idx = _state ^ (data >> (1 * 4));
	_state = *(uint32_t *)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4);
}

uint32_t CRC32::finalize() const
{
	return ~_state;
}
