#include "printInt16_t.h"
#include "fixFont.h"

uint8_t print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft)
{
  uint16_t useVal = p_int;
  #ifdef COMPRESSED_FONT
  uint8_t pre = 0x32;
  #else
  uint8_t pre = ' ';
  #endif
  if(p_int < 0){
      useVal = p_int*-1;
      #ifdef COMPRESSED_FONT
      pre = 0x03;
      #else
      pre = '-';
      #endif
  }
  uint8_t aciidig[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  uint8_t i = 0;
  uint8_t digits[6] = {0,0,0,0,0,0};
  while(useVal >= 10000){digits[0]++; useVal-=10000;}
  while(useVal >= 1000){digits[1]++; useVal-=1000;}
  while(useVal >= 100){digits[2]++; useVal-=100;}
  while(useVal >= 10){digits[3]++; useVal-=10;}
  digits[4] = useVal;
  #ifdef COMPRESSED_FONT
  const char blank = 0x32;
  const char zero = 0x06;
  const char point = 0x04;
  char result[6] = {0x32,0x32,0x32,0x32,0x32,0x06};
  #else
  const char blank = ' ';
  const char zero = '0';
  const char point = '.';
  char result[6] = {' ',' ',' ',' ',' ','0'};
  #endif
  uint8_t signdone = 0;
  for(i = 0; i < 6;i++){
      if(i == 5 && signdone == 0) continue;
      else if(aciidig[digits[i]] != '0' && signdone == 0){
          result[i] = pre;
          signdone = 1;
      }else if(signdone) result[i] = fixNo(aciidig[digits[i-1]]);
  }
  uint8_t CharPos = 0;
  for(i = 0; i < 6;i++){
    if(result[i] != blank || (AlignLeft == 0 || (i > 5-dec))) str[CharPos++] = result[i];
    if(dec != 0 && i == 5-dec) str[CharPos++] = point;
    if(dec != 0 && i > 5-dec && str[CharPos-1] == blank) str[CharPos-1] = zero;
  }
  if(AlignLeft == 1 || CharPos > 1)
  {
    CharPos--;
  }
  str[CharPos+1] = 0x00;
  return CharPos+1;
}

void print_time(unsigned long time, char *time_str) 
{
    uint16_t seconds = time / 1000;
    uint8_t minutes = seconds / 60;
    if (seconds >= 60) 
    {
      minutes = seconds/60;
    } 
    else 
    {
      minutes = 0;
    }
    seconds = seconds - (minutes * 60);
    static char time_sec[6];
    uint8_t i = 0;
    uint8_t time_pos = print_int16(minutes, time_str,0,1);
    #ifdef COMPRESSED_FONT
    time_str[time_pos++] = 0x10;
    #else
    time_str[time_pos++] = ':';
    #endif
    
    uint8_t sec_pos = print_int16(seconds, time_sec,0,1);
    if(seconds < 10) 
    {
      #ifdef COMPRESSED_FONT
      time_str[time_pos++] = 0x06;
      #else
      time_str[time_pos++] = '0';
      #endif
    }
    for (i=0; i<sec_pos; i++)
    {
      time_str[time_pos++] = time_sec[i];
    }
    time_str[time_pos++] = 0x00;
}
