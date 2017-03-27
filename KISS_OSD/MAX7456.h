/*
  MAX7456.h - MAX7456 On Screen Display Library for Arduino 
  Copyright (c) 2013 F. Robert Honeyman.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version 1.0 - 2013.01.31 - by F. Robert Honeyman <www.coldcoredesign.com>
*/

#ifndef _MAX7456_h
  #define _MAX7456_h



// Required Libraries //////////////////////////////////////////////////////////

  #include <stddef.h>   // typedef size_t
  #include <stdint.h>   // typedef [u]intN_t
  #include <stdbool.h>  // typedef bool
  #include <Print.h>
  #include <SPI.h>
  




// Global Macros ///////////////////////////////////////////////////////////////
  
  // Define method-related enumerations.
  #define MAX7456_ASCII      2    // Callnumber for ASCII encoding, used by
                                  //   setCharEncoding() 
  #define MAX7456_MAXIM      1    // Callnumber for MAXIM encoding, used by
                                  //   setCharEncoding() 
  #define MAX7456_NTSC       2    // Callnumber for NTSC system, used by 
                                  //   videoSystem(), setDefaultSystem()
  #define MAX7456_PAL        1    // Callnumber for PAL system, used by 
                                  //   videoSystem(), setDefaultSystem()
  #define MAX7456_INTSYNC    3    // Callnumber for internally generated video 
                                  //   sync, used by 
  #define MAX7456_EXTSYNC    2    // Callnumber for externally recieved video 
                                  //   sync, used by 
  #define MAX7456_AUTOSYNC   1    // Callnumber for automatically chosen video 
                                  //   sync, used by 
  #define MAX7456_CUR_FWD    ' '  // Callnumber for moving the cursor right,
                                  //   used by moveCursor()
  #define MAX7456_CUR_BACK   '\b' // Callnumber for moving the cursor left,
                                  //   used by moveCursor()
  #define MAX7456_CUR_UP     127  // Callnumber for moving the cursor up,
                                  //   used by moveCursor()
  #define MAX7456_CUR_DOWN   '\n' // Callnumber for moving the cursor down,
                                  //   used by moveCursor()
  #define MAX7456_CUR_HT     '\t' // Callnumber for moving the cursor to next
                                  //   horiz. tabstop, used by moveCursor()
  #define MAX7456_CUR_FF     '\f' // Callnumber for moving the cursor to the 
                                  //   home location, used by moveCursor()
  #define MAX7456_CUR_CR     '\r' // Callnumber for moving the cursor to the 
                                  //   start of line, used by moveCursor()

  // Define constant-related enumerations.
  #define MAX7456_FULLSCREEN 0    // FullScreen callnumber
  #define MAX7456_BORDERED   1    // Bordered callnumber
  #define MAX7456_ACTIONSAFE 2    // ActionSafe callnumber
  #define MAX7456_TITLESAFE  3    // TitleSafe callnumber

  // Define constant-related parameters.
  #define MAX7456_COLS_N0    30   // Columns in NTSC, FullScreen
  #define MAX7456_COLS_N1    28   // Columns in NTSC, Bordered
  #define MAX7456_COLS_N2    26   // Columns in NTSC, ActionSafe
  #define MAX7456_COLS_N3    24   // Columns in NTSC, TitleSafe
  #define MAX7456_ROWS_N0    13   // Rows in NTSC, FullScreen
  #define MAX7456_ROWS_N1    12   // Rows in NTSC, Bordered
  #define MAX7456_ROWS_N2    12   // Rows in NTSC, ActionSafe
  #define MAX7456_ROWS_N3    11   // Rows in NTSC, TitleSafe
  #define MAX7456_COLS_P0    30   // Columns in PAL, FullScreen
  #define MAX7456_COLS_P1    28   // Columns in PAL, Bordered
  #define MAX7456_COLS_P2    26   // Columns in PAL, ActionSafe
  #define MAX7456_COLS_P3    24   // Columns in PAL, TitleSafe
  #define MAX7456_ROWS_P0    16   // Rows in PAL, FullScreen
  #define MAX7456_ROWS_P1    15   // Rows in PAL, Bordered
  #define MAX7456_ROWS_P2    14   // Rows in PAL, ActionSafe
  #define MAX7456_ROWS_P3    13   // Rows in PAL, TitleSafe

// Define general parameters.
  #define MAX7456_HPX_C      12   // Character symbol width in number of 
                                  //   Horizontal Pixels
  #define MAX7456_VPX_C      18   // Character symbol height in number of 
                                  //   Vertical Pixels
  #define MAX7456_HPX_N      360  // Horizontal Pixels displayable by OSD 
                                  //   in NTSC mode
  #define MAX7456_VPX_N      234  // Vertical Pixels displayable by OSD
                                  //   in NTSC mode
  #define MAX7456_HPX_P      360  // Horizontal Pixels displayable by OSD 
                                  //   in PAL mode
  #define MAX7456_VPX_P      288  // Vertical Pixels displayable by OSD
                                  //   in PAL mode
  #define MAX7456_HOS_MAX    25   // Maximum horizontal offset in pixels
  #define MAX7456_HOS_MIN    -32  // Maximum horizontal offset in pixels
  #define MAX7456_VOS_MAX    15    // Maximum vertical offset in pixels
  #define MAX7456_VOS_MIN    -7   // Maximum vertical offset in pixels





// Class Definition ////////////////////////////////////////////////////////////

  class MAX7456 : public Print {
    
    public:
      
      const static uint8_t safeVideoCols[2][4];
      const static uint8_t safeVideoRows[2][4];
      //const static uint8_t defaultCodePage[128];
      
      
      
      MAX7456( 
          uint8_t chipSelectPin 
        );
        
      ~MAX7456();
      
      
      virtual float  version() { return 1.0; }
      
      bool           begin();
      bool           begin( 
                         uint8_t    cols, 
                         uint8_t    rows, 
                         uint8_t    area        = MAX7456_ACTIONSAFE, 
                         SPIClass * handleToSPI = &SPI
                       );
      
      void           end();
      
      void           display();
      void           noDisplay();
      void           video();
      void           noVideo();
      
      void           reset();
      bool           resetIsBusy();
      void           clear();
      bool           clearIsBusy();
      bool           videoIsBad();
      bool           status();
      bool           notInVSync();
      bool           notInHSync();
      bool           lossOfSync();
      
      bool           setSyncSource( 
                         uint8_t sync 
                       );
      
      bool           setSwitchingTime( 
                         uint8_t timing 
                       );
      
      /*bool           setBlinkingTime( 
                         uint8_t timing 
                       );
      
      bool           setBlinkingDuty( 
                         uint8_t timing 
                       );*/
      
      bool           setWhiteLevel( 
                         uint8_t brightness 
                       );
      bool           setWhiteLevel( 
                         uint8_t brightness, 
                         int8_t row 
                       );
      
      bool           setBlackLevel( 
                         uint8_t brightness 
                       );
      bool           setBlackLevel( 
                         uint8_t brightness, 
                         int8_t row 
                       );
      
      bool           setGrayLevel( 
                         uint8_t brightness 
                       );
      
      uint8_t        videoSystem();
      
      bool           setDefaultSystem( 
                         uint8_t system 
                       );
      
      bool           setTextOffset( 
                         int8_t xOffset, 
                         int8_t yOffset 
                       );
      bool           setTextArea( 
                         int8_t  cols, 
                         int8_t  rows, 
                         uint8_t area = MAX7456_ACTIONSAFE 
                       );
      
      bool           setCharEncoding( 
                         uint8_t charEncoding 
                       );

      void           videoBackground();
      void           grayBackground();
      void           blink();
      void           noBlink();
      void           normalColor();
      void           invertColor();
      bool           setTextAttributes( 
                         uint8_t attributes 
                       );
      
      void           lineWrap();
      void           noLineWrap();
      void           pageWrap();
      void           noPageWrap();
      bool           cursor();
      bool           noCursor();
      bool           home();
      bool           setCursor( 
                         int8_t col, 
                         int8_t row 
                       );
      bool           moveCursor( 
                         uint8_t mode
                       );
      bool           moveCursor( 
                         uint8_t mode, 
                         uint8_t distance
                       );
      
      size_t         write( 
                         uint8_t symbol 
                       );
                     using Print::write;

      bool           readChar( 
                         uint8_t ord, 
                         uint8_t image[] 
                       );
      bool           createChar( 
                         uint8_t ord, 
                         const uint8_t image[] 
                       );
      
      uint8_t        columns();
      uint8_t        rows();
      int8_t         cursorColumn();
      int8_t         cursorRow();
      
      
      
      
    private:
    
      uint8_t             deviceSelectPin;
      uint8_t             deviceSelectMask;
      volatile uint8_t  * deviceSelectPort;
      
      SPIClass *          SPIObject;

      unsigned long       resetFinishTime;
      
      uint8_t             syncSource;
      uint8_t             defaultSystem;
      uint8_t             charEncoding;
      uint8_t             videoMode;
      
      int8_t              offsetPixelsH;
      int8_t              offsetPixelsV;
      int8_t              shiftPixelsH;
      int8_t              shiftPixelsV;
      int8_t              shiftColumns;
      int8_t              shiftRows;
      int8_t              displayColumns;
      int8_t              displayRows;
      int8_t              cursorX;
      int8_t              cursorY;
      bool                wrapLines;
      bool                wrapPages;
      bool                cursorValid;
      bool                badIncrement;
      bool                begun;
      bool                inReset;
      bool                inAutoIncrement;
      bool                charTransfered;
      
      
      
      uint8_t send( 
                  uint8_t value 
                );
      uint8_t send( 
                  uint8_t registerAddress, 
                  uint8_t value 
                );
      uint8_t send( 
                  uint8_t registerAddress, 
                  uint8_t value, 
                  uint8_t bitOffest 
                );
      uint8_t send( 
                  uint8_t registerAddress, 
                  uint8_t value, 
                  uint8_t bitOffest, 
                  uint8_t valueLength 
                );
      uint8_t receive( 
                  uint8_t registerAddress 
                );
      
      uint8_t decode( 
                  uint8_t symbol 
                );
      void    increment();
      
  };



#endif


