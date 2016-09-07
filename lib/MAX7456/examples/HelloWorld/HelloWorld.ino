/*
  HelloWorld.ino
    
    Demonstrates most simple use of the MAX7456 Library.


    This example code is in the public domain.
    Requires Arduino 1.0 or later.


    History :
      1.0 ~ Creation of HelloWorld from MAX7456Demo 0.4
            2013.01.301 - F. Robert Honeyman <www.coldcoredesign.com>
      
*/



// Included Libraries //////////////////////////////////////////////////////////

  #include <SPI.h>
  #include <MAX7456.h>





// Pin Mapping /////////////////////////////////////////////////////////////////
  
  // pinValue = 0 means "not connected"

  //  FDTI Basic 5V                   ---  Arduino  VCC      (AVCC,VCC)
  //  FDTI Basic GND                  ---  Arduino  GND      (AGND,GND)
  //  FDTI Basic CTS                  ---  Arduino  GND      (AGND,GND)
  //  FDTI Basic DTR                  ---  Arduino  GRN
  //  FDTI Basic TXO                  ---> Arduino  TXO [PD0](RXD)
  //  FDTI Basic RXI                 <---  Arduino  RXI [PD1](TXD)
  
  
  //  Max7456 +5V   [DVDD,AVDD,PVDD]  ---  Arduino  VCC      (AVCC,VCC)
  //  Max7456 GND   [DGND,AGND,PGND]  ---  Arduino  GND      (AGND,GND)
  //  Max7456 CS    [~CS]            <---  Arduino  10  [PB2](SS/OC1B)
  //  Max7456 CS    [~CS]            <---  Mega2560 43  [PL6]
  const byte osdChipSelect             =            43;
  //  Max7456 DIN   [SDIN]           <---  Arduino  11  [PB3](MOSI/OC2)
  //  Max7456 DIN   [SDIN]           <---  Mega2560 51  [PB2](MOSI)
  const byte masterOutSlaveIn          =                      MOSI;
  //  Max7456 DOUT  [SDOUT]           ---> Arduino  12  [PB4](MISO)
  //  Max7456 DOUT  [SDOUT]           ---> Mega2560 50  [PB3](MISO)
  const byte masterInSlaveOut          =                      MISO;
  //  Max7456 SCK   [SCLK]           <---  Arduino  13  [PB5](SCK)
  //  Max7456 SCK   [SCLK]           <---  Mega2560 52  [PB1](SCK)
  const byte slaveClock                =                      SCK;
  //  Max7456 RST   [~RESET]          ---  Arduino  RST      (RESET)
  const byte osdReset                  =            0;
  //  Max7456 VSYNC [~VSYNC]          -X-
  //  Max7456 HSYNC [~HSYNC]          -X-
  //  Max7456 LOS   [LOS]             -X-





// Global Macros ///////////////////////////////////////////////////////////////





// Global Constants ////////////////////////////////////////////////////////////

  const unsigned long debugBaud = 9600;         // Initial serial baud rate for 
                                                //   Debug PC interface
  




// Global Variables ////////////////////////////////////////////////////////////
  
  HardwareSerial Debug = Serial;                // Set debug connection
  
  MAX7456 OSD( osdChipSelect );





// Hardware Setup //////////////////////////////////////////////////////////////

  void setup() 
  {
    
    // Initialize the Serial connection to the debug computer:
    Debug.begin( debugBaud );
    
    
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    
    // Initialize the MAX7456 OSD:
    OSD.begin();                                // Use NTSC with default area.
    //OSD.setCharEncoding( MAX7456_ASCII );       // Only needed if ascii font.
    OSD.display();                              // Activate the text display.
    
  }
  // setup()





// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    while (OSD.notInVSync());                   // Wait for VSync to start to 
                                                //   prevent write artifacts.
    OSD.print( "Hello World!" );                // The '!' will not display
                                                //   because it does not come 
                                                //   in the factory font.
    while (true);
    
  } 
  // loop()





// Interrupt Methods ///////////////////////////////////////////////////////////





// Local Methods ///////////////////////////////////////////////////////////////



