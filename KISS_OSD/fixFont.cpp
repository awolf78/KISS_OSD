#include "fixFont.h"
#include "Arduino.h"
#include "CSettings.h"

extern CSettings settings;

char fixChar(char str)
{
  //str = tolower(str);
  #ifdef COMPRESSED_FONT
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
  #ifdef NICE_FONT
  if(settings.m_fontSize > 0)
  {
    uint8_t str2 = (uint8_t) str;
    if(str2 > 0x60 && str2 < 0x7B)
    {
      return (char)(str2 - 0x20);
    }
    if(str2 > 0xAF && str2 < 0xB2)
    {
      return (char)(str2 + 0x2);
    }
    if(str > 0x2F && str < 0x3A)
    {
      return fixNo(str);
    }
  }
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

char* fixPStr(const char *str)
{
  uint8_t i;
  for(i=0; i<strlen_P(str) && i<MAX_FIX_STR-1; i++)
  {
    fixedString[i] = fixChar(static_cast<char>(pgm_read_byte(str + i)));
  }
  if(i < MAX_FIX_STR)
  {
    fixedString[i] = 0x00;
  }
  return fixedString;
}

char fixNo(char no)
{
  #ifdef COMPRESSED_FONT
  return no - 0x2A;
  #endif
  #ifdef NICE_FONT
  if(settings.m_fontSize > 0)
  {
    uint8_t no2 = (uint8_t) no;
    if(no2 > 0x2F && no2 < 0x3A)
    {
      return (char)(no2 + 0x5A);
    }
  }
  #endif
  return no;
}
