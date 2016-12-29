#ifndef MAX7456CONFIG_H
#define MAX7456CONFIG_H

#include "MyMax7456.h"

class CMax7456Config : public CMyMax7456
{
  public:
    CMax7456Config(uint8_t chipSelect);
    void updateFont();
  private:
    bool m_ledstatus;
    const uint8_t LEDPIN = 7;
    uint8_t m_fontData[54];
    byte O, L, M;
    int16_t m_offset;
    int16_t m_length;
    static const int16_t RING_BUF_SIZE = 512;
    uint8_t ringBuf[RING_BUF_SIZE];
    int16_t ringHead;
    
    uint8_t spi_transfer(uint8_t data);
    void write_NVM(uint8_t char_address);
    uint8_t decompress();
};

#endif
