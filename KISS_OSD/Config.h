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
#define PROP_ICON
#define _MAH_ICON
#define WATTMETER
#define SHOW_KISS_LOGO
#define RSSI_
#define _RSSI_ICON
#define STOPWATCH_ICON
#define NEW_FILTER



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
#define NEW_FC_SETTINGS

#endif
