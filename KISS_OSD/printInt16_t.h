#ifndef PrintInt16_th
#define PrintInt16_th

#include "Arduino.h"

#define COMPRESSED_FONT

uint8_t print_int16(int16_t p_int, char *str, uint8_t dec, uint8_t AlignLeft);

void print_time(unsigned long time, char *time_str);

#endif
