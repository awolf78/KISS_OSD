/*
  VideoTimeInserter.ino
    
    Demonstrates a very basic on screen time display.


    This example code is in the public domain.
    Requires Arduino 1.0 or later.


    History :
      1.0 ~ Creation of VideoTimeInserter from MAX7456Demo 0.4
            2013.01.31 - F. Robert Honeyman <www.coldcoredesign.com>
      
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
    OSD.setSwitchingTime( 5 );                  // Set video croma distortion 
                                                //   to a minimum.
    //OSD.setCharEncoding( MAX7456_ASCII );       // Only needed if ascii font.
    OSD.display();                              // Activate the text display.
    
  }
  // setup()





// Main Code ///////////////////////////////////////////////////////////////////

  void loop() 
  {
    
    char date[] = "2012-01-01";
    char time[] = "00:00:00.000";
    
    int year = (date[0] - '0') * 1000
                 + (date[1] - '0') * 100
                 + (date[2] - '0') * 10
                 + (date[3] - '0');
    byte leap = isLeapYear(year);
    byte month = (date[5] - '0') * 10
                   + (date[6] - '0');
    byte day = (date[8] - '0') * 10
                 + (date[9] - '0');
    int msOffset = (time[11] - '0') * 100
                     + (time[10] - '0') * 10
                     + (time[9] - '0');
    char daysPerMonth[2][12] = { 
               { 31, 28, 31, 30, 31, 30, 
                 31, 31, 30, 31, 30, 31  },
               { 31, 29, 31, 30, 31, 30, 
                 31, 31, 30, 31, 30, 31  }
             };
    bool toggle = false;
    
    
    while (true)
    {
      
      while (OSD.notInVSync()) {

        time[11] = (millis() + msOffset)        // The millis() function keeps 
                     % 10 + '0';                //   the time even when not
        time[10] = (millis() + msOffset)        //   calculating it.
                     / 10 % 10 + '0';
        time[9] = (millis() + msOffset)
                     / 100 % 10 + '0';

        if (!toggle && time[9] == '5') {        // Converts millis() into a 
          toggle = true;                        //   pulse per second.
        }
        if (toggle && time[9] == '0') {
          time[7]++;                            // Increment on falling edge.
          toggle = false;
        }
        
        if(time[7] > '9') {
          time[7] = '0';
          time[6]++;
        }
        if(time[6] > '5') {
          time[6] = '0';
          time[4]++;
        }
        if (time[4] > '9') {
          time[4] = '0';
          time[3]++;
        }
        if (time[3] > '5') {
          time[3] = '0';
          time[1]++;
        }
        if (time[1] > '9') {
          time[1] = '0';
          time[0]++;
        }
        if (time[0] >= '2' && time[1] > '3') {
          time[1] = '0';
          time[0] = '0';
          date[9]++;
          day++;
        }
        
        if (date[9] > '9') {
          date[9] = '0';
          date[8]++;
        }
        
        if (day > daysPerMonth[leap][month - 1]) {
          date[9] = '1';
          date[8] = '0';
          day = 1;
          date[6]++;
          month++;
        }
        if (date[6] > '9') {
          date[6] = '0';
          date[5]++;
        }
        if (date[5] >= '1' && date[6] > '2') {
          date[6] = '1';
          date[5] = '0';
          month = 1;
          date[3]++;
          year++;
          leap = isLeapYear(year);
        }
        if (date[3] > '9') {
          date[3] = '0';
          date[2]++;
        }
        if (date[2] > '9') {
          date[2] = '0';
          date[1]++;
        }
        if (date[1] > '9') {
          date[1] = '0';
          date[0]++;
        }
        if (date[0] > '9') {
          date[0] = '0';
        }
        
        // Add aditional code here.
      }
      
      OSD.setCursor( 0, -1 );
      OSD.print( date );
      OSD.setCursor( 0 - strlen(time), -1 );
      OSD.print( time );
      
      while (!OSD.notInVSync())
      {
        // Add aditional code here.
      }
    }
    
  } 
  // loop()





// Interrupt Methods ///////////////////////////////////////////////////////////





// Local Methods ///////////////////////////////////////////////////////////////

  byte isLeapYear( int year ) 
  {
    if (year % 400 == 0) return 1;
    else if (year % 100 == 0) return 0;
    else if (year % 4 == 0) return 1;
    return 0;
  }
  //end isLeapYear()

