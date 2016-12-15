#ifndef MYMAX7456_H
#define MYMAX7456_H

#include "MAX7456.h"
#include "Flash.h"

class CMyMax7456 : public MAX7456
{
  public:
    CMyMax7456(uint8_t chipSelect);
    void printInt16(uint8_t col, uint8_t row, int16_t value, uint8_t dec, uint8_t AlignLeft, const char* suffix = "", boolean twoBlanks = false, const char* prefix = "");
    void printFS(uint8_t col, uint8_t row, _FLASH_STRING *key, uint8_t menuItem = 200);
    void printInt16(uint8_t col, uint8_t row, _FLASH_STRING *key, int16_t value, uint8_t dec, uint8_t AlignLeft, const char* suffix = "", boolean twoBlanks = false);
    void printInt16(uint8_t col, uint8_t row, char *key, int16_t value, uint8_t dec, uint8_t AlignLeft, const char* suffix = "", boolean twoBlanks = false);
    void printIntArrow(uint8_t col, uint8_t row, _FLASH_STRING *key, int16_t value, uint8_t dec, uint8_t AlignLeft, uint8_t menuItem, const char* suffix = "", boolean twoBlanks = false);
    void printAligned(uint8_t row, int16_t value, uint8_t dec, uint8_t AlignLeft, const char* suffix = "", const char* suffix2 = "");
    void printTime(uint8_t col, uint8_t row, unsigned long time);
  private:
    uint8_t print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft);
    void print_time(unsigned long time, char *time_str);
    void printInternal(const char* suffix, boolean twoBlanks);
    void checkArrow(uint8_t currentRow, uint8_t menuItem);
};

#endif
