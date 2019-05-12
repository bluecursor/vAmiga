// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AMIGA_T_INC
#define _AMIGA_T_INC

#include "DriveTypes.h"

//
// Type aliases
//

typedef int64_t Frame;

typedef int64_t Cycle;    // Cycle in master cycle units
typedef int64_t CPUCycle; // Cycle in CPU cycle units
typedef int64_t CIACycle; // Cycle in CIA cycle units
typedef int64_t DMACycle; // Cycle in DMA cycle units

#define CPU_CYCLES(cycles) ((cycles) << 2)
#define CIA_CYCLES(cycles) ((cycles) * 40)
#define DMA_CYCLES(cycles) ((cycles) << 3)

#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)


//
// Enumeration types
//

typedef enum : long
{
    A500,
    A1000,
    A2000
}
AmigaModel;

inline bool isAmigaModel(long model)
{
    return model >= A500 && model <= A2000;
}

inline const char *modelName(AmigaModel model)
{
    return
    model == A500 ? "Amiga 500" :
    model == A1000 ? "Amiga 1000" :
    model == A2000 ? "Amiga 2000" : "???";
}

typedef enum : long
{
    VA_AMIGA_MODEL,
    VA_KB_LAYOUT,
    VA_CHIP_RAM,
    VA_SLOW_RAM,
    VA_FAST_RAM,
    VA_DRIVE_CONNECT,
    VA_DRIVE_TYPE,
    VA_DRIVE_SPEED,
    VA_RT_CLOCK,
    VA_EXACT_BLITTER,
    VA_FIFO_BUFFERING,

    // DEPRECATED
    /*
    VA_DF0_CONNECT,
    VA_DF0_TYPE,
    VA_DF0_SPEED,
    VA_DF1_CONNECT,
    VA_DF1_TYPE,
    VA_DF1_SPEED,
    VA_DF2_CONNECT,
    VA_DF2_TYPE,
    VA_DF2_SPEED,
    VA_DF3_CONNECT,
    VA_DF3_TYPE,
    VA_DF3_SPEED
    */
}
ConfigOption;

inline bool isConfigOption(long value)
{
    return value >= VA_AMIGA_MODEL && value <= VA_FIFO_BUFFERING;
}

typedef enum
{
    RL_SNAPSHOT           = 0b00001,
    RL_INSPECT            = 0b00010,
    RL_ENABLE_TRACING     = 0b00100,
    RL_ENABLE_BREAKPOINTS = 0b01000,
    RL_STOP               = 0b10000,
    
    RL_DEBUG              = 0b01100
}
RunLoopControlFlag;


//
// Structure types
//

typedef struct
{
    bool connected;
    DriveType type;
    uint16_t speed;
}
DriveConfiguration;

typedef struct
{
    AmigaModel model;
    bool realTimeClock;
    long layout;
    DriveConfiguration df0;
    DriveConfiguration df1;
    DriveConfiguration df2;
    DriveConfiguration df3;
    bool exactBlitter;
    bool fifoBuffering;
}
AmigaConfiguration;

typedef struct
{
    long chipRamSize; // size in KB
    long slowRamSize; // size in KB
    long fastRamSize; // size in KB
}
AmigaMemConfiguration;

typedef struct
{
    Cycle masterClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
AmigaInfo;

#endif
