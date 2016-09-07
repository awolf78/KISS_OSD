/*
  UploadFont.ino
    
    Recieves an MCM file from an incoming XModem stream and uploads its
    contents into the MAX7456 Character Glyph Non-Volitile Memory.
    This can be done at 38400 baud with HyperTerminal or with Tera Term.
    
    
    This example code is in the public domain.
    Requires Arduino 1.0 or later.
    
    
    History :
      1.0   ~ Creation of UploadFont from SerialXModemEcho 0.3.
              2013.02.01 - by F. Robert Honeyman <www.coldcoredesign.com>

*/



// Included Libraries //////////////////////////////////////////////////////////
  
  #include <util/crc16.h>
  #include <SPI.h>
  #include <MAX7456.h>





// Pin Mapping /////////////////////////////////////////////////////////////////
  
  // pinValue = 0 means "not connected"

  //         FDTI Basic 5V                    ---  Arduino VCC      (AVCC,VCC)  
  //         FDTI Basic GND                   ---  Arduino GND      (AGND,GND)  
  //         FDTI Basic CTS                   ---  Arduino GND      (AGND,GND)  
  //         FDTI Basic DTR                   ---  Arduino GRN                  
  //         FDTI Basic TXO                   ---> Arduino TXO [PD0](RXD)       
  //         FDTI Basic RXI                  <---  Arduino RXI [PD1](TXD)       
  
  
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
  
  #define XMODEM_REFRESH      3000  // Millisecond delay between repeated 
                                    //   XMODEM_START sends
  #define XMODEM_REPLYTIMEOUT 50    // Milliseconds to wait between the send 
                                    //   of XMODEM_START and the reciept of the  
                                    //   begining of the first block
  #define XMODEM_FAILLAPSE    20    // Number of baudrate-length delays between 
                                    //   reception of last data in bad block 
                                    //   and XMODEM_NACK send
  #define XMODEM_NACK_CT      5     // Number of XMODEM_NACK send reattempts
                                    //   upon failure to reply
  #define XMODEM_TOTALTIMEOUT 30000 // Milliseconds to try to resolve bad 
                                    //   communication before cancel
  #define XMODEM_BS           '\b'  // Backspace character: sent to move back 
                                    //   over the last sent character
  #define XMODEM_ERASE        ' '   // Terminal erase character: sent to write
                                    //   empty space over characters
  #define XMODEM_START        'C'   // XModem protocol control character used 
                                    //   to signal that the recipiant is 
                                    //   ready to recieve transmission 
  #define XMODEM_SOH          0x01  // XModem protocol control character used 
                                    //   by the sender to mark the begining 
                                    //   of each block
  #define XMODEM_ACK          0x06  // XModem protocol control character used 
                                    //   to signal that the recipiant has 
                                    //   sucessfully recieved a block
  #define XMODEM_NACK         0x15  // XModem protocol control character used 
                                    //   to signal that the recipiant has 
                                    //   failed to recieve a block correctly
  #define XMODEM_CAN          0x18  // XModem protocol control character used 
                                    //   to forcefully cancel transmission
  #define XMODEM_CAN_CT       5     // Number of XMODEM_CANCEL in a row needed
                                    //   to forcefully cancel transmission
  #define XMODEM_EOFPAD       0x1A  // XModem protocol control character used 
                                    //   by the sender to mark null bytes at 
                                    //   the end of the last block
  #define XMODEM_EOT          0x04  // XModem protocol control character used 
                                    //   to signal the end of transmission





// Global Constants ////////////////////////////////////////////////////////////

  const unsigned long initBaudPC     = 38400;    // Initial Serial baud rate 
                                                 //   for the FDTI RS232 to USB 
  const unsigned long waitCycles     = 100000;   // Number of loops to wait  
                                                 //   before dumping last buffer





// Global Variables ////////////////////////////////////////////////////////////
  
  MAX7456 OSD( osdChipSelect );
  MAX7456 ExtCom = OSD;
  
  unsigned long ulRefreshTime  = 0;     // Temp timekeeper for periodically 
                                        //   sending XMODEM_START
  unsigned long ulTimeoutTime  = 0;     // Temp timekeeper for total timeout
  unsigned long ulReplyTime    = 0;     // Temp timekeeper for reply delay
  boolean       bValidSender   = false;
  boolean       bEndConfirmed  = false;
  byte          ubCancels      = 0;
  byte          ubNacks        = 0;
  byte          ubWrittenChars = 0;
  byte          ubMaxAvaliable = 0;     // Checks for overflow
  byte          ubValidBlock   = 0;
  byte          ubCurrentBlock = 0;
  byte          ubPageOffset   = 0;
  char          acInputPage[128];
  uint16_t      uiCurrentCRC   = 0x0000;

  uint16_t      uiFontCRC      = 0x0000;
  char          acHeader[]     = "MAX7456";
  byte          ubHeaderState  = 0;
  char          cBit           = 7;
  byte          ubByte         = 0;
  byte          ubOrd          = 0;
  byte          aubImage[54];





// Hardware Setup //////////////////////////////////////////////////////////////

  void setup() {
    
    // Open Serial session
    Serial.begin(initBaudPC);
    
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    
    // Initialize the MAX7456 OSD:
    OSD.begin();                                // Use NTSC with default area.
    OSD.setSwitchingTime( 3 );                  // Set video chroma distortion 
                                                //   to a minimum.
    OSD.display();                              // Activate the text display.
    
  } // setup()





// Main Code ///////////////////////////////////////////////////////////////////
  
  void loop() {
    
    // Instantiate local variables
    char          cIn         = '\0';
    uint16_t      uiValidCRC  = 0x0000;
    
    
    
    // A faster, more controlled loop structure
    while (true) 
    {
      
      // Show existing font:
      OSD.display();                            // Activate the text display.
      OSD.clear();
      OSD.home();
      OSD.setCharEncoding(MAX7456_ASCII);       // Use non-decoded access.
      OSD.noLineWrap();                         // Set wrap behaviour.
      OSD.noPageWrap();
      writeFont();
      
      
      
      
      // Clear terminal screen
      Serial.write( 0x1B );  // "ESC [ 2 J" Clears the whole screen
      Serial.print( "[2J" );
      Serial.write( 0x1B );  // "ESC [ 1 ; 1 H" Resets cursor
      Serial.write( "[1;1H" );
      
      // Show message to indicate begining
      Serial.write( '\n' );  
      Serial.write( '\r' );  
      Serial.print( "Awaiting MCM File via XModem-CRC... " );
      
      
      
      
      // Initialize variables
      XModemInitState:
        Serial.write( 0x1B );                 // "ESC [ s" Saves the current
        Serial.write( "[s" );                 //   cursor location
        ubWrittenChars = 0;                   // Reset number of control chars
                                              //   sent that need to be erased
        ulTimeoutTime =                       // Set absolute timeout
            millis() + XMODEM_TOTALTIMEOUT;
        
        
        
        
      // Request transmission start
      XModemStartState:
        bValidSender   = false;               // Assume not XModem sender
        bEndConfirmed  = false;               // Require resend to end file
        ubCancels      = 0;                   // Reset cancel count
        ubMaxAvaliable = 0;                   // Reset overflow
        ubValidBlock   = 0;                   // Reset last good block id
        ubCurrentBlock = 0;                   // Reset block count
        
        
        while (millis() < ulRefreshTime)      // While waiting for periodic 
        {                                     //   refresh, 
          if (millis() >= ulTimeoutTime)      // if absolute timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        ulRefreshTime =                       // Set next attempt to happen at 
            millis() + XMODEM_REFRESH;        //   XMODEM_REFRESH number of 
                                              //   milliseconds in the future.
        
        
        while (Serial.available())            // For any unexpected data still 
        {                                     //   in the buffer,
          if (Serial.peek() == XMODEM_SOH)    // if sender jumped the gun,
          {
            goto XModemHeaderState;           // start the header read.
          }
          
          cIn = cancelableRead(&Serial);      // retrieve char and update the
                                              //   cancel count
          if (ubCancels >= XMODEM_CAN_CT)     // upon recieving full cancel,
          {
            goto XModemCancelState;           // go to default cancel state;
          }
        }
        
        
        Serial.write( XMODEM_START );         // Send XMODEM_START.
        ubWrittenChars++;                     // Note the written char.
        
        
        ulReplyTime = millis() +              // Set interrupt time for 
            XMODEM_REPLYTIMEOUT;              //   recieve busy wait.
        
        while (Serial.available() == 0)       // While waiting for sender to  
        {                                     //   process XMODEM_START,
          if (millis() > ulReplyTime)         // if reply timeout,
          {
            
            while (ubWrittenChars > 0)        // if unused written chars,
            {
              Serial.write( XMODEM_BS );      // move back over each
              Serial.write( " " );            // erase each
              Serial.write( XMODEM_BS );      // move back over each
              ubWrittenChars--;
            }
            Serial.write( 0x1B );             // undo cursor movement
            Serial.write( "[u" );
            
            goto XModemStartState;            // reattempt start.
          }
        }
                                              // Transmission has started, so
                                              //   enter first recieve state 
                                              //   by falling through.
        
        
        
        
      // Recieve start of block
      XModemHeaderState:
        ubCurrentBlock = ubValidBlock + 1;    // Set expected block.
        uiCurrentCRC   = 0x0000;              // Reset checksum
        
        
        while (Serial.available() == 0)       // Wait for character, and 
        {
          if (millis() >= ulTimeoutTime)      // while waiting, if timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        ubMaxAvaliable =                      // Update peak buffer use var
            max( Serial.available(),          //   to identify overflows.
                 ubMaxAvaliable );
        
        
        cIn = cancelableRead(&Serial);        // Retrieve char, update the
                                              //   checksum and cancel count
        if (ubCancels >= XMODEM_CAN_CT)       // Upon recieving full cancel,
        {
          goto XModemCancelState;             // go to default cancel state.
        }
        
        
        if (cIn == XMODEM_EOT)                // If end of file, 
        {
          if (!bEndConfirmed) {               // confirm it by nack
            bEndConfirmed = true;
            goto XModemNackState;             // go to not acknowledged state;
          }
          else                                // and then if confirmed,
          {
            goto XModemAckState;              // go to not acknowledged state.
          }
        }
        if (cIn != XMODEM_SOH)                // If incorrect start of block, 
        {
          
          while (ubWrittenChars > 0)          // if unused written chars,
          {
            Serial.write( XMODEM_BS );        // move back over each
            Serial.write( " " );              // erase each
            Serial.write( XMODEM_BS );        // move back over each
            ubWrittenChars--;
          }
          Serial.write( 0x1B );               // undo cursor movement
          Serial.write( "[u" );
          
          goto XModemNackState;               // go to not acknowledged state
                                              //   to handle error.
        }
                                              // If transmission has started,
                                              //   enter block number recieve 
                                              //   state by falling through.
        
        
        
        
      // Recieve block number
      XModemBlocknumberState:
        while (Serial.available() == 0)       // Wait for character, and 
        {
          if (millis() >= ulTimeoutTime)      // while waiting, if timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        ubMaxAvaliable =                      // Update peak buffer use var
            max( Serial.available(),          //   to identify overflows.
                 ubMaxAvaliable );
        
        
        cIn = cancelableRead(&Serial);        // Retrieve char, update the
                                              //   checksum and cancel count
        if (ubCancels >= XMODEM_CAN_CT)       // Upon recieving full cancel,
        {
          goto XModemCancelState;             // go to default cancel state.
        }
        if ((byte)cIn != ubCurrentBlock)      // If not expected block,
        {
          goto XModemNackState;               // go to not acknowledged state
                                              //   to handle error.
        }
        
        
        while (Serial.available() == 0)       // Wait for character, and 
        {
          if (millis() >= ulTimeoutTime)      // while waiting, if timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        ubMaxAvaliable =                      // Update peak buffer use var
            max( Serial.available(),          //   to identify overflows.
                 ubMaxAvaliable );
        
        
        cIn = cancelableRead(&Serial);        // Retrieve char, update the
                                              //   checksum and cancel count
        if (ubCancels >= XMODEM_CAN_CT)       // Upon recieving full cancel,
        {
          goto XModemCancelState;             // go to default cancel state.
        }
        if ((255 - (byte)cIn)                 // If wrong reverse block value,
                != ubCurrentBlock)
        {
          goto XModemNackState;               // go to not acknowledged state
                                              //   to handle error.
        }
        else                                  // otherwise,
        {
          bValidSender = true;                // assume this means a real
        }
                                              //   XModem is sending, so enter 
                                              //   block data recieve state by 
                                              //   falling through.
        
        
        
        
      // Recieve data
      XModemRecieveState:
        if (ubMaxAvaliable > 60)              // Upon overflow,
        {
          goto XModemNackState;               // go to not acknowledged state
                                              //   to handle error.
        }
        
        for (int iDataCt = 0;                 // Until 128 bytes come in,
             iDataCt < 128;  )
        {
          while (Serial.available() &&
                 iDataCt < 128        )
          {
            cIn = summedRead(&Serial);        // retrieve char, update the
                                              //   checksum and cancel count
            if (ubCancels >= XMODEM_CAN_CT)   // upon recieving full cancel,
            {
              goto XModemCancelState;         // go to default cancel state;
            }
            
            
            acInputPage[iDataCt] = cIn;
            
            
            iDataCt++;                        // note the successful read
          }
          
          if (millis() >= ulTimeoutTime)      // upon timeout,
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        
        
      // Recieve and evaluate checksum
      XModemChecksumState:
        while (Serial.available() == 0)       // Wait for character, and 
        {
          if (millis() >= ulTimeoutTime)      // while waiting, if timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        cIn = cancelableRead(&Serial);        // Retrieve char and update the
                                              //   cancel count
        if (ubCancels >= XMODEM_CAN_CT)       // Upon recieving full cancel,
        {
          goto XModemCancelState;             // go to default cancel state.
        }
        
        
        uiValidCRC = (byte)cIn << 8;
        
        
        while (Serial.available() == 0)       // Wait for character, and 
        {
          if (millis() >= ulTimeoutTime)      // while waiting, if timeout, 
          {
            goto XModemCancelState;           // go to default cancel state.
          }
        }
        
        
        cIn = cancelableRead(&Serial);        // Retrieve char and update the
                                              //   cancel count
        if (ubCancels >= XMODEM_CAN_CT)       // Upon recieving full cancel,
        {
          goto XModemCancelState;             // go to default cancel state.
        }
        
        
        uiValidCRC += (byte)cIn;
        if (uiValidCRC != uiCurrentCRC)       // If data checksum is wrong,
        {
          goto XModemNackState;               // request resend.
        }
        
        
        
        
      // Process the data
      XModemProcessState:
        for (int li2n = 0; 
             li2n < 128;
             li2n++       )
        {
          if (acInputPage[li2n] ==              // Read through header.
                acHeader[ubHeaderState])
          {
            ubHeaderState++;
          }
          if (ubHeaderState ==                  // If end of header,
                strlen(acHeader))
          {
            if (acInputPage[li2n] == '0')
            {
              if (ubByte < 54)
                aubImage[ubByte] &= 
                    ~(1 << cBit);
              cBit--;
            }
            if (acInputPage[li2n] == '1')
            {
              if (ubByte < 54)
                aubImage[ubByte] |= 
                    1 << cBit;
              cBit--;
            }
            if (cBit < 0) {
              cBit = 7;
              ubByte++;
            }
            if (ubByte >= 64) {
              
              if (ubOrd > 0)                    // Never overwrite the blank.
              {
                while (!OSD.createChar(
                           ubOrd, aubImage ));  // Wait for write to complete.
                OSD.display();
              }
              
              ubByte = 0;
              ubOrd++;
            }
          }
        }
        
        
        
        
      // Reply success and ready for next block
      XModemAckState:
        ubValidBlock = ubCurrentBlock;        // Set current block as valid.
        
        do                                    // Checking at least once,
        {
          
          while (Serial.available())          // for any unexpected data 
          {                                   //   still in the buffer,
            cIn = cancelableRead(&Serial);    // retrieve char and update the
                                              //   cancel count
            if (ubCancels >= XMODEM_CAN_CT)   // upon recieving full cancel,
            {
              goto XModemCancelState;         // go to default cancel state;
            }
          }
          
          delayMicroseconds(                  // wait for sender to finish 
              1000000 / initBaudPC *          //   sending data
              XMODEM_FAILLAPSE);
          
          if (millis() >= ulTimeoutTime)      // upon timeout,
          {
            goto XModemCancelState;           // go to default cancel state;
          }
          
        }
        while (Serial.available());           // done while still sending.
        
        
        ubMaxAvaliable = 0;                   // Since buffer is clear, 
                                              //   clear the overflow count.
        
        Serial.write( XMODEM_ACK );           // Send acknowledged.
        
        
        ulTimeoutTime =                       // Reset absolute timeout
            millis() + XMODEM_TOTALTIMEOUT;
        
        
        if (bEndConfirmed) {
          goto XModemCancelState;             // go to default cancel state;
        }
          
        
        goto XModemHeaderState;               // Attempt to read in a new 
                                              //   header.
        
        
        
        
      // Reply that data was invalid
      XModemNackState:
        if (ubNacks > XMODEM_NACK_CT)         // upon excessive reattempt,
        {
          goto XModemCancelState;             // go to default cancel state;
        }
        
        do                                    // Checking at least once,
        {
          
          while (Serial.available())          // for any unexpected data 
          {                                   //   still in the buffer,
            cIn = cancelableRead(&Serial);    // retrieve char and update the
                                              //   cancel count
            if (ubCancels >= XMODEM_CAN_CT)   // upon recieving full cancel,
            {
              goto XModemCancelState;         // go to default cancel state;
            }
          }
          
          delayMicroseconds(                  // wait for sender to finish 
              1000000 / initBaudPC *          //   sending data
              XMODEM_FAILLAPSE);
          
          if (millis() >= ulTimeoutTime)      // upon timeout,
          {
            goto XModemCancelState;           // go to default cancel state;
          }
          
        }
        while (Serial.available());           // while still sending
        
        ubMaxAvaliable = 0;                   // Since buffer is clear, 
                                              //   clear the overflow count.
        
        
        Serial.write( XMODEM_NACK );          // Send not acknowledged.
        ubWrittenChars++;                     // Note the written char.
        
        
        ulReplyTime = millis() +              // Set interrupt time for 
            XMODEM_REPLYTIMEOUT;              //   recieve busy wait.
        
        while (Serial.available() == 0)       // While waiting for sender to  
        {                                     //   process XMODEM_NACK,
          if (millis() > ulReplyTime)         // if reply timeout,
          {
            
            while (ubWrittenChars > 0)        // if unused written chars,
            {
              Serial.write( XMODEM_BS );      // move back over each
              Serial.write( " " );            // erase each
              Serial.write( XMODEM_BS );      // move back over each
              ubWrittenChars--;
            }
            Serial.write( 0x1B );             // undo cursor movement
            Serial.write( "[u" );
            
            if (!bValidSender)                // If awaiting first block,
            {
              goto XModemStartState;          // return to transmission start
                                              //   because of uncertainty that
                                              //   sender was an XModem,
            }
            else                              // if not first block,
            {
              ubNacks++;                      // increment reattempt count
              goto XModemNackState;           // reattempt XMODEM_NACK send.
            }
            
          }
        }
        ubNacks = 0;
        
        goto XModemHeaderState;               // Attempt to read in a new 
                                              //   header.
        
        
        
      // Cancel transmission
      XModemCancelState:
        for (int iCt = 0; 
             iCt < XMODEM_CAN_CT; 
             iCt++               )
        {
          Serial.write( XMODEM_CAN );         // send XMODEM_CANCEL
          ubWrittenChars++;                   // update terminal clutter count
        }
        
        
        cleanTerminal( &Serial );             // Clear terminal clutter
                                              //   and undo cursor movement
        Serial.write( "            " );       // Erase error clutter
        Serial.write( 0x1B );                 // Undo cursor movement
        Serial.write( "[u" );
        
        
        ulReplyTime = millis() +              // Set interrupt time for 
            XMODEM_REPLYTIMEOUT;              //   recieve busy wait.
        
        while (millis() < ulReplyTime);       // Wait for send app to close
                                              // Repeat whole process by
                                              //   falling out of loop. 
      
      
      
    }
    // end while (true) 
    
    
  } // loop()





// Interrupt Functions /////////////////////////////////////////////////////////





// Local Functions /////////////////////////////////////////////////////////////
  
  // Removes terminal clutter caused by sending control codes.
  char cleanTerminal(Stream * Com) {
    
    for (int iCt = 0;                         // For each written char,
         iCt <= ubWrittenChars;
         iCt++                 )
    {
      Com->write( '\b' );                    // move back over each.
    }
    
    for (int iCt = 0;                         // For each written char,
         iCt <= ubWrittenChars;
         iCt++                 )
    {
      Com->write( ' ' );                     // erase each.
    }
    
    ubWrittenChars = 0;                       // Reset count.
    
    Com->write( 0x1B );                      // Undo cursor movement.
    Com->write( "[u" );
    
  } // cleanTerminal()
  
  
  
  // Reads in a byte from a stream and handles checksum and cancel.
  inline char summedRead(Stream * Com) {
    
    char cIn = cancelableRead(Com);
    
    uiCurrentCRC =                            // Update checksum.
        _crc_xmodem_update( uiCurrentCRC, 
                            (byte)cIn);
    
    return cIn;
    
  } // summedRead()
  
  
  
  // Reads in a byte from a stream and handles cancel.
  char cancelableRead(Stream * Com) {
    
    char cIn = Com->read();
    
    if (cIn == XMODEM_CAN)                    // If cancel command found,
    {
      //ubCancels++;                            // total the cancels
    }
    else                                      // otherwise, 
    {
      ubCancels = 0;                          // start over if not a full 
    }                                         //   cancel sequence.
    
    return cIn;
    
  } // cancelableRead()
  
  
  
  // Sends hex representation of integers to Serial.
  inline void printHex( Stream * Com, long value ) {
    
    printHex( Com, (unsigned long)value );
    
  }
  inline void printHex( Stream * Com, unsigned long value ) {
    
    printHex( Com, (byte)((value >> 24) & 0xFF) );
    printHex( Com, (byte)((value >> 16) & 0xFF) );
    printHex( Com, (byte)((value >> 8) & 0xFF) );
    printHex( Com, (byte)(value & 0xFF) );
    
  }
  inline void printHex( Stream * Com, int value ) {
    
    printHex( Com, (unsigned int)value );
    
  }
  inline void printHex( Stream * Com, unsigned int value ) {
    
    printHex( Com, (byte)((value >> 8) & 0xFF) );
    printHex( Com, (byte)(value & 0xFF) );
    
  }
  inline void printHex( Stream * Com, char value ) {
    
    printHex( Com, (byte)value );
    
  }
  void printHex( Stream * Com, byte value ) {
    
    char ubNib = 0;
    
    ubNib = (value >> 4) & 0xF;    // Get upper nibble
    if (ubNib < 10) ubNib += '0';  // and convert it to
    else ubNib += 'A' - 10;        // hex as ASCII.
    Com->write( ubNib );
    
    ubNib = value & 0x0F;          // Get lower nibble
    if (ubNib < 10) ubNib += '0';  // and convert it to
    else ubNib += 'A' - 10;        // hex as ASCII.
    Com->write( ubNib );
    
  } // printHex()
  
  
  
  // Displays the full font on screen.
  void writeFont() {
    
    // Instantiate local variables:
    byte ubSymbol = 0;

    
    // Loop through symbols and display them:
    while (OSD.notInVSync());                   // Wait for VSync to start
    OSD.clear();                                // Clear display
    
    for (int iRow = 0;                          // Write the last 80 symbols 
         iRow < 0 + 5;                          //   in two blocks, writing
         iRow++       )                         //   40 in the leftmost block
    {
      OSD.setCursor( -8, iRow );
      for (int iCol = -8;                       // and let it contain 5 rows 
           iCol < (-8) + 8;                     //   of 8 columns each, 
           iCol++)                              //   starting at (-8, 0)
      {
        ubSymbol = (iRow - 0 + 11) * 16         // given the row and column, 
                       + (iCol - (-8) + 0);     //   derive symbol index
        OSD.write( ubSymbol );                  // write symbol to display
      }
    }
    
    for (int iRow = 0;                          // Write the first 112 symbols 
         iRow < 7;                              //   in a block of 7 rows
         iRow++       )
    {
      OSD.setCursor( 0, iRow );
      for (int iCol = 0;                        // by 16 columns, starting at 
           iCol < 16;                           //   (0, 0)
           iCol++       )
      {
        ubSymbol = iRow * 16                    // given the row and column, 
                       + iCol;                  //   derive symbol index
        OSD.write( ubSymbol );                  // write symbol to display
      }
    }
    
    for (int iRow = -4;                         // Write the mid 64 symbols 
         iRow < -4 + 4;                         //   in a block of 4 rows
           iRow++        )
    {
      OSD.setCursor( 0, iRow );
      for (int iCol = 0;                        // by 16 columns, starting at 
           iCol < 16;                           //   (0, 0)
           iCol++       )
      {
        ubSymbol = (iRow - (-4) + 7) * 16       // given the row and column, 
                       + iCol;                  //   derive symbol index
        OSD.write( ubSymbol );                  // write symbol to display
      }
    }
    
    for (int iRow = -5;                         // Write the last 80 symbols 
         iRow < -5 + 5;                         //   in two blocks, writing
         iRow++        )                        //   40 in the rightmost block
    {
      OSD.setCursor( -8, iRow );
      for (int iCol = -8;                       // and let it contain 5 rows 
           iCol < (-8) + 8;                     //   of 8 columns each, 
           iCol++          )                    //   starting at (-8, -5)
      {
        ubSymbol = (iRow - (-5) + 11) * 16      // given the row and column, 
                       + (iCol - (-8) + 8);     //   derive symbol index
        OSD.write( ubSymbol );                  // write symbol to display
      }
    }
    
    OSD.display();                              // Display text
    while (!OSD.notInVSync());                  // Wait for VSync to finish
    
  } // writeFont()



