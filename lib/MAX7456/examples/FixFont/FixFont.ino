/*
  FixFont.ino
    
    Fixes pixel at (r6,c8) in the 'G' of the MAX7456 factory loaded font.


    This example code is in the public domain.
    Requires Arduino 1.0 or later.


    History :
      1.0 ~ Creation of FixFont from MAX7456Demo 0.4
            2013.01.31 - F. Robert Honeyman <www.coldcoredesign.com>
      
*/



// Included Libraries //////////////////////////////////////////////////////////

  #include <SPI.h>
  #include <MAX7456.h>
  #include <util/crc16.h>





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
  
  //#define DEBUG true





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
    OSD.setSwitchingTime( 5 );                  // Set video chroma distortion 
                                                //   to a minimum.
    OSD.display();                              // Activate the text display.
    
  }
  // setup()





// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    unsigned long errTime   = millis();
    byte          image[54];
    byte          backup[54] = { 0x55, 0x55, 0x55, 
                                 0x55, 0x55, 0x55, 
                                 0x54, 0x00, 0x15, 
                                 0x52, 0xAA, 0x85, 
                                 0x4A, 0xAA, 0xA1, 
                                 0x2A, 0x02, 0xA1, 
                                 0x28, 0x55, 0x28, 
                                 0x28, 0x55, 0x28, 
                                 0x28, 0x55, 0x41, 
                                 0x28, 0x40, 0x01, 
                                 0x28, 0x2A, 0xA8, 
                                 0x28, 0x2A, 0xA8, 
                                 0x28, 0x40, 0x28, 
                                 0x28, 0x55, 0x28, 
                                 0x2A, 0x00, 0x28, 
                                 0x4A, 0xAA, 0xA8, 
                                 0x52, 0xAA, 0xA8, 
                                 0x54, 0x00, 0x01  };
    char          pixel[4]  = { '#', '-', ' ', '-' };
    uint16_t      uiCRC     = 0x0000;
    
    
    // Read character image from NVM:
    while (!OSD.readChar( 0x11, image ))        // While waiting for read,
    {
      if (!OSD.status() &&                      // if connection fails,
          millis() > errTime)                   // and only every half second,
      {
        Debug.println( "OSD SPI failure" );     // report the failure,
        errTime = millis() + 500;               // reset timer.
      }
    }
    
    
    // Display character image:
    for(char i = 0;
        i < 54; 
        i++        )
    {
      Debug.print( pixel[(image[i] >> 6)
                           & 0x03       ] );
      Debug.print( pixel[(image[i] >> 4)
                           & 0x03       ] );
      Debug.print( pixel[(image[i] >> 2)
                           & 0x03       ] );
      Debug.print( pixel[(image[i] >> 0)
                           & 0x03       ] );
      
      uiCRC = _crc_xmodem_update( uiCRC,        // Update checksum.
                                  image[i] );

      if (i % 3 == 2) Debug.println();          // Display rows of 3 bytes.
    }
    
    Debug.println();
    
    
    // If it is the factory loaded 'G', fix it:
    if (uiCRC == 0x71CA)
    {
      
      Debug.println("Already fixed!");
      
    }
    else if (uiCRC == 0xA8DD &&
             image[16] == 2) 
    {
      
      while (Debug.available()) Debug.read();
      Debug.println( "Press any key to fix font..." );
      while (!Debug.available());
    
      backup[16] = 0;
    
      // Display corrected character image:
      for(char i = 0;
          i < 54; 
          i++        )
      {
        Debug.print( pixel[(backup[i] >> 6)
                             & 0x03       ] );
        Debug.print( pixel[(backup[i] >> 4)
                             & 0x03       ] );
        Debug.print( pixel[(backup[i] >> 2)
                             & 0x03       ] );
        Debug.print( pixel[(backup[i] >> 0)
                             & 0x03       ] );
        
        if (i % 3 == 2) Debug.println();        // Display rows of 3 bytes.
      }
      
      // Write corrected character image from NVM:
      while (!OSD.createChar( 0x11, backup ))   // While waiting for write,
      {
        if (!OSD.status() &&                    // if connection fails,
            millis() > errTime)                 // and only every half second,
        {
          Debug.println( "OSD SPI failure" );   // report the failure,
          errTime = millis() + 500;             // reset timer.
        }
      }
      
      Debug.println();
      Debug.println("Done!");
      
    }
    else
    {
      
      Debug.println("Wrong font.");
      
    }
    
    
    OSD.display();                              // Activate the text display 
                                                //   (both readChar() and 
                                                //   createChar() force 
                                                //   display off).
    while (OSD.notInVSync());                   // Wait for VSync to prevent 
                                                //   display artifacts.
    OSD.write( 'G' );
    
    while (true);                               // Busy wait end.
    
  } 
  // loop()





// Interrupt Methods ///////////////////////////////////////////////////////////





// Local Methods ///////////////////////////////////////////////////////////////



