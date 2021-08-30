#ifndef _HELPERS_H_
#define _HELPERS_H_ 

#include <stdint.h>

class Helpers
{
public:
  static uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
  static uint32_t Wheel(uint8_t pos);
};


#endif