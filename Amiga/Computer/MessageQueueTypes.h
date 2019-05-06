// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _MESSAGE_QUEUE_T_INC
#define _MESSAGE_QUEUE_T_INC

// List of all known message id's
typedef enum
{
    MSG_NONE = 0,
    
    // Emulator state
    MSG_CONFIG,
    MSG_READY_TO_POWER_ON,
    MSG_POWER_ON,
    MSG_POWER_OFF,
    MSG_RUN,
    MSG_PAUSE,
    MSG_RESET,
    MSG_ROM_MISSING,
    MSG_WARP_ON,
    MSG_WARP_OFF,
    MSG_POWER_LED_ON,
    MSG_POWER_LED_OFF,
    
    // CPU
    MSG_BREAKPOINT_CONFIG,
    MSG_BREAKPOINT_REACHED,
    
    // Memory
    MSG_MEM_LAYOUT,
    
    // Keyboard
    MSG_MAP_CMD_KEYS,
    MSG_UNMAP_CMD_KEYS,
    
    // Floppy drives
    MSG_DRIVE_CONNECT,
    MSG_DRIVE_DISCONNECT,
    MSG_DRIVE_LED_ON,
    MSG_DRIVE_LED_OFF,
    MSG_DRIVE_DISK_INSERT,
    MSG_DRIVE_DISK_EJECT,
    MSG_DRIVE_DISK_SAVED,
    MSG_DRIVE_DISK_UNSAVED,
    MSG_DRIVE_DISK_PROTECTED,
    MSG_DRIVE_DISK_UNPROTECTED,
    MSG_DRIVE_MOTOR_ON,
    MSG_DRIVE_MOTOR_OFF,
    MSG_DRIVE_DMA_ON,
    MSG_DRIVE_DMA_OFF,
    MSG_DRIVE_HEAD,
    MSG_DRIVE_HEAD_POLL,
    
    
    
    //
    // DEPRECATED C64 related messages ...
    //
    
    // ROM and snapshot handling
    MSG_SNAPSHOT_TAKEN,
    
    // CPU related messages
    MSG_CPU_OK,
    MSG_CPU_SOFT_BREAKPOINT_REACHED,
    MSG_CPU_HARD_BREAKPOINT_REACHED,
    MSG_CPU_ILLEGAL_INSTRUCTION,
    
    // Keyboard
    MSG_KEYMATRIX,
    MSG_CHARSET,
    
    // Peripherals (Disk drive)
    MSG_VC1541_ATTACHED,
    MSG_VC1541_ATTACHED_SOUND,
    MSG_VC1541_DETACHED,
    MSG_VC1541_DETACHED_SOUND,
    MSG_VC1541_DISK,
    MSG_VC1541_DISK_SOUND,
    MSG_VC1541_NO_DISK,
    MSG_VC1541_NO_DISK_SOUND,
    MSG_VC1541_MOTOR_ON,
    MSG_VC1541_MOTOR_OFF,
    
    // Peripherals (Disk)
    MSG_DISK_SAVED,
    MSG_DISK_UNSAVED,
}
MessageType;

/* A single message
 * Only a very messages utilize the data file. E.g., the drive related message
 * use it to code the drive number (0 = df0 etc.).
 */
typedef struct
{
    MessageType type;
    long data;
}
Message;

// Callback function signature
typedef void Callback(const void *, unsigned, long);

#endif