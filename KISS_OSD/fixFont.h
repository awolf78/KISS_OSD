#ifndef Fixfonth
#define Fixfonth

inline char fixNo(char no)
{
#ifdef COMPRESSED_FONT
  return no - (char) 0x2A;
#else
  return no;
#endif
}

char* fixStr(char* str);

#endif
