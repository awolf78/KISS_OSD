#include "Max7456Config.h"
#include "Config.h"
#ifdef STEELE_PDB
#include "fontCompressedSteelePDB.h"
#else
#include "fontCompressed.h"
#endif

class Flashbits {
public:
  void begin(byte *s) {
    src = s;
    mask = 0x01;
  }
  byte get1(void) {
    byte r = (pgm_read_byte(src) & mask) != 0;
    mask <<= 1;
    if (mask == 0) {
      mask = 1;
      src++;
    }
    return r;
  }
  uint16_t getn(byte n) {
    uint16_t r = 0;
    while (n--) {
      r <<= 1;
      r |= get1();
    }
    return r;
  }
private:
  byte *src;
  byte mask;
};

static Flashbits BS;


CMax7456Config::CMax7456Config(uint8_t chipSelect) : CMyMax7456(chipSelect)
{
  
}

void CMax7456Config::updateFont(uint8_t *byteBuf)
{
  ringBuf = byteBuf;
  BS.begin(fontCompressed);
  O = BS.getn(4);
  L = BS.getn(4);
  M = BS.getn(2);
  BS.getn(16); //length, ignoring for now
  ringHead = 0;
  m_length = 0;
  m_offset = 0;    
  m_ledstatus = false; 
  
  for(uint16_t x = 0; x < 256; x++){
    for(uint8_t i = 0; i < 54; i++){
      m_fontData[i] = decompress();
    }
    write_NVM(x);
    m_ledstatus=!m_ledstatus;
    if (m_ledstatus==true){
      digitalWrite(LEDPIN,HIGH);
    }
    else{
      digitalWrite(LEDPIN,LOW);
    }
    delay(20); // Shouldn't be needed due to status reg wait.
  }
}

#define MAX7456ADD_VM0          0x00 
#define MAX7456ADD_VM1          0x01
#define MAX7456ADD_HOS          0x02
#define MAX7456ADD_VOS          0x03
#define MAX7456ADD_DMM          0x04
#define MAX7456ADD_DMAH         0x05
#define MAX7456ADD_DMAL         0x06
#define MAX7456ADD_DMDI         0x07
#define MAX7456ADD_CMM          0x08
#define MAX7456ADD_CMAH         0x09
#define MAX7456ADD_CMAL         0x0a
#define MAX7456ADD_CMDI         0x0b
#define MAX7456ADD_OSDM         0x0c
#define MAX7456ADD_RB0          0x10
#define MAX7456ADD_RB1          0x11
#define MAX7456ADD_RB2          0x12
#define MAX7456ADD_RB3          0x13
#define MAX7456ADD_RB4          0x14
#define MAX7456ADD_RB5          0x15
#define MAX7456ADD_RB6          0x16
#define MAX7456ADD_RB7          0x17
#define MAX7456ADD_RB8          0x18
#define MAX7456ADD_RB9          0x19
#define MAX7456ADD_RB10         0x1a
#define MAX7456ADD_RB11         0x1b
#define MAX7456ADD_RB12         0x1c
#define MAX7456ADD_RB13         0x1d
#define MAX7456ADD_RB14         0x1e
#define MAX7456ADD_RB15         0x1f
#define MAX7456ADD_OSDBL        0x6c
#define MAX7456ADD_STAT         0xA0
#define NVM_ram_size 54
#define WRITE_nvr 0xa0
#define STATUS_reg_nvr_busy 0x20

uint8_t CMax7456Config::spi_transfer(uint8_t data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
    ;
  return SPDR;                    // return the received byte
}

void CMax7456Config::write_NVM(uint8_t char_address)
{
  // disable display
   digitalWrite(deviceSelectPin,LOW);
  spi_transfer(MAX7456ADD_VM0); 
  //spi_transfer(DISABLE_display);

  
  
  //digitalWrite(deviceSelectPin,LOW);
  //spi_transfer(VM0_reg);
#ifdef PAL
  spi_transfer(0x40);
#else
  spi_transfer(0);
#endif

  spi_transfer(MAX7456ADD_CMAH); // set start address high
  spi_transfer(char_address);

  for(uint8_t x = 0; x < NVM_ram_size; x++) // write out 54 bytes of character to shadow ram
  {
    spi_transfer(MAX7456ADD_CMAL); // set start address low
    spi_transfer(x);
    spi_transfer(MAX7456ADD_CMDI);
    spi_transfer(m_fontData[x]);
  }

  // transfer 54 bytes from shadow ram to NVM
  spi_transfer(MAX7456ADD_CMM);
  spi_transfer(WRITE_nvr);
  
  // wait until bit 5 in the status register returns to 0 (12ms)
  while ((spi_transfer(MAX7456ADD_STAT) & STATUS_reg_nvr_busy) != 0x00);

 spi_transfer(MAX7456ADD_VM0); // turn on screen next vertical
  //spi_transfer(ENABLE_display_vert);
#ifdef PAL
  spi_transfer(0x4c);
#else
  spi_transfer(0x0c);
#endif 
  digitalWrite(deviceSelectPin,HIGH);
}

uint8_t CMax7456Config::decompress()
{
  uint8_t next;
  if(m_length == 0 && BS.get1() == 0) 
  {
    next = (byte)BS.getn(8);
  } 
  else 
  {
    if(m_length == 0)
    {
      m_offset = -BS.getn(O) - 1;
      m_length = BS.getn(L) + M;
    }
    if(m_length--) 
    {
      next = ringBuf[(ringHead+m_offset)%RING_BUF_SIZE];
    }
  }  
  ringBuf[ringHead%RING_BUF_SIZE] = next;
  ringHead++;
  return next;
}
