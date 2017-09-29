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
//#define PROP_ICON
#define _MAH_ICON
#define WATTMETER
#define SHOW_KISS_LOGO
#define RSSI_
#define _RSSI_ICON
#define STOPWATCH_ICON
#define NEW_FILTER
#define FAILSAFE
//#define VTX_POWER_KNOB
//#define ADVANCED_STATS
#define ADVANCED_ESC_STATS
#define MAH_CORRECTION
//#define CROSSHAIR_ANGLE
#define RC_SPLIT_CONTROL
#define ARMING_STATUS
//#define CAMERA_CONTROL
#define SERIAL_SETTINGS
//#define AUSSIE_CHANNELS


// INTERNALS - DO NOT CHANGE
//=============================
//#define STEELE_PDB
//#define STEELE_PDB_OVERRIDE
//#define BF32_MODE


// vTx config
//=============================
//#define  IMPULSERC_VTX


// DO NOT CHANGE
//=============================

#if defined(BF32_MODE) && defined(CUSTOM_TPA)
#undef CUSTOM_TPA
#endif

#endif
