#ifndef CONFIG_H
#define CONFIG_H

// Uncomment ONE if PAL/NTSC autodetection does not work
//======================================================
//#define FORCE_NTSC
//#define FORCE_PAL



// INTERNALS - DO NOT CHANGE
//=============================

// vTx config
//=============================
//#define IMPULSERC_VTX
#ifdef IMPULSERC_VTX
# define MAX7456RESET  9         // RESET
#endif

//#define UPDATE_FONT_ONLY

#endif
