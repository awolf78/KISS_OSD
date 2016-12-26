#ifndef Fixfonth
#define Fixfonth
#include "Flash.h"

//#define COMPRESSED_FONT
#define NICE_FONT

char fixNo(char no);

char* fixStr(const char* str);

char fixChar(char str);

char* fixFlashStr(_FLASH_STRING *str);

#endif
