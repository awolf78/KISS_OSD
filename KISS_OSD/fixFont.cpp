#include "fixFont.h"
#include "Arduino.h"

char fixChar(char str)
{
  #ifdef COMPRESSED_FONT
  //str = tolower(str);
  if(str > 0x60 && str < 0x7B)
  {
    str = str - 0x4B;
    return str;
  }
  switch(str)
  {
    case '!': str = 0x01; return str;
    case '%': str = 0x02; return str;
    case '-': str = 0x03; return str;
    case '.': str = 0x04; return str;
    case '/': str = 0x05; return str;
    case 0xB0: str = 0x30; return str; //°C
    case 0xB1: str = 0x31; return str; //°F
    case '+': str = 0x33; return str;
    case '>': str = 0x14; return str;
  }    
  if(str > 0x2F && str < 0x40)
  {
    str = fixNo(str);
    return str;
  }
  str = 0x32;
  #endif
  return str; 
}

static const uint8_t MAX_FIX_STR = 30;
static char fixedString[MAX_FIX_STR];

char* fixStr(const char* str)
{
  uint8_t i;
  for(i=0; i<strlen(str) && i < MAX_FIX_STR-1; i++)
  {
    fixedString[i] = fixChar(str[i]);
  }
  if(i < MAX_FIX_STR)
  {
    fixedString[i] = 0x00;
  }
  return fixedString;
}

char* fixFlashStr(_FLASH_STRING* str)
{
  uint8_t i;
  for(i=0; i<str->length() && i<MAX_FIX_STR-1; i++)
  {
    fixedString[i] = fixChar((*str)[i]);
  }
  if(i < MAX_FIX_STR)
  {
    fixedString[i] = 0x00;
  }
  return fixedString;
}
