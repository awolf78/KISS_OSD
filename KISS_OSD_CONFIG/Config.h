#ifndef CONFIG_H
#define CONFIG_H

#include "MAX7456.h"

// vTx config
//=============================
//#define IMPULSERC_VTX
#ifdef IMPULSERC_VTX
# define MAX7456RESET  9         // RESET
#endif

// video system
//=============================
//#define PAL
#define NTSC

#ifdef PAL
static const uint8_t ROWS = MAX7456_ROWS_P1;
static const uint8_t COLS = MAX7456_COLS_P1;
#else
static const uint8_t ROWS = MAX7456_ROWS_N0;
static const uint8_t COLS = MAX7456_COLS_N1;
#endif

#endif
