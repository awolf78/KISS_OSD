#ifdef IMPULSERC_VTX
#ifndef MAX7456RESET
# define MAX7456RESET  9         // RESET                     
#endif
# define MAX7456SELECT 10        // ss     


#define DATAOUT 11              // MOSI
#define DATAIN  12              // MISO
#define SPICLOCK  13            // sck
#define VSYNC 2                 // INT0

#ifndef WHITEBRIGHTNESS
  #define WHITEBRIGHTNESS 0x01
#endif
#ifndef BLACKBRIGHTNESS
  #define BLACKBRIGHTNESS 0x00
#endif

#define BWBRIGHTNESS ((BLACKBRIGHTNESS << 2) | WHITEBRIGHTNESS)

//MAX7456 opcodes
#define DMM_reg   0x04
#define DMAH_reg  0x05
#define DMAL_reg  0x06
#define DMDI_reg  0x07
#define VM0_reg   0x00
#define VM1_reg   0x01

// video mode register 0 bits
#define OSD_ENABLE 0x08
#define VIDEO_MODE_PAL 0x40
#define VIDEO_MODE_NTSC 0x00

//MAX7456 commands
#define CLEAR_display 0x04
#define CLEAR_display_vert 0x06
#define END_string 0xff


#define MAX7456ADD_VM0          0x00  //0b0011100// 00 // 00             ,0011100
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
#define MAX7456ADD_STAT         0xA0


//////////////////////////////////////////////////////////////
uint8_t spi_transfer(uint8_t data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
    ;
  return SPDR;                    // return the received byte
}

// ============================================================   WRITE TO SCREEN

void MAX7456Setup(void)
{
  uint8_t MAX7456_reset=0x02;

  pinMode(MAX7456RESET,OUTPUT);
  digitalWrite(MAX7456RESET,LOW); //force reset
  delay(100);
  digitalWrite(MAX7456RESET,HIGH); //hard enable
  delay(100);

  pinMode(MAX7456SELECT,OUTPUT);
  digitalWrite(MAX7456SELECT,HIGH); //disable device

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(VSYNC, INPUT);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (4 meg)

  SPCR = (1<<SPE)|(1<<MSTR);
  SPSR = (1<<SPI2X);
  uint8_t spi_junk;
  spi_junk=SPSR;
  spi_junk=SPDR;
  //delay(100);

  // force soft reset on Max7456
  digitalWrite(MAX7456SELECT,LOW);
  MAX7456_Send(VM0_reg, MAX7456_reset);
  delay(100);

  // set all rows to same charactor black/white level
  /*uint8_t x;
  for(x = 0; x < settings.ROWS; x++) {
    MAX7456_Send(MAX7456ADD_RB0+x, BWBRIGHTNESS);
  }*/

  // make sure the Max7456 is enabled
  spi_transfer(VM0_reg);

  if (settings.m_videoMode == 1){
    spi_transfer(OSD_ENABLE|VIDEO_MODE_PAL);
  }
  else{
    spi_transfer(OSD_ENABLE|VIDEO_MODE_NTSC);
  }
  digitalWrite(MAX7456SELECT,HIGH);
  delay(100);
}

void MAX7456_Send(uint8_t add, uint8_t data)
{
  spi_transfer(add);
  spi_transfer(data);
}

#endif
