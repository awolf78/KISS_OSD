#ifndef  CONFIG_H
#define  CONFIG_H


// Uncomment ONE if PAL/NTSC autodetection does not work
//======================================================
//#define FORCE_NTSC
//#define FORCE_PAL

// Feature config
//=============================
#define BEERMUG
#define CUSTOM_TPA
#define CROSSHAIR




// INTERNALS - DO NOT CHANGE
//=============================

// vTx config
//=============================
//#define  IMPULSERC_VTX

#ifdef  IMPULSERC_VTX
# define  MAX7456RESET  9         // RESET
#endif

// DO NOT CHANGE
//=============================
#define  NEW_FC_SETTINGS

#endif
