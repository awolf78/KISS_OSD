#ifndef Fixfonth
#define Fixfonth
#include "Flash.h"

#define COMPRESSED_FONT

inline char fixNo(char no)
{
  return no - 0x2A;
}

char* fixStr(const char* str);

char fixChar(char str);

char* fixFlashStr(_FLASH_STRING *str);

#endif
