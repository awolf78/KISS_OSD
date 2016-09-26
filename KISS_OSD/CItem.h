#ifndef CItemh
#define CItemh
#include "MAX7456.h"
#include "CPage.h"

class CPage;

class CItem
{
  public:
  CItem();
  ~CItem();
  virtual void Setup(MAX7456 *OSD, char* key, CPage *page = NULL);
  virtual void Print();
  virtual void ValueLeft();
  virtual void ValueRight();
  virtual char* GetKey();
  virtual int16_t GetValue();
  virtual CPage* GetNextPage();
  static const uint8_t MAX_KEY_LENGTH = 15;
  
  protected:
  MAX7456 *m_OSD;
  CPage *m_page;
  char m_key[MAX_KEY_LENGTH];
};

class CIntItem : public CItem
{
  public:
  CIntItem();
  ~CIntItem();
  virtual void Setup(MAX7456 *OSD, char* key, int16_t *value, int16_t minVal, int16_t maxVal, uint8_t decimal = 0, uint8_t tick = 1, char* suffix = "");
  virtual void Print();
  virtual void ValueLeft();
  virtual void ValueRight();
  virtual int16_t GetValue();
  static const uint8_t MAX_SUFFIXLENGTH = 5;
  
  private:
  int16_t m_tick;
  char m_suffix[MAX_SUFFIXLENGTH];
  int16_t m_minVal, m_maxVal;
  uint8_t m_decimal;
  int16_t *m_value;
  void Print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft);
};

class CStringItem : public CItem
{
  public:
  CStringItem();
  ~CStringItem();
  virtual void Setup(MAX7456 *OSD, char* key, int16_t *value, char* valueRange[], uint8_t amount);
  virtual void Print();
  virtual void ValueLeft();
  virtual void ValueRight();
  virtual int16_t GetValue();
  static const uint8_t MAX_RANGE = 5;
  static const uint8_t MAX_VALUE_LENGTH = 10;
  
  private:
  int16_t *m_index;
  uint8_t m_amount;
  char m_valueRange[MAX_RANGE][MAX_VALUE_LENGTH];
};

#endif
