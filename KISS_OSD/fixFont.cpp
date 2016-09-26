#include "fixFont.h"
#include "Arduino.h"

char* fixStr(char* str)
{
#ifdef COMPRESSED_FONT
  uint8_t i;
  for(i=0; i<strlen(str); i++)
  {
    //str[i] = tolower(str[i]);
    if(str[i] > 0x60 && str[i] < 0x7B)
    {
      str[i] = str[i] - 0x4B;
      continue;
    }
    switch(str[i])
    {
      case '!': str[i] = 0x01; continue;
      case '%': str[i] = 0x02; continue;
      case '-': str[i] = 0x03; continue;
      case '.': str[i] = 0x04; continue;
      case '/': str[i] = 0x06; continue;
      case 0xB0: str[i] = 0x2F; continue; //°C
      case 0xB1: str[i] = 0x30; continue; //°F
      case '+': str[i] = 0x32; continue;
    }    
    if(str[i] > 0x2F && str[i] < 0x40)
    {
      str[i] = fixNo(str[i]);
      continue;
    }
    str[i] = 0x31; 
  }
  return str;
#else
  return str;
#endif
}

/*char* fixStr(const char* str)
{
  static char temp[50];
  strcpy(temp, str);
  return fixStr(temp);
}*/
