/*
  MAX7456.cpp - MAX7456 On Screen Display Library for Arduino 
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



// Required Libraries //////////////////////////////////////////////////////////

  #include <Print.h>
  #include <SPI.h>
  #include "MAX7456.h"
  




// Private Macros //////////////////////////////////////////////////////////////

  // Define general parameters.
  //#define DEBUG true
  
  #define MAX7456_TIME_CLR 20
  #define MAX7456_TIME_RST 110
  #define MAX7456_TIME_PWR 50000

  #define MAX7456_RRAO     0x80  // Register Receive Address Offset (this is 
                                 //   added to each of the values below to
                                 //   form the receive address location value)
  
  #define MAX7456_HOS_ZERO 0x20  // Horizontal Offset Zero point
  #define MAX7456_HOS_WID  6     // Horizontal Offset bit Width
  #define MAX7456_VOS_ZERO 0x10  // Vertical Offset Zero point
  #define MAX7456_VOS_WID  5     // Vertical Offset bit Width

  #define MAX7456_COLS_D   30    // Display Memory number of columns
  #define MAX7456_ROWS_D   16    // Display Memory number of rows
  
  
  // Define register addresses.
  //   ex: u1status = receive(MAX7456_STAT_R);
  //   ex: send(MAX7456_DMM_W, 0);
  #define MAX7456_VM0_R   0x80  // Video Mode 0 read
  #define MAX7456_VM0_W   0x00  // Video Mode 0 write
  #define MAX7456_VM1_R   0x81  // Video Mode 1 read
  #define MAX7456_VM1_W   0x01  // Video Mode 1 write
  #define MAX7456_HOS_R   0x82  // Horizontal Offset read
  #define MAX7456_HOS_W   0x02  // Horizontal Offset write
  #define MAX7456_VOS_R   0x83  // Vertical Offset read
  #define MAX7456_VOS_W   0x03  // Vertical Offset write
  #define MAX7456_DMM_R   0x84  // Display Memory Mode read
  #define MAX7456_DMM_W   0x04  // Display Memory Mode write
  #define MAX7456_DMAH_R  0x85  // Display Memory Address High read
  #define MAX7456_DMAH_W  0x05  // Display Memory Address High write
  #define MAX7456_DMAL_R  0x86  // Display Memory Address Low read
  #define MAX7456_DMAL_W  0x06  // Display Memory Address Low write
  #define MAX7456_DMDI_R  0x87  // Display Memory Data In read
  #define MAX7456_DMDI_W  0x07  // Display Memory Data In write
  #define MAX7456_CMM_R   0x88  // Character Memory Mode read
  #define MAX7456_CMM_W   0x08  // Character Memory Mode write
  #define MAX7456_CMAH_R  0x89  // Character Memory Address High read
  #define MAX7456_CMAH_W  0x09  // Character Memory Address High write
  #define MAX7456_CMAL_R  0x8A  // Character Memory Address Low read
  #define MAX7456_CMAL_W  0x0A  // Character Memory Address Low write
  #define MAX7456_CMDI_R  0x8B  // Character Memory Data In read
  #define MAX7456_CMDI_W  0x0B  // Character Memory Data In write
  #define MAX7456_OSDM_R  0x8C  // OSD Insertion Mux read
  #define MAX7456_OSDM_W  0x0C  // OSD Insertion Mux write
  #define MAX7456_RB0_R   0x90  // Row 0 Brightness read
  #define MAX7456_RB0_W   0x10  // Row 0 Brightness write
  #define MAX7456_RB1_R   0x91  // Row 1 Brightness read
  #define MAX7456_RB1_W   0x11  // Row 1 Brightness write
  #define MAX7456_RB2_R   0x92  // Row 2 Brightness read
  #define MAX7456_RB2_W   0x12  // Row 2 Brightness write
  #define MAX7456_RB3_R   0x93  // Row 3 Brightness read
  #define MAX7456_RB3_W   0x13  // Row 3 Brightness write
  #define MAX7456_RB4_R   0x94  // Row 4 Brightness read
  #define MAX7456_RB4_W   0x14  // Row 4 Brightness write
  #define MAX7456_RB5_R   0x95  // Row 5 Brightness read
  #define MAX7456_RB5_W   0x15  // Row 5 Brightness write
  #define MAX7456_RB6_R   0x96  // Row 6 Brightness read
  #define MAX7456_RB6_W   0x16  // Row 6 Brightness write
  #define MAX7456_RB7_R   0x97  // Row 7 Brightness read
  #define MAX7456_RB7_W   0x17  // Row 7 Brightness write
  #define MAX7456_RB8_R   0x98  // Row 8 Brightness read
  #define MAX7456_RB8_W   0x18  // Row 8 Brightness write
  #define MAX7456_RB9_R   0x99  // Row 9 Brightness read
  #define MAX7456_RB9_W   0x19  // Row 9 Brightness write
  #define MAX7456_RB10_R  0x9A  // Row 10 Brightness read
  #define MAX7456_RB10_W  0x1A  // Row 10 Brightness write
  #define MAX7456_RB11_R  0x9B  // Row 11 Brightness read
  #define MAX7456_RB11_W  0x1B  // Row 11 Brightness write
  #define MAX7456_RB12_R  0x9C  // Row 12 Brightness read
  #define MAX7456_RB12_W  0x1C  // Row 12 Brightness write
  #define MAX7456_RB13_R  0x9D  // Row 13 Brightness read
  #define MAX7456_RB13_W  0x1D  // Row 13 Brightness write
  #define MAX7456_RB14_R  0x9E  // Row 14 Brightness read
  #define MAX7456_RB14_W  0x1E  // Row 14 Brightness write
  #define MAX7456_RB15_R  0x9F  // Row 15 Brightness read
  #define MAX7456_RB15_W  0x1F  // Row 15 Brightness write
  #define MAX7456_OSDBL_W 0x6C  // OSD Black Level write
  #define MAX7456_OSDBL_R 0xEC  // OSD Black Level read
  #define MAX7456_STAT_R  0xA0  // Status read
  #define MAX7456_DMDO_R  0xB0  // Display Memory Data Out read
  #define MAX7456_CMDO_R  0xC0  // Character Memory Data Out read

  // Define configuration register masks.
  // Video Mode 0:
  //   ex: send(MAX7456_VM0_W, videoMode);
  #define MAX7456_VM0_PAL      6  // 1 bit;  1: Use PAL video system,
                                  //         0: Use NTSC video system
  #define MAX7456_VM0_FIXSYNC  5  // 1 bit;  1: Use VM0_INTSYNC setting,
                                  //         0: Automatically select sync source
  #define MAX7456_VM0_INTSYNC  4  // 1 bit;  1: If fixed, use internal sync,
                                  //         0: If fixed, use external sync
  #define MAX7456_VM0_OSDON    3  // 1 bit;  1: Draw OSD to screen,
                                  //         0: Leave screen blank
  #define MAX7456_VM0_SYNCV    2  // 1 bit;  1: Enable OSD at the next VSYNC,
                                  //         0: Enable OSD immediately
  #define MAX7456_VM0_SRESET   1  // 1 bit;  1: Clear registers & OSD (100us),
                                  //         0: (set automatically)
  #define MAX7456_VM0_BUFOFF   0  // 1 bit;  1: Disable video buffer,
                                  //         0: Enable video buffer
  // Video Mode 1:
  //   ex: send(MAX7456_VM1_W, 1, MAX7456_VM1_BT, MAX7456_VM1_BT_LEN);
  #define MAX7456_VM1_GRAYBG   7  // 1 bit;  1: Use all gray background,
                                  //         0: Use transparent background
  #define MAX7456_VM1_GLVL     4  // 3 bit;  Gray BG mode brightness
  #define MAX7456_VM1_GLVL_LEN       3  //      [ 0-7 ]
                                  //          = [value] * 7% of white level
  #define MAX7456_VM1_BT       2  // 2 bit;  Blinking time
  #define MAX7456_VM1_BT_LEN         2  //      [ 0-3 ]
                                  //          = [value] * 2 fields
  #define MAX7456_VM1_BDC      0  // 2 bit;  Blinking duty cycle;
  #define MAX7456_VM1_BDC_LEN        2  //   [ 0,1,2,3 ]
                                  //         0: 50%,   1: 33%,
                                  //         2: 25%,   3: 75%
  // Display Memory Mode:
  //   ex: send(MAX7456_DMM_W, 1, MAX7456_DMM_INV);
  #define MAX7456_DMM_OM       6  // 1 bit;  1: Set Operating Mode to require an 
                                  //            attribute byte to be written for 
                                  //            each character writen to display,
                                  //         0: Copy DMM attribute bits to the 
                                  //            display attribute byte for each 
                                  //            character writen to display
  #define MAX7456_DMM_LBC      5  // 1 bit;  1: Set Local Background Control to 
                                  //            gray background,
                                  //         0: Use transparent background
  #define MAX7456_DMM_BLK      4  // 1 bit;  1: Blink given VM1[BT,BDC]
                                  //         0: Do not blink
  #define MAX7456_DMM_INV      3  // 1 bit;  1: Invert black and white pixels,
                                  //         0: Normal font color displayed
  #define MAX7456_DMM_CDM      2  // 1 bit;  1: Clear Display Memory, 
                                  //         0: (set automatically)
  #define MAX7456_DMM_VSC      1  // 1 bit;  1: Wait For VSync to do DMM[CDM], 
                                  //         0: Allow DMM[CDM] to be immediate
  #define MAX7456_DMM_AI       0  // 1 bit;  1: Automatically Increment the 
                                  //            display memory character address,
                                  //         0: Require the display memory 
                                  //            character address each time
  // Row Brightness 0 to 15:
  //   ex: send(MAX7456_RB0_W, 3, MAX7456_RBn_CWL, MAX7456_RBn_CWL_LEN);
  #define MAX7456_RBn_CBL      2  // 2 bit;   Character Black Level
  #define MAX7456_RBn_CBL_LEN        2  //      [ 0-3 ]
                                  //          = [value] * 10% of white level
  #define MAX7456_RBn_CWL      0  // 2 bit;   Character White Level;
  #define MAX7456_RBn_CWL_LEN        2  //   [ 0,1,2,3 ]
                                  //         0: 120%,  1: 100%,
                                  //         2: 90%,   3: 80%
  // On Screen Display Black Level:
  //   ex: send(MAX7456_OSDBL_W, 0, MAX7456_OSDBL_AUTO);
  #define MAX7456_OSDBL_MAN    4  // 1 bit;  1: Disable automatic control,
                                  //         0: Enable automatic control
  // Status:
  //   ex: ubReady = receive( MAX7456_STAT_R ) & 1<<MAX7456_STAT_RBUSY;
  #define MAX7456_STAT_RBUSY   6  // 1 bit;  1: In process of resetting,
                                  //         0: Reset process is complete
  #define MAX7456_STAT_CMBUSY  5  // 1 bit;  1: Character memory is unavaliable,
                                  //         0: Character memory is avaliable
  #define MAX7456_STAT_NVSYNC  4  // 1 bit;  1: Sending video lines,
                                  //         0: Sending VSYNC
  #define MAX7456_STAT_NHSYNC  3  // 1 bit;  1: Sending video pixels,
                                  //         0: Sending HSYNC
  #define MAX7456_STAT_LOS     2  // 1 bit;  1: Loss of sync,
                                  //         0: Sync is active
  #define MAX7456_STAT_NTSC    1  // 1 bit;  1: NTSC system is detected at VIN,
                                  //         0: NTSC system isn't detected at VIN
  #define MAX7456_STAT_PAL     0  // 1 bit;  1: PAL system is detected at VIN,
                                  //         0: PAL system isn't detected at VIN






// Class Public Constants //////////////////////////////////////////////////////





// Class Private Constants /////////////////////////////////////////////////////

  // These can be used in place of the macros to select a safe number of 
  // columns and rows to display. Use like so:
  //   uint8_t rows = OSD.safeVideoRows[encoding][safety];
  //   uint8_t cols = OSD.safeVideoCols[encoding][safety];
  const uint8_t MAX7456::safeVideoCols[2][4] = { { MAX7456_COLS_N0, 
                                                   MAX7456_COLS_N1,  
                                                   MAX7456_COLS_N2,  
                                                   MAX7456_COLS_N3  },
                                                 { MAX7456_COLS_P0, 
                                                   MAX7456_COLS_P1,  
                                                   MAX7456_COLS_P2,  
                                                   MAX7456_COLS_P3  } };
  const uint8_t MAX7456::safeVideoRows[2][4] = { { MAX7456_ROWS_N0, 
                                                   MAX7456_ROWS_N1,  
                                                   MAX7456_ROWS_N2,  
                                                   MAX7456_ROWS_N3  },
                                                 { MAX7456_ROWS_P0, 
                                                   MAX7456_ROWS_P1,  
                                                   MAX7456_ROWS_P2,  
                                                   MAX7456_ROWS_P3  } };
  
  // This is code page remapping array for converting ASCII encoded
  // characters to the Maxim default character map.
  /*const uint8_t MAX7456::defaultCodePage[128] = 
                 {    0,    0,    0,    0,    0,    0,    0,    0,
                      0,    0,    0,    0,    0,    0,    0,    0,
                      0,    0,    0,    0,    0,    0,    0,    0,
                      0,    0,    0,    0,    0,    0,    0,    0,
                   0x00,    0, 0x48,    0,    0,    0,    0, 0x46,
                   0x3F, 0x40,    0,    0, 0x45, 0x49, 0x41, 0x47,
                   0x0A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x44, 0x43, 0x4A,    0, 0x4B, 0x42,
                   0x4C, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
                   0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                   0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
                   0x22, 0x23, 0x24,    0,    0,    0,    0,    0,
                      0, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
                   0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
                   0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
                   0x3C, 0x3D, 0x3E,    0,    0,    0,    0,    0  };*/





// Constructors ////////////////////////////////////////////////////////////////
  
  // Create new MAX7456 object. The variable chipSelectPin refers to the 
  // arduino digital pin number that is attached to the MAX7456's ~CS pin. It 
  // is assumed that the SPI object is initialized with the pin values for 
  // connecting MOSI, MISO, and SCK to SDIN, SDOUT, and SCLK.
  MAX7456::MAX7456(
               uint8_t chipSelectPin 
             ) 
  {
    
    // Import arguments to private member variables:
    deviceSelectPin  = chipSelectPin;
    
    
    // Set private member variables defaults:
    deviceSelectMask = 0x00;
    deviceSelectPort = NULL;
    SPIObject        = NULL;
    
    resetFinishTime = micros() +                // Assume reset until end of 
                          MAX7456_TIME_PWR;     //   power on reset time.
    
    syncSource       = MAX7456_AUTOSYNC;
    defaultSystem    = MAX7456_NTSC;
    charEncoding     = MAX7456_MAXIM;
  
    offsetPixelsH    = 0;
    offsetPixelsV    = 0;
    shiftPixelsH     = 0;
    shiftPixelsV     = 0;
    shiftColumns     = 0;
    shiftRows        = 0;
    displayColumns   = MAX7456_COLS_P0;
    displayRows      = MAX7456_ROWS_P0;
    cursorX          = 0;
    cursorY          = 0;
    videoMode        = 0;
    wrapLines        = false;
    wrapPages        = false;

    cursorValid      = false;                   // Prevents write() until after 
                                                //   begin() has configured the 
                                                //   CS pin.
    badIncrement     = true;                    // Safeguard for autoincrement.
    begun            = false;                   // Prevents all other functions
                                                //   from calling send() until 
                                                //   begin() has configured the 
                                                //   CS pin.
    inReset          = true;                    // Assume hardware reset.
    inAutoIncrement  = false;                   // Used to safeguard any 
                                                //   interrupt from write().
    charTransfered   = false;                   // Contains the state flag 
                                                //   for the NVM.

  }
  // end MAX7456()





// Destructor //////////////////////////////////////////////////////////////////
  
  // Frees the memory associated with the MAX7456 object.
  MAX7456::~MAX7456() 
  {
    
    end();
    
  }
  // end ~MAX7456()





// Public Methods //////////////////////////////////////////////////////////////
  
  // Recieve SPI object and initialize the required video configurations. This 
  // requires an SPIClass-compatible SPI library that supports calling the 
  // functions: setBitOrder() and transfer(); it also requires the library to 
  // specify the macro: MSBFIRST. The user must configure the SPI object to 
  // operate at more than 1MHz, but less than 10MHz.
  bool MAX7456::begin() 
  {
    return begin( NULL, NULL );                 // Default to Actionsafe NTSC.
  }
  bool MAX7456::begin( 
                    uint8_t    cols, 
                    uint8_t    rows, 
                    uint8_t    area, 
                    SPIClass * handleToSPI 
                  ) 
  {
    
    // Instantiate local variables:
    bool bSuccess  = true;
    
    
    // Initialize the data direction for the pins:
    pinMode( deviceSelectPin, OUTPUT );
    
    // Get port addresses and masks to bypass digitalWrite():
    deviceSelectMask = 
        digitalPinToBitMask(deviceSelectPin);
    deviceSelectPort = 
        portOutputRegister(
            digitalPinToPort(deviceSelectPin)
          );
    
    // Set initial pin values:
    *deviceSelectPort |= deviceSelectMask;      // Leave pin high (slave is  
                                                //   selected when low).
    
    
    // Copy to private variables:
    SPIObject = handleToSPI;
    
    
    // Initialize the SPI connection to the MAX7456 OSD:
    SPIObject->setBitOrder( MSBFIRST );         // Transfer data with the most 
                                                //   significant bit first.
    
    
    // Handle first beginning events:
    if (!begun)                                 // If first attempt at begin(), 
    {
      begun = true;                             // note port initialized,
      while (receive( MAX7456_STAT_R ) == 0);   // wait until it replies,
      while (receive( MAX7456_STAT_R )          // wait while reset,
                  & (1 << MAX7456_STAT_RBUSY));
      inReset = false;                          // reverse reset assumption,
    }
                                                //   Otherwise, if still busy 
                                                //   with reset, begin() will 
                                                //   fail within setTextArea().
    
    
    // Configure video settings:
    bSuccess &= setTextArea( cols,              // Set displayColumns, 
                             rows,              //   displayRows, shiftColumns, 
                             area  );           //   shiftRows, shiftPixelsH, 
                                                //   shiftPixelsV, send offsets 
                                                //   to OSD, and return if it 
                                                //   succeeded.
    
    send( MAX7456_OSDBL_W,                      // Set black level control to 
          0,                                    //   automatically adjust.
          MAX7456_OSDBL_MAN );
    
    cursorValid = bSuccess;                     // Enable write() if success.
    
    return bSuccess;
    
  }
  // end begin()
  
  
  
  
  // Cleanup anything before object handle being freed.
  void MAX7456::end() 
  {
    
    if (!begun) return;
    
    reset();
    
  }
  // end end()
  
  
  
  
  // Sets the OSD to enabled.
  void MAX7456::display() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    videoMode |= 1 << MAX7456_VM0_OSDON;        // Enable display.
    
    send( MAX7456_VM0_W, videoMode );           // Update mode.

  }
  // end display()
  
  
  
  
  // Sets the OSD to disabled.
  void MAX7456::noDisplay() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    videoMode &= ~(1 << MAX7456_VM0_OSDON);     // Disable display.
    
    send( MAX7456_VM0_W, videoMode );           // Update mode.

  }
  // end noDisplay()
  
  
  
  
  // Sets the video output to pass through buffer (enabled).
  void MAX7456::video() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    videoMode &= ~(1 << MAX7456_VM0_BUFOFF);    // Enable video.
    
    send( MAX7456_VM0_W, videoMode );           // Update mode.
    
  }
  // end video()
  
  
  
  
  // Sets the video output to high impedance (disabled).
  void MAX7456::noVideo() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    videoMode |= 1 << MAX7456_VM0_BUFOFF;       // Disable video.
    
    send( MAX7456_VM0_W, videoMode );           // Update mode.
    
  }
  // end noVideo()
  
  
  
  
  // Executes a software reset. Requires begin() to resume use.
  void MAX7456::reset() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset implies reset.
    
    noInterrupts();
    
    send( MAX7456_VM0_W,                        // Send reset.
          1 << MAX7456_VM0_SRESET );
    
    inReset = true;
    resetFinishTime = micros() +                // Start countdown.
                          MAX7456_TIME_RST;
    videoMode = 0;                              // Reset mode.
    cursorValid = false;                        // Lock out write().
    
    interrupts();
    
  }
  // end reset()
  
  
  
  
  // Returns false when reset() operation is complete.
  bool MAX7456::resetIsBusy() 
  {
    
    if (!inReset && begun) return false;        // Prevents micros() rollover 
                                                //   error and boosts speed,
                                                //   but still allows check 
                                                //   before begin().
    
    if (micros() <= resetFinishTime)
    {
      return true;
    }
    
    if (!begun) return false;                   // Implies hard reset done and 
                                                //   prevents receive() 
                                                //   without begin().
    
    byte u1stat = receive( MAX7456_STAT_R );
    
    if ( u1stat == 0x00 ||                      // If not responding, 
         u1stat & (1 << MAX7456_STAT_RBUSY))    // or if reset is busy,
    {
      return true;                              // assume reset in progress.
    }
    
    send( MAX7456_OSDBL_W,                      // Reset black level control 
          0,                                    //   to automatically adjust.
          MAX7456_OSDBL_MAN );
    
    inReset = false;
    
    return false;
    
  }
  // end resetIsBusy()
  
  
  
  
  // Clears the OSD display area.
  void MAX7456::clear() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset implies clear.
    
    send( MAX7456_DMM_W,                        // Begins display clearing.
          1, 
          MAX7456_DMM_CDM );
    
  }
  // end clear()
  
  
  
  
  // Returns false when clear() operation is complete.
  bool MAX7456::clearIsBusy() 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return true;             // Reset implies clear busy.
    
    if (receive( MAX7456_DMM_R ) 
            & (1 << MAX7456_DMM_CDM))
    {
      return true;
    }
    return false;
    
  }
  // end clearIsBusy()
  
  
  
  
  // Returns true if bad video tripped a reset.
  bool MAX7456::videoIsBad() 
  {
    
    if (inReset) return false;
    
    inReset = true;                             // Force reset to query.
    
    inReset = resetIsBusy();                    // Update bypass variable.
    
    return inReset;
    
  }
  // end resetIsBusy()
  
  
  
  
  // Returns true if device has been initialized, is not in reset, and is 
  // operating normally. Used to identify bad connections at runtime.
  bool MAX7456::status() 
  {
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents receive.
    
    uint8_t statReg = receive( MAX7456_STAT_R );
    
    if (statReg > 0) return true;
    else return false;
    
  }
  // end status()
  
  
  
  
  // Returns false at the beginning of the vertical sync.
  bool MAX7456::notInVSync() 
  {
    
    if (!begun) return true;
    
    if (resetIsBusy()) return true;             // Reset implies default.
    
    if (receive( MAX7456_STAT_R ) 
            & (1 << MAX7456_STAT_NVSYNC))
    {
      return true;
    }
    return false;
    
  }
  // end notInVSync()
  
  
  
  
  // Returns false at the beginning of the horizontal sync.
  bool MAX7456::notInHSync() 
  {
    
    if (!begun) return true;
    
    if (resetIsBusy()) return true;             // Reset implies default.
    
    if (receive( MAX7456_STAT_R ) 
            & (1 << MAX7456_STAT_NHSYNC))
    {
      return true;
    }
    return false;
    
  }
  // end notInHSync()
  
  
  
  
  // Returns true upon missing 32 lines of video input.
  bool MAX7456::lossOfSync() 
  {
    
    if (!begun) return true;
    
    if (resetIsBusy()) return true;             // Reset implies default.
    
    if (receive( MAX7456_STAT_R ) 
            & (1 << MAX7456_STAT_LOS))
    {
      return true;
    }
    return false;
    
  }
  // end lossOfSync()
  
  
  
  
  // Sets the source of video synchronization. Internal sync will force the 
  // on screen display to show without any incoming video. External sync will 
  // only output video when usable video is sent in. Auto sync will show the 
  // on screen display and overlay it onto incoming video if avaliable.
  bool MAX7456::setSyncSource( 
                    uint8_t sync 
                  ) 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    // Clean and handle input:
    if (sync == MAX7456_INTSYNC)
    {
      videoMode |= 1 << MAX7456_VM0_FIXSYNC;    // Set sync to fixed.
      videoMode |= 1 << MAX7456_VM0_INTSYNC;    // Set the fixed sync source to 
                                                //   internal.
      send( MAX7456_VM0_W, videoMode );         // Update mode.
    }
    else if (sync == MAX7456_EXTSYNC)
    {
      videoMode |= 1 << MAX7456_VM0_FIXSYNC;    // Set sync to fixed.
      videoMode &= ~(1 << MAX7456_VM0_INTSYNC); // Set the fixed sync source to 
                                                //   external.
      send( MAX7456_VM0_W, videoMode );         // Update mode.
    }
    else if (sync == MAX7456_AUTOSYNC)
    {
      videoMode &= ~(1 << MAX7456_VM0_FIXSYNC); // Set sync to auto.
      videoMode |= 1 << MAX7456_VM0_INTSYNC;    // Set the default sync source 
                                                //   to internal.
      send( MAX7456_VM0_W, videoMode );         // Update mode.
    }
    else
    {
      return false;
    }
    return true;
    
  }
  // end setSyncSource()
  
  
  
  
  // Sets timings for text edge rise and fall, and for video mux transition.
  // Longer timings will set croma distortion to a minimum and shorter timings  
  // will make the sharpness slightly better.
  bool MAX7456::setSwitchingTime( 
                    uint8_t timing              // Rise & fall time [0 - 5] 
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (timing <= 5 && timing >= 0)             // If a valid timing,
    {
      send(MAX7456_OSDM_W, timing * 0x9);       // set both timings.
      
      return true;
    }
    
    return false;
    
  }
  // end setSwitchingTime()
  
  
  
  
  // Sets the length of each cycle of blinking text.
  /*bool MAX7456::setBlinkingTime( 
                    uint8_t timing              // Blink delay [0 - 3] 
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (timing <= 3 && timing >= 0)             // If timing is valid,
    {
      
      send( MAX7456_VM1_W, 
            timing,                             // set the timing.
            MAX7456_VM1_BT, 
            MAX7456_VM1_BT_LEN );
      
      return true;
    }
    
    return false;
    
  }
  // end setBlinkingTime()
  
  
  
  
  // Sets the duty cycle mode of blinking text.
  bool MAX7456::setBlinkingDuty( 
                    uint8_t duty                // Duty cycle mode [0,1,2,3] 
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (duty <= 3 && duty >= 0)                 // If duty mode is valid,
    {
      send( MAX7456_VM1_W, 
            duty,                               // set the duty mode.
            MAX7456_VM1_BDC, 
            MAX7456_VM1_BDC_LEN );
      
      return true;
    }
    
    return false;
    
  }*/
  // end setBlinkingDuty()
  
  
  
  
  // Sets the brightness of white pixels of text across whole screen or on 
  // a row by row basis.
  bool MAX7456::setWhiteLevel( 
                    uint8_t brightness 
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    for (int iRowAddress = MAX7456_RB0_W;       // For each row,
         iRowAddress <= MAX7456_RB15_W;
         iRowAddress++                   ) 
    {
      send( iRowAddress, 
            brightness,                         // set the white level.
            MAX7456_RBn_CWL, 
            MAX7456_RBn_CWL_LEN );
    }
    
    return true;
    
  }
  bool MAX7456::setWhiteLevel( 
                    uint8_t brightness,
                    int8_t  row                 // Displayable row [-15 - 15]
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (row < 0) row += displayRows;            // Handle negitive row id.
    
    if (row > 0 && row < displayRows)           // If a valid row,
    {
      send( row + shiftRows + MAX7456_RB0_W, 
            brightness,                         // set the white level.
            MAX7456_RBn_CWL, 
            MAX7456_RBn_CWL_LEN );
      
      return true;
    }
    
    return false;
    
  }
  // end setWhiteLevel()
  
  
  
  
  // Sets the brightness of black pixels of text across whole screen or on 
  // a row by row basis.
  bool MAX7456::setBlackLevel( 
                    uint8_t brightness 
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    for (int iRowAddress = MAX7456_RB0_W;       // For each row,
         iRowAddress <= MAX7456_RB15_W;
         iRowAddress++                   ) 
    {
      send( iRowAddress, 
            brightness,                         // set the black level.
            MAX7456_RBn_CBL, 
            MAX7456_RBn_CBL_LEN );
    }
    
    return true;
    
  }
  bool MAX7456::setBlackLevel( 
                    uint8_t brightness,
                    int8_t  row                 // Displayable row [-15 - 15]
                  )
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (row < 0) row += displayRows;            // Handle negitive row id.
    
    if (row > 0 && row < displayRows)           // If a valid row,
    {
      send( row + shiftRows + MAX7456_RB0_W, 
            brightness,                         // set the black level.
            MAX7456_RBn_CBL, 
            MAX7456_RBn_CBL_LEN );
      
      return true;
    }
    
    return false;
    
  }
  // end setBlackLevel()
  
  
  
  
  // Sets how bright the gray background appears.
  bool MAX7456::setGrayLevel( 
                    uint8_t brightness          // Gray brightness [0 - 7] 
                  )
  {
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (brightness <= 7 && brightness >= 0)     // If brightness is valid,
    {
      send( MAX7456_VM1_W, 
            brightness,                         // set the gray level.
            MAX7456_VM1_GLVL, 
            MAX7456_VM1_GLVL_LEN );
      
      return true;
    }
    
    return false;
    
  }
  // end setGrayLevel()
  
  
  
  
  // Returns video system detected at video input. Returns a value equal to 
  // MAX7456_PAL, MAX7456_NTSC, or NULL.
  uint8_t MAX7456::videoSystem() 
  {
    
    if (!begun) return NULL;
    
    if (resetIsBusy()) return NULL;             // Reset implies default.
    
    uint8_t ubSystem = NULL;
    uint8_t ubMask   = 0;
    uint8_t ubShift  = 0;
    
    ubSystem = receive( MAX7456_STAT_R );       // Get status register value.
    if (ubSystem & 1 << MAX7456_STAT_NTSC)
    {
      return MAX7456_NTSC;
    }
    if (ubSystem & 1 << MAX7456_STAT_PAL)
    {
      return MAX7456_PAL;
    }
    return NULL;
    
  }
  // end videoSystem()
  
  
  
  
  // Changes the default video output system to NTSC or PAL. This does not 
  // force a conversion at the video output to this system because the MAX7456 
  // does not support conversion from PAL to NTSC or vice-versa. It should,
  // however, be set to the video system detected at video input or unexpected 
  // behaviour may happen. If NULL is passed in, it will not set anything.
  // Returns true on success and false on failure.
  bool MAX7456::setDefaultSystem( 
                    uint8_t system 
                  ) 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    if (system == MAX7456_PAL)
    {
      videoMode |= 1 << MAX7456_VM0_PAL;        // Set the video system to PAL.
      
      send( MAX7456_VM0_W, videoMode );         // Update mode.
      
      defaultSystem = system;
    }
    if (system == MAX7456_NTSC)
    {
      videoMode &= ~(1 << MAX7456_VM0_PAL);     // Set the video system to NTSC.
      
      send( MAX7456_VM0_W, videoMode );         // Update mode.
      
      defaultSystem = system;
    }
    else
    {
      return false;
    }
    return true;
    
  }
  // end setDefaultSystem()
  
  
  
  
  // Shifts the display by pixels to adjust centering.
  bool MAX7456::setTextOffset( 
                    int8_t xOffset, 
                    int8_t yOffset 
                  ) 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    xOffset += MAX7456_HOS_ZERO;                // Normalize values.
    yOffset += MAX7456_VOS_ZERO;
    
    if (xOffset + shiftPixelsH >=               // If offset pixels greater 
            1 << MAX7456_HOS_WID   ||           // than max accepted, or less
        xOffset + shiftPixelsH < 0   )          // than the min accepted,
    {
      return false;                             // return with error flag. 
    }
    if (yOffset + shiftPixelsV >=               // If offset pixels greater 
            1 << MAX7456_VOS_WID   ||           // than max accepted, or less
        yOffset + shiftPixelsV < 0   )          // than the min accepted,
    {
      return false;                             // return with error flag. 
    }

    // Set offsets:
    offsetPixelsH =                             // Zero values and save.
        xOffset - MAX7456_HOS_ZERO;
    offsetPixelsV = 
        yOffset - MAX7456_VOS_ZERO;
    send( MAX7456_HOS_W,                        // Set Horizontal Offset.
          shiftPixelsH + xOffset );
    send( MAX7456_VOS_W,                        // Set Vertical Offset.
          shiftPixelsV + yOffset );
    
    return true;
    
  }
  // end setTextOffset()
  
  
  
  
  // Sets the borders and dimentions of the display area. Requires the display 
  // to be cleared. Completion of operation can be checked by clearIsBusy().
  // Returns true on success and false on failure.
  bool MAX7456::setTextArea( 
                    int8_t  cols, 
                    int8_t  rows,  
                    uint8_t area 
                  ) 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    // Instantiate local variables:
    uint8_t ubSystem      = NULL;
    int16_t iTempPixelsH  = 0;
    int16_t iTempPixelsV  = 0;
    uint8_t ubTempColumns = 0;
    uint8_t ubTempRows    = 0;
    uint8_t ubMaxColumns  = 0;
    uint8_t ubMaxRows     = 0;
    /*
    if (displayColumns && displayRows &&        // Only clear if set up and 
        !clearIsBusy()                  )       // not already clearing.
    {
      clear();
    }
    */
    ubSystem = videoSystem();
    if (ubSystem == NULL)                       // If no valid input,
    {
      ubSystem = defaultSystem;                 // default system is used.
    }
    
    if (cols == NULL || rows == NULL)
    {
      if (area < 0 || area >= 4) 
      {
        return false;
      }
      
      if (ubSystem == MAX7456_PAL) 
      {
        cols = safeVideoCols[1][area];
        rows = safeVideoRows[1][area];
      }
      else
      {
        cols = safeVideoCols[0][area];
        rows = safeVideoRows[0][area];
      }
    }
    
    // Get the maximum number of rows and columns:
    if (ubSystem == MAX7456_NTSC)
    {
      ubMaxColumns = MAX7456_COLS_N0;
      ubMaxRows = MAX7456_ROWS_N0;
    }
    else if (ubSystem == MAX7456_PAL)
    {
      ubMaxColumns = MAX7456_COLS_P0;
      ubMaxRows = MAX7456_ROWS_P0;
    }
    else 
    {
      return false;
    }
    
    
    // Calculate centering offsets:
    if (cols > ubMaxColumns ||                  // Set text display columns and 
        rows > ubMaxRows      )                 // rows only if less than max.
    {
      return false;
    }
    if (cols < 0) cols += ubMaxColumns;         // Allows user to set the area 
    if (rows < 0) rows += ubMaxRows;            // via negitive values to 
                                                //   specify a margin value.
    ubTempColumns = ubMaxColumns - cols;
    ubTempRows = ubMaxRows - rows;
    iTempPixelsH = (ubTempColumns % 2)          // If even, no pixel offset is 
                       * MAX7456_HPX_C / 2;     // needed, so take the modulus 
    iTempPixelsV = (ubTempRows % 2)             // to set row and column 
                       * MAX7456_VPX_C / 2;     // pixel offsets to the half
                                                //   width or half height of a 
                                                //   single character.
    ubTempColumns = ubTempColumns / 2;          // Do an integer division to 
    ubTempRows = ubTempRows / 2;                // convert the original value 
                                                //   of the rows and columns
                                                //   (the differance between
                                                //   the max and the selected), 
                                                //   to the new value that 
                                                //   only counts the offset on
                                                //   one side. This will ignore 
                                                //   half columns, which are 
                                                //   accounted for by the pixel 
                                                //   offset.
    if (iTempPixelsV > 0)                       // If vertical offset in 
    {                                           //   pixels is positive,
      iTempPixelsV -= MAX7456_VPX_C;            // make it negitive,
      ubTempRows++;                             // and offset rows accordingly
    }                                           //   to improve headroom for 
                                                //   setTextOffset().
    
    // Set private variables and send offsets:
    shiftPixelsH = (char)iTempPixelsH;
    send( MAX7456_HOS_W,                        // Set Horizontal Offset
          offsetPixelsH + shiftPixelsH 
              + MAX7456_HOS_ZERO );
    shiftColumns = ubTempColumns;
    displayColumns = cols;
    shiftPixelsV = (char)iTempPixelsV;
    send( MAX7456_VOS_W,                        // Set Vertical Offset
          offsetPixelsV + shiftPixelsV 
              + MAX7456_VOS_ZERO );
    shiftRows = ubTempRows;
    displayRows = rows;
    cursorX = 0;                                // Restore cursor.
    cursorY = 0;
    cursorValid = true;                         // Enable write().
    
    return true;
    
  }
  // end setTextArea()
  
  
  
  
  // Changes Character Encoding between MAXIM and ASCII.
  // Returns true on success and false on failure.
  bool MAX7456::setCharEncoding( 
                    uint8_t encoding 
                  ) 
  {
    
    if (!begun) return false;
    
    if (encoding == MAX7456_ASCII ||
        encoding == MAX7456_MAXIM   )
    {
      charEncoding = encoding;
      
      return true;
    }
    return false;
    
  }
  // end setCharEncoding()
  
  
  
  
  // Subsequent writes will have the local background be transparent and thus 
  // allow video to show through at those pixels.
  void MAX7456::videoBackground() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          0, 
          MAX7456_DMM_LBC );
    
  }
  // end videoBackground()
  
  
  
  
  // Subsequent writes will have the local background be gray, and video will 
  // only exist around blocks of text.
  void MAX7456::grayBackground() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          1, 
          MAX7456_DMM_LBC );
    
  }
  // end grayBackground()
  
  
  
  
  // Subsequent writes will display blinking.
  void MAX7456::blink() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          1, 
          MAX7456_DMM_BLK );
    
  }
  // end blink()
  
  
  
  
  // Subsequent writes will display without blinking.
  void MAX7456::noBlink() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          0, 
          MAX7456_DMM_BLK );
    
  }
  // end noBlink()
  
  
  
  
  // Subsequent writes will have the font color displayed as non-inverted 
  // (white is white, black is black).
  void MAX7456::normalColor() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          0, 
          MAX7456_DMM_INV );
    
  }
  // end normalColor()
  
  
  
  
  // Subsequent writes will have the font color displayed inverted 
  // (white is black, black is white).
  void MAX7456::invertColor() 
  {
    
    if (!begun) return;
    
    if (resetIsBusy()) return;                  // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          1, 
          MAX7456_DMM_INV );
    
  }
  // end invertColor()
  
  
  
  
  // Subsequent writes will display with given attributes.
  bool MAX7456::setTextAttributes( 
                    uint8_t attributes 
                  ) 
  {
    
    if (!begun) return false;
    
    if (resetIsBusy()) return false;            // Reset prevents send.
    
    send( MAX7456_DMM_W, 
          attributes, 
          MAX7456_DMM_INV,
          3 );
    
    return true;
    
  }
  // end setTextAttributes()
  
  
  
  
  void MAX7456::lineWrap()
  {
    wrapLines = true;
  }
  // end lineWrap()
  
  
  
  
  void MAX7456::noLineWrap()
  {
    wrapLines = false;
  }
  // end noLineWrap()
  
  
  
  
  void MAX7456::pageWrap()
  {
    wrapPages = true;
  }
  // end pageWrap()
  
  
  
  
  void MAX7456::noPageWrap()
  {
    wrapPages = false;
  }
  // end noPageWrap()
  
  
  
  
  bool MAX7456::cursor()
  {
    
    if (!cursorValid)                           // If known invalid location, 
    {                                           //   (or if begun is false),
      return false;                             // don't write anything.
    }
    
    // Instantiate local variables:
    uint16_t u2address = 0;
    
    // Update display memory address:
    u2address += cursorY;
    u2address += shiftRows;                     // Shift for vsafe modes.
    u2address *= MAX7456_COLS_D;                // Scale rows to address.
    u2address += cursorX;
    u2address += shiftColumns;                  // Shift for vsafe modes.
    
    send( MAX7456_DMAH_W,                       // Set the higher byte of the
          (uint8_t)(u2address >> 8) );          //   display address.
    send( MAX7456_DMAL_W,                       // Set the lower byte of the
          (uint8_t)(u2address) );               //   display address.
    
    // Write cursor to display memory:
    send( MAX7456_DMDI_W, 0xFF );               // Write the cursor symbol.
    
    return true;
    
  }
  // end cursor()
  
  
  
  
  bool MAX7456::noCursor()
  {
    
    if (!cursorValid)                           // If known invalid location, 
    {                                           //   (or if begun is false),
      return false;                             // don't write anything.
    }
    
    // Instantiate local variables:
    uint16_t u2address = 0;
    
    // Update display memory address:
    u2address += cursorY;
    u2address += shiftRows;                     // Shift for vsafe modes.
    u2address *= MAX7456_COLS_D;                // Scale rows to address.
    u2address += cursorX;
    u2address += shiftColumns;                  // Shift for vsafe modes.
    
    send( MAX7456_DMAH_W,                       // Set the higher byte of the
          (uint8_t)(u2address >> 8) );          //   display address.
    send( MAX7456_DMAL_W,                       // Set the lower byte of the
          (uint8_t)(u2address) );               //   display address.
    
    // Write cursor to display memory:
    send( MAX7456_DMDI_W, 0x00 );               // Write empty space.
    
    return true;
    
  }
  // end noCursor()
  
  
  
  
  // Returns the cursor to the origin (0,0). This is the upper left corner 
  // of the user defined screen area.
  bool MAX7456::home() 
  {
    
    if (!begun) return false;
    
    // Set private variables:
    noInterrupts();                             // Protect any interrupt 
                                                //   functions from bad data.
    cursorX = 0;
    cursorY = 0;
    cursorValid = true;                         // Store location validity 
                                                //   to improve write() speed.
    badIncrement = true;                        // Note movement to cause
                                                //   rewrite of DMAH & DMAL.
    interrupts();                               // Resume normal interrupts.
    
    return cursorValid;
  }
  // end home()
  
  
  
  
  // Sets the position of the cursor in the grid of columns and rows, where
  // the upper left is defined as the origin (0,0). Wraps negitive values
  // to the other side of the screen to allow any alignment.  Returns if the 
  // new position is a valid location. Will move regardless of validity.
  bool MAX7456::setCursor( 
                    int8_t col, 
                    int8_t row 
                  ) 
  {
    
    if (!begun) return false;
    
    // Copy to private variables:
    if (col < 0) col += displayColumns;         // Right aligned 
    if (row < 0) row += displayRows;            // Bottom aligned 
    if (col >= 0             &&                 // If valid location,
        col < displayColumns &&
        row >= 0             &&
        row < displayRows) 
    {
      noInterrupts();                           // protect any interrupt 
                                                //   functions from bad data,
      cursorX = col;
      cursorY = row;
      cursorValid = true;                       // store location validity 
                                                //   to improve write() speed,
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL,
      interrupts();                             // resume normal interrupts;
    }
    else                                        // Otherwise,
    {
      noInterrupts();                           // protect any interrupt 
                                                //   functions from bad data,
      cursorX = col;
      cursorY = row;
      cursorValid = false;                      // store location validity 
                                                //   to improve write() speed,
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL,
      interrupts();                             // resume normal interrupts.
    }
    
    return cursorValid;
    
  }
  // end setCursor()
  
  
  
  
  // Moves the cursor to relative positions. Returns if the new position is a 
  // valid location. Will move regardless of validity.
  bool MAX7456::moveCursor( 
                    uint8_t mode
                  ) 
  {
    if (mode == MAX7456_CUR_HT) 
    {
      return moveCursor( MAX7456_CUR_HT, 8 );   // Make 8 the default tabstop.
    }
    else
    {
      return moveCursor( mode, 1 );             // Make 1 the default distance.
    }
  }
  bool MAX7456::moveCursor( 
                    uint8_t mode, 
                    uint8_t distance 
                  ) 
  {
    
    if (!begun) return false;
    
    // Instantiate local variables:
    uint8_t u1x    = cursorX;
    uint8_t u1y    = cursorY;
    
    if (mode == MAX7456_CUR_FWD)
    {
      u1x += distance;                          // Move cursor right
      if (wrapLines &&                          // If line wapping enabled and
          u1x >= displayColumns)                // at end of line,
      {
        u1x = 0;                                // go to start of next line.
        u1y++;
      }
      if (wrapPages &&                          // If page wapping enabled and
          u1y >= displayRows)                   // at end of the display,
      {
        u1x = 0;                                // reset cursor to home.
        u1y = 0;
      }
    }
    else if (mode == MAX7456_CUR_BACK)
    {
      u1x -= distance;                          // Move cursor left
      if (wrapLines &&                          // If line wapping enabled and
          u1x < 0 )                             // at start of line,
      {
        u1x = displayColumns-1;                 // go to end of previous line.
        u1y--;
      }
      if (wrapPages &&                          // If page wapping enabled and
          u1y < 0 )                             // at start of the display,
      {
        u1x = displayColumns - 1;               // reset cursor to end.
        u1y = displayRows - 1;
      }
    }
    else if (mode == MAX7456_CUR_CR)
    {
      u1x = 0;                                  // Move all the way left
    }
    else if (mode == MAX7456_CUR_DOWN)
    {
      u1y += distance;                          // Move cursor down
      if (wrapPages &&                          // If page wapping enabled and
          u1y >= displayRows)                   // at end of the display,
      {
        u1y = 0;                                // reset cursor to top.
      }
    }
    else if (mode == MAX7456_CUR_UP)
    {
      u1y -= distance;                          // Move cursor down
      if (wrapPages &&                          // If page wapping enabled and
          u1y < 0 )                             // at start of the display,
      {
        u1y = displayRows - 1;                  // reset cursor to bottom.
      }
    }
    else if (mode == MAX7456_CUR_HT)
    {
      u1x++;                                    // Make tab at least one space.
      while (u1x % distance != 0) {             // Move cursor to tabstop
        u1x++;
      }
    }
    else if (mode == MAX7456_CUR_FF)
    {
      return home();
    }
    else
    {
      cursorValid = false;
      return cursorValid;
    }
    
    if (u1x >= 0             &&                 // If valid location,
        u1x < displayColumns &&
        u1y >= 0             &&
        u1y < displayRows) 
    {
      noInterrupts();                           // protect any interrupt 
                                                //   functions from bad data,
      cursorX = u1x;
      cursorY = u1y;
      cursorValid = true;                       // store location validity 
                                                //   to improve write() speed,
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL,
      interrupts();                             // resume normal interrupts;
    }
    else                                        // Otherwise,
    {
      noInterrupts();                           // protect any interrupt 
                                                //   functions from bad data,
      cursorX = u1x;
      cursorY = u1y;
      cursorValid = false;                      // store location validity 
                                                //   to improve write() speed,
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL,
      interrupts();                             // resume normal interrupts.
    }
    
    return cursorValid;
    
  }
  // end setCursor()
  
  
  
  
  // Write a char to the MAX7456 display at the x and y coordinate stored 
  // previously. Caution! For performance reasons, this function does not 
  // check for resetIsBusy() condition, and thus should not be used until
  // after a reset has completed.
  size_t MAX7456::write( 
                      uint8_t symbol 
                    ) 
  {
    
    if (!cursorValid)                           // If invalid location (or 
    {                                           //   if begun is false),
      increment();                              // move cursor anyway,
      setWriteError();                          // throw error, and 
      return 0;                                 // don't write anything.
    }
    
    // Instantiate local variables:
    uint16_t u2address = 0;
    
    // Decode the symbol:
    symbol = decode( symbol );
    
    // Update display memory address as needed:
    if (badIncrement ||                         // If new location,
        !inAutoIncrement ||                     // or no longer in AI,
        symbol == 0xFF)                         // or if break symbol,
    {
      u2address += cursorY;
      u2address += shiftRows;                   // shift for vsafe modes,
      u2address *= MAX7456_COLS_D;              // scale rows to address,
      u2address += cursorX;
      u2address += shiftColumns;                // shift for vsafe modes,
      
      send( MAX7456_DMAH_W,                     // set the higher byte of the
            (uint8_t)(u2address >> 8) );        //   display address,
      send( MAX7456_DMAL_W,                     // set the lower byte of the
            (uint8_t)(u2address) );             //   display address,
      badIncrement = false;                     // note sucessful relocation.
    }
    
    // Write to display memory:
    if (symbol != 0xFF) {                       // If not attempting to write 
                                                //   the break character,
      send( symbol );                           // write the symbol in 
                                                //   autoincrement mode;
    }
    else                                        // Otherwise,
    {
      send( MAX7456_DMDI_W,                     // write the symbol normally.
            symbol );
    }
    
    // Increment cursor:
    increment();
        
    return 1;
    
  }
  // end write()
  
  
  
  
  // Reading a character glyph takes time so this function is intended to be 
  // called repeadedly within a while loop. Returns true on completion.
  bool MAX7456::readChar( 
                    uint8_t ord, 
                    uint8_t image[] 
                  ) 
  {
    
    // Error conditions return true to break loop:
    if (!begun) return true;                    // Return true to break loop.
    
    if (resetIsBusy()) return true;             // Reset prevents send.
    
    // Disables display as needed and returns:
    if (receive( MAX7456_VM0_R ) &              // If display is enabled,
          1 << MAX7456_VM0_OSDON  )
    {
      videoMode &= ~(1 << MAX7456_VM0_OSDON);   // disable display,
      
      send( MAX7456_VM0_W, videoMode );         // update mode,
      
      return false;                             // return busy state.
    }
    
    // Wait indefinitely on Non-Volitile Memory:
    if (receive( MAX7456_STAT_R ) &             // If NVM is busy copying,
          1 << MAX7456_STAT_CMBUSY )
    {
      return false;                             // return busy state.
    }
    
    // Send read command as needed:
    if (!charTransfered)                        // If no command has been sent,
    {
      send( MAX7456_CMAH_W, ord );              // send address of character 
                                                //   glyph to be read,
      send( MAX7456_CMM_W, 0x50 );              // begin copy from NVR block 
                                                //   to Shadow RAM,
      charTransfered = true;                    // note state.
      
      return false;                             // return busy state.
    }
    
    // Receive data into provided array:
    for(uint8_t u1n = 0; 
        u1n < 54; 
        u1n++           )
    {
      send( MAX7456_CMAL_W, u1n );              // Select the offset address 
                                                //   in the shadow RAM
      image[u1n] = receive( MAX7456_CMDO_R );   // Write the data to the  
                                                //   shadow RAM
    }
    
    charTransfered = false;                     // Fix state.
    
    return true;                                // Return complete state.
    
  }
  // end readChar()
  
  
  
  
  // Writing a character glyph takes time so this function is intended to be 
  // called repeadedly within a while loop. Returns true on completion.
  bool MAX7456::createChar( 
                    uint8_t ord, 
                    const uint8_t image[] 
                  ) 
  {
    
    // Error conditions return true to break loop:
    if (!begun) return true;                    // Return true to break loop.
    
    if (resetIsBusy()) return true;             // Reset prevents send.
    
    // Disables display as needed and returns:
    if (receive( MAX7456_VM0_R ) &              // If display is enabled,
          1 << MAX7456_VM0_OSDON  )
    {
      videoMode &= ~(1 << MAX7456_VM0_OSDON);   // disable display,
      
      send( MAX7456_VM0_W, videoMode );         // update mode,
      
      return false;                             // return busy state.
    }
    
    // Wait indefinitely on Non-Volitile Memory:
    if (receive( MAX7456_STAT_R ) &             // If NVM is busy copying,
          1 << MAX7456_STAT_CMBUSY )
    {
      return false;                             // return busy state.
    }
    
    // Write data from provided array:
    if (!charTransfered)                        // If no command has been sent,
    {
      send( MAX7456_CMAH_W, ord );              // send address of character 
                                                //   glyph to be read,
      for(uint8_t u1n = 0; 
          u1n < 54; 
          u1n++           )
      {
        send( MAX7456_CMAL_W, u1n );            // Select the offset address 
                                                //   in the shadow RAM
        send( MAX7456_CMDI_W, image[u1n] );     // Write the data to the  
                                                //   shadow RAM
      }
      
      send( MAX7456_CMM_W, 0xA0 );              // Begin copy from Shadow RAM
                                                //    to NVM block.
      charTransfered = true;                    // note state.
      
      return false;                             // return busy state.
    }
    
    charTransfered = false;                     // Fix state.
    
    return true;                                // Return complete state.
    
  }
  // end createChar()
  
  
  
  
  // Gets the columns number of the cursor.
  int8_t MAX7456::cursorColumn() 
  {
    
    return cursorX;
    
  }
  // end columns()
  
  
  
  
  // Gets the row number of the cursor.
  int8_t MAX7456::cursorRow() 
  {
    
    return cursorY;
    
  }
  // end columns()
  
  
  
  
  // Gets the total number of displayable columns.
  uint8_t MAX7456::columns() 
  {
    
    return displayColumns;
    
  }
  // end columns()
  
  
  
  
  // Gets the total number of displayable rows.
  uint8_t MAX7456::rows() 
  {
    
    return displayRows;
    
  }
  // end rows()
  
  
  
  
// Private Methods /////////////////////////////////////////////////////////////

  // Communicates via SPI to store a byte of data into a memory location 
  // in the MAX7456 OSD. Returns the resulting written data.
  inline uint8_t MAX7456::send( 
                              uint8_t value 
                            ) 
  {
    
    if (!inAutoIncrement) {                     // If needed,
      noInterrupts();                           // send and note change without 
                                                //   interrupt to prevent any 
                                                //   errors with send(),
      send( MAX7456_DMM_W,                      // reenter autoincrement mode,
            1, 
            MAX7456_DMM_AI );
      inAutoIncrement = true;                   // note change for safety,
      interrupts();                             // resume normal interrupts,
    }
    
    *deviceSelectPort &= ~deviceSelectMask;     // Set the chip select pin low 
                                                //   to enable slave SPI.
    SPIObject->transfer( value );               // Send value to store into 
                                                //   register.
    *deviceSelectPort |= deviceSelectMask;      // Set the chip select pin high 
                                                //   to disable slave SPI.
    return value;
    
  }
  inline uint8_t MAX7456::send( 
                              uint8_t address, 
                              uint8_t value 
                            ) 
  {
    
    if (inAutoIncrement) {                      // If needed,
      noInterrupts();                           // send and note change without 
                                                //   interrupt to prevent any 
                                                //   errors with send(),
      send( 0xFF );                             // break out of autoincrement,
      inAutoIncrement = false;                  // note change for safety,
      interrupts();                             // resume normal interrupts.
    }
    
    *deviceSelectPort &= ~deviceSelectMask;     // Set the chip select pin low 
                                                //   to enable slave SPI.
    SPIObject->transfer( address );             // Send register address.
    SPIObject->transfer( value );               // Send value to store into 
                                                //   register.
    *deviceSelectPort |= deviceSelectMask;      // Set the chip select pin high 
                                                //   to disable slave SPI.
    return value;
    
  }
  uint8_t MAX7456::send( 
                      uint8_t address, 
                      uint8_t value, 
                      uint8_t bitOffest 
                    ) 
  {
    
    uint8_t ubContent;                          // Create temporary byte for 
                                                //   received value.
    ubContent = receive(                        // Get content currently in the 
                    address + MAX7456_RRAO      //   register.
                  );
    if (value) ubContent |= (1 << bitOffest);   // If value != 0, set the 
                                                //   selected bit to 1,
    else ubContent &= ~(1 << bitOffest);        // otherwise, set the selected 
                                                //   bit to 0.
    return send( address, ubContent );          // Write the modified register 
                                                //   contents.   
  }
  uint8_t MAX7456::send( 
                       uint8_t address, 
                       uint8_t value, 
                       uint8_t bitOffest, 
                       uint8_t valueLength 
                     ) 
  {
    
    uint8_t ubContent;                          // Create temporary byte for 
                                                //   received value and
    uint8_t ubMask;                             // for input cleaning.
    
    ubContent = receive(                        // Get content currently in the 
                    address + MAX7456_RRAO      //   register.
                  );
    ubMask = (1 << valueLength) - 1;            // Form mask of proper width, 
    ubMask = ubMask << bitOffest;               // shifted into place.
    value = value << bitOffest;                 // Shift vaule into place, and 
    value = value & ubMask;                     // clean it.
    ubContent = ubContent & ~ubMask;            // Clear affected bits, and 
    ubContent = ubContent | value;              // set them with value.
    
    return send( address, ubContent );          // Write the modified register 
                                                //   contents.
  }
  // end send()
  
  
  
  
  // Communicates via SPI to receive a byte of data from a memory location in 
  // the MAX7456 OSD.
  inline uint8_t MAX7456::receive( 
                              uint8_t registerAddress 
                            ) 
  {
    
    uint8_t ubContent = 0;                      // Create temporary byte for 
                                                //   received value.
    
    if (inAutoIncrement) {                      // If needed,
      noInterrupts();                           // send and note change without 
                                                //   interrupt to prevent any 
                                                //   errors with send(),
      send( 0xFF );                             // break out of autoincrement,
      inAutoIncrement = false;                  // note change for safety,
      interrupts();                             // resume normal interrupts.
    }
    
    *deviceSelectPort &= ~deviceSelectMask;     // Set the chip select pin low 
                                                //   to enable slave SPI.
    SPIObject->transfer( registerAddress );     // Send register address.
    ubContent = SPIObject->transfer( 0xFF );    // Get value from register.
    
    *deviceSelectPort |= deviceSelectMask;      // Set the chip select pin high 
                                                //   to disable slave SPI.
    return ubContent;                           // Return received value.
    
  }
  // end receive()
  
  
  
  
  // Handles character encoded writes to display memory for write().
  inline uint8_t MAX7456::decode( 
                              uint8_t symbol 
                            ) 
  {
    
    /*if (charEncoding == MAX7456_MAXIM)
    {
      if (symbol < 128 && 
          symbol >= 0    ) 
      {
        return defaultCodePage[symbol];         // Send char to display.
      } 
      else 
      {
        return symbol;                          // Send symbol to display.
      }
    }
    else if (charEncoding == MAX7456_ASCII) 
    {
      return symbol;                            // Send symbol to display.
    }*/ 
    return symbol;
  }
  // end decode()
  
  
  
  
  // Handles local variable increment for write().
  inline void MAX7456::increment() 
  {
    
    // Instantiate local variables:
    uint8_t u1x    = cursorX;
    uint8_t u1y    = cursorY;
    
    u1x += 1;                                   // Move cursor right
    if (wrapLines &&                            // If line wapping enabled and
        u1x >= displayColumns)                  // at end of line,
    {
      u1x = 0;                                  // go to start of next line.
      u1y++;
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL.
    }
    if (wrapPages &&                            // If page wapping enabled and
        u1y >= displayRows)                     // at end of the display,
    {
      u1x = 0;                                  // reset cursor to home.
      u1y = 0;
      badIncrement = true;                      // note movement to cause
                                                //   rewrite of DMAH & DMAL.
    }
    
    noInterrupts();                             // Update local vars without 
                                                //   interrupt to prevent any 
                                                //   errors with send().
    cursorX = u1x;
    cursorY = u1y;
    cursorValid = (u1x >= 0             &       // Define location validity.
                   u1x < displayColumns &
                   u1y >= 0             &
                   u1y < displayRows     );
    
    interrupts();                               // Resume normal interrupts.
    
  }
  // end increment()
  
  
  
  

