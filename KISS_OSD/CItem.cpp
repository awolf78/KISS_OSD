#include "CItem.h"
#include "fixFont.h"

CItem::CItem():
m_OSD(NULL),
m_page(NULL)
{
  m_key[0] = 0;
}

CItem::~CItem()
{
}

void CItem::Setup(MAX7456 *OSD, char* key, CPage *page)
{
  m_OSD = OSD;
  m_page = page;
  strcpy(m_key, fixStr(key));
}

void CItem::Print()
{
  m_OSD->print(m_key);
}

void CItem::ValueLeft()
{
}

void CItem::ValueRight()
{
}

char* CItem::GetKey()
{
  return m_key;
}

int16_t CItem::GetValue()
{
  return -1;
}

CPage* CItem::GetNextPage()
{
  return m_page;
}



CIntItem::CIntItem()
{
}

CIntItem::~CIntItem()
{
}

void CIntItem::Setup(MAX7456 *OSD, char* key, int16_t *value, int16_t minVal, int16_t maxVal, uint8_t decimal, uint8_t tick, char* suffix)
{
  CItem::Setup(OSD, fixStr(key), NULL);
  m_tick = tick;
  m_minVal = minVal;
  m_maxVal = maxVal;
  m_value = value;
  m_decimal = decimal;
  strcpy(m_suffix, fixStr(suffix));
}

void CIntItem::Print()
{
  char value[15];
  Print_int16(*m_value, value, m_decimal,0);
  if(m_key[0] == 0)
  {
    m_OSD->print(value);
  }
  else
  {
    m_OSD->print(m_key);
    m_OSD->print(fixStr(" "));
    m_OSD->print(value);
    m_OSD->print(m_suffix);
  }
}

void CIntItem::ValueLeft()
{
  if((*m_value - m_tick) > m_minVal)
  {
    *m_value = *m_value - m_tick;
  }
  else
  {
    *m_value = m_minVal;
  }
}

void CIntItem::ValueRight()
{
  if((*m_value + m_tick) < m_maxVal)
  {
    *m_value = *m_value + m_tick;
  }
  else
  {
    *m_value = m_maxVal;
  }
}

int16_t CIntItem::GetValue()
{
  return *m_value;
}

void CIntItem::Print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft){
  uint16_t useVal = p_int;
  #ifdef COMPRESSED_FONT
  uint8_t pre = 0x31;
  #else
  uint8_t pre = ' ';
  #endif
  if(p_int < 0){
      useVal = p_int*-1;
      #ifdef COMPRESSED_FONT
      pre = 0x03;
      #elseif
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
  const char blank = 0x31;
  const char zero = 0x06;
  const char point = 0x04;
  char result[6] = {0x31,0x31,0x31,0x31,0x31,0x06};
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
  str[CharPos++] = 0x00;
  //return CharPos;
}



CStringItem::CStringItem()
{
}

CStringItem::~CStringItem()
{
}

void CStringItem::Setup(MAX7456 *OSD, char* key, int16_t *value, char* valueRange[], uint8_t amount)
{
  CItem::Setup(OSD, key, NULL);
  m_index = value;
  m_amount = amount;
  uint8_t i;
  for(i=0; i<amount; i++)
  {
    strcpy(m_valueRange[i], valueRange[i]);
  }
}

void CStringItem::Print()
{
  m_OSD->print(m_key);
  m_OSD->print(" ");
  m_OSD->print(m_valueRange[*m_index]);
}

void CStringItem::ValueLeft()
{
  if(*m_index > 0)
  {
    *m_index--;
  }
  Print();
}

void CStringItem::ValueRight()
{
  if(*m_index < (m_amount-1))
  {
    *m_index++;
  }
  Print();
}

int16_t CStringItem::GetValue()
{
  return *m_index;
}
