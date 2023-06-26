#ifndef _BASE64_H_
#define _BASE64_H_

/**
 * The MIT License (MIT)
 * Copyright (c) 2016 tomykaira
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cstdint>
#include <string>

class Base64 {
 public:

  static std::string Encode(const char* dataPtr, size_t dataLen) {
    static constexpr char sEncodingTable[] = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t out_len = 4 * ((dataLen + 2) / 3);
    std::string ret;
    ret.resize(out_len);

    size_t i = 0;
    char *p = ret.data();

    if (dataLen >= 2) {
      for (; i < dataLen - 2; i += 3) {
        *p++ = sEncodingTable[(dataPtr[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((dataPtr[i] & 0x3) << 4) | ((int) (dataPtr[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((dataPtr[i + 1] & 0xF) << 2) | ((int) (dataPtr[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[dataPtr[i + 2] & 0x3F];
      }
    }

    if (i < dataLen) {
      *p++ = sEncodingTable[(dataPtr[i] >> 2) & 0x3F];
      if (i == (dataLen - 1)) {
        *p++ = sEncodingTable[((dataPtr[i] & 0x3) << 4)];
        *p++ = '=';
      }
      else {
        *p++ = sEncodingTable[((dataPtr[i] & 0x3) << 4) | ((int) (dataPtr[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((dataPtr[i + 1] & 0xF) << 2)];
      }
      *p++ = '=';
    }

    return ret;
  }

  static std::string Encode(const std::string data) {
    return Encode(data.data(), data.length());
  }

  static bool Decode(const char* dataPtr, size_t dataLen, std::string& out) {
    static constexpr unsigned char kDecodingTable[] = {
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
      64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
      64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    if (dataLen % 4 != 0)
    {
      out.clear();
      return false;
    }

    size_t out_len = dataLen / 4 * 3;
    if (dataLen >= 1 && dataPtr[dataLen - 1] == '=') out_len--;
    if (dataLen >= 2 && dataPtr[dataLen - 2] == '=') out_len--;

    out.resize(out_len);

    for (size_t i = 0, j = 0; i < dataLen;) {
      uint32_t a = dataPtr[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(dataPtr[i++])];
      uint32_t b = dataPtr[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(dataPtr[i++])];
      uint32_t c = dataPtr[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(dataPtr[i++])];
      uint32_t d = dataPtr[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(dataPtr[i++])];

      uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

      if (j < out_len) out[j++] = (triple >> 2 * 8) & 0xFF;
      if (j < out_len) out[j++] = (triple >> 1 * 8) & 0xFF;
      if (j < out_len) out[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return true;
  }

  static bool Decode(const std::string& input, std::string& out) {
    return Decode(input.data(), input.length(), out);
  }

};

#endif /* _BASE64_H_ */
