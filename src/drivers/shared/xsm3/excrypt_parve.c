#include <stdlib.h>
#include <string.h>

#include "xsm3/excrypt.h"

void ExCryptParveEcb(const uint8_t* key, const uint8_t* sbox, const uint8_t* input, uint8_t* output)
{
  uint8_t block[9];

  memcpy(block, input, 8);
  block[8] = block[0];

  for (int i = 8; i > 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      uint8_t x = key[j] + block[j] + i;
      uint8_t y = sbox[x] + block[j + 1];
      block[j + 1] = ROTL8(y, 1);
    }

    block[0] = block[8];
  }

  memcpy(output, block, 8);
}

void ExCryptParveCbcMac(const uint8_t* key, const uint8_t* sbox, const uint8_t* iv, const uint8_t* input, uint32_t input_size, uint8_t* output)
{
  uint64_t block;
  uint64_t temp;
  memcpy(&block, iv, 8);

  if (input_size >= 8)
  {
    for (uint32_t i = 0; i < input_size / 8; i++)
    {
      memcpy(&temp, input + (i * 8), sizeof(temp));
      block ^= temp;
      ExCryptParveEcb(key, sbox, (uint8_t*)&block, (uint8_t*)&block);
    }
  }

  memcpy(output, &block, 8);
}

static uint32_t read_be32(const uint8_t* data)
{
  uint32_t value;
  memcpy(&value, data, sizeof(value));
  return SWAP32(value);
}

void ExCryptChainAndSumMac(const uint8_t* cd, const uint8_t* ab, const uint8_t* input, uint32_t input_dwords, uint8_t* output)
{
  uint64_t out0 = 0;
  uint64_t out1 = 0;

  uint32_t ab0 = read_be32(ab) % 0x7FFFFFFF;
  uint32_t ab1 = read_be32(ab + sizeof(uint32_t)) % 0x7FFFFFFF;
  uint32_t cd0 = read_be32(cd) % 0x7FFFFFFF;
  uint32_t cd1 = read_be32(cd + sizeof(uint32_t)) % 0x7FFFFFFF;

  for (uint32_t i = 0; i < input_dwords / 2; i++)
  {
    out0 += (uint64_t)read_be32(input) * 0xE79A9C1;
    out0 = (out0 % 0x7FFFFFFF) * ab0;
    out0 += ab1;
    out0 = out0 % 0x7FFFFFFF;

    out1 += out0;

    out0 = (uint64_t)(read_be32(input + sizeof(uint32_t)) + out0) * cd0;
    out0 = (out0 % 0x7FFFFFFF) + cd1;
    out0 = out0 % 0x7FFFFFFF;

    out1 += out0;

    input += 2 * sizeof(uint32_t);
  }
  uint32_t result0 = SWAP32((out0 + ab1) % 0x7FFFFFFF);
  uint32_t result1 = SWAP32((out1 + cd1) % 0x7FFFFFFF);
  memcpy(output, &result0, sizeof(result0));
  memcpy(output + sizeof(result0), &result1, sizeof(result1));
}
