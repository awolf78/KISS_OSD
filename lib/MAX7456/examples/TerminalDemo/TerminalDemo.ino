/*
  TerminalDemo.ino
    
    Demonstrates full capabilities of the MAX7456 Library.


    This example code is in the public domain.
    Requires Arduino 1.0 or later.


    History :
      1.0 ~ Creation of TerminalDemo from MAX7456Demo 0.4
            2013.01.31 - F. Robert Honeyman <www.coldcoredesign.com>
      
*/



// Included Libraries //////////////////////////////////////////////////////////

  #include <util/crc16.h>
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
  
  //#define DEBUG true





// Global Constants ////////////////////////////////////////////////////////////

  const unsigned long debugBaud = 9600;         // Initial serial baud rate for 
                                                //   Debug PC interface
  const unsigned long dataSourceBaud = 9600;    // Initial serial baud rate for 
                                                //   Input used by OSD
  




// Global Variables ////////////////////////////////////////////////////////////
  
  HardwareSerial Debug      = Serial;           // Set debug connection
  HardwareSerial DataSource = Serial;           // Set display source connection
  
  MAX7456 OSD( osdChipSelect );
  
  char * message = "Awaiting serial data.";





// Hardware Setup //////////////////////////////////////////////////////////////

  void setup() 
  {
    
    // Initialize the Serial connection to the debug computer:
    Debug.flush();
    while (Debug.available()) Debug.read();
    Debug.begin( debugBaud );
    
    // Initialize the Serial connection to the OSD data source:
    DataSource.begin( dataSourceBaud );
    
    
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    
    // Initialize the MAX7456 OSD:
    OSD.begin( NULL, NULL, NULL, &SPI );        // Use NTSC with no area.
    OSD.setSwitchingTime( 5 );                  // Set video croma distortion 
                                                //   to a minimum.
    OSD.setBlinkingTime( 3 );                   // Set blinking lapse time:
                                                //   [ 0-3 ] * 2 fields
    OSD.setBlinkingDuty( 3 );                   // Set blinking duty cycle:
                                                //   0: 50%,   1: 33%,
                                                //   2: 25%,   3: 75%
    OSD.setDefaultSystem( MAX7456_PAL );        // Output PAL video system until 
                                                //   a video source is provided.
    OSD.display();                              // Activate the text display.
    
    // Automatically get font decoding:
    byte          image[54];
    unsigned long fontCRC = 0;
    while (!OSD.readChar( 0x11, image ));       // Wait for read.
    for(char i = 0;
        i < 54; 
        i++        )
    {
      fontCRC =                                 // Do a checksum.
          _crc_xmodem_update( fontCRC,
                              image[i] );
    }
    if (fontCRC != 0x71CA &&
        fontCRC != 0xA8DD)
    {
      OSD.setCharEncoding(MAX7456_ASCII);
    }
    else
    {
      OSD.setCharEncoding(MAX7456_MAXIM);
    }
    
  }
  // setup()





// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    // Display screen saver until imput is recieved:
    byte          system     = NULL;
    float         angle      = 0.0;
    float         col        = 0.0;
    float         row        = 0.0;
    float         brightness = 0.0;
    byte          textmode   = 0;
    
    OSD.setTextOffset( MAX7456_HOS_MAX, 0 );    // Set initial offset.
    writeScreenSaver();                         // Write screen.
    
    while (!DataSource.available())
    {
      
      // Proper way of handling changes in video:
      checkLink();                              // Check connection at least
                                                //   one time each loop.
      if (OSD.videoIsBad())                     // If bad video caused reset,
      {
        while(OSD.resetIsBusy()) checkLink();   // wait for it to complete,
        system = ~NULL;                         // trigger refresh.
      }
      if (system != OSD.videoSystem())          // If user changes video input,
      {
        system = OSD.videoSystem();             // get new system,
        OSD.setDefaultSystem( system );         // update default system,
        writeScreenSaver();                     // rewrite screen,
        OSD.display();                          // activate the text display.
      }
      
      while (OSD.notInVSync()) checkLink();     // Wait for VSync to start to 
                                                //   prevent write artifacts.
      
      
      // Calculate and draw screensaver:
      angle += atan( 1.0 / MAX7456_HOS_MAX );   // At least 1 pixel/frame
      if (angle > 2 * PI)                       // Upon full rotation,
      {
        angle -= 2 * PI;                        // Reset angle
        textmode++;                             // Change write attributes
        if (textmode >= 8) textmode = 0;
      }

      col = MAX7456_HOS_MAX * cos( angle );     // Use the max h(x) offset and 
      row = MAX7456_VOS_MIN * sin( angle );     // the min v(y) offest to pick
                                                //   the radii of the oval.
      brightness = cos( 8 * angle ) * 2 + 2;
      
      OSD.setTextOffset( col, row );            // Abuse this function to 
                                                //   simulate motion.
      OSD.setWhiteLevel( brightness );          
      OSD.setBlackLevel( brightness );          
      
      OSD.setTextAttributes( textmode );        
      OSD.home();                               
      OSD.print( message );                     // Text must be rewritten to 
                                                // show with new attributes.
    }
    
    OSD.setWhiteLevel( 1 );                     // Restore normal parameters.
    OSD.setBlackLevel( 0 );                     
    OSD.home();                                 
    OSD.videoBackground();                      
    OSD.noBlink();                              
    OSD.normalColor();                          
    
    
    
    // Copy incoming serial data to screen:
    unsigned long timeout  = millis() + 600000; // Screensaver in 10 minutes.
    boolean       lastIsCR = false;             // Prevents ignore of repeated 
                                                //   carriage returns.
    
    OSD.setTextArea( NULL, NULL,                // Resize display for TV safe.
                     MAX7456_ACTIONSAFE );
    
    if (OSD.videoSystem() == MAX7456_NTSC)      // Adjust centering in TV.
    {
      OSD.setTextOffset( -4, 3 );
    }
    else
    {
      OSD.setTextOffset( 1, 1 );
    }
    
    OSD.lineWrap();                             // Set terminal behaviour.
    OSD.pageWrap();
    
    while (OSD.clearIsBusy());                  // Resize clears display, so
                                                //   wait for it to finish.    
    
    while (true) 
    {
      if (millis() > timeout) break;
      
      while (OSD.notInVSync()) checkLink();     // Wait for VSync to start to 
                                                //   prevent write artifacts.
      while (!OSD.notInVSync()) checkLink();    // Skip odd fields to remove
                                                //   mice teeth artifacts.
      while (OSD.notInVSync()) checkLink();     // Wait for next VSync.
      
      while (DataSource.available())
      {
        if (DataSource.peek() == '\n')          // Ignore newlines.
        {
          DataSource.read();
        }
        else if (DataSource.peek() == '\r')     // Upon carriage return,
        {
          DataSource.read();
          DataSource.write( '\n' );
          if (lastIsCR) OSD.write( ' ' );       // bump the cursor, and
          while (OSD.cursorColumn() != 0)       // until the next line,
          {
            OSD.write( ' ' );                   // write empty space.
          }
          lastIsCR = true;
        }
        else                                    // For all other characters,
        {
          OSD.write( DataSource.read() );       // just write them.
          lastIsCR = false;
        }
      }
      
      OSD.blink();
      OSD.cursor();
      OSD.noBlink();
    }
    
    OSD.home();
    
  } 
  // loop()





// Interrupt Methods ///////////////////////////////////////////////////////////





// Local Methods ///////////////////////////////////////////////////////////////
  
  void writeScreenSaver()
  {
    OSD.setTextArea( strlen(message), 1 );      // Set display size.
    while (OSD.clearIsBusy()) checkLink();      // Wait for clear to finish 
                                                //   (resize clears display).
    while (OSD.notInVSync()) checkLink();       // Wait for VSync to start to 
                                                //   prevent write artifacts.
    OSD.print( message );                       // Write screen.
  }
  
  
  // Handles connection problems with OSD.
  unsigned long errTime;
  void checkLink()
  {
    errTime = millis() + 110;                   // Soft reset safety.
    while (!OSD.status())                       // If connection fails, wait 
    {                                           //   for it to be corrected.
      if (millis() > errTime)                   //   This is a failsafe that 
      {                                         //   originated from the need 
        Debug.println( "Connection failure" );  //   to identify unreliable 
        errTime = millis() + 500;               //   solder connections.
      }
    }
  }

