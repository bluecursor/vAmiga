// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_H
#define _AMIGA_H

// General
#include "AmigaComponent.h"
#include "Serialization.h"
#include "MessageQueue.h"

// Sub components
#include "Agnus.h"
#include "Blitter.h"
#include "ControlPort.h"
#include "CIA.h"
#include "Copper.h"
#include "CPU.h"
#include "Denise.h"
#include "Disk.h"
#include "Drive.h"
#include "Joystick.h"
#include "Keyboard.h"
#include "Memory.h"
#include "Moira.h"
#include "Mouse.h"
#include "RTC.h"
#include "Paula.h"
#include "SerialPort.h"
#include "ZorroManager.h"

// File types
#include "RomFile.h"
#include "EncryptedRomFile.h"
#include "ExtFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "DIRFile.h"
#include "FSVolume.h"

/* A complete virtual Amiga. This class is the most prominent one of all. To
 * run the emulator, it is sufficient to create a single object of this type.
 * All subcomponents are created automatically. The public API gives you
 * control over the emulator's behaviour such as running and pausing the
 * emulation. Please note that most subcomponents have their own public API.
 * E.g., to query information from Paula, you need to invoke a public method on
 * amiga.paula.
 */
class Amiga : public HardwareComponent {

    /* The inspection target. In order to update the GUI periodically, the
     * emulator schedules this event in the inspector slot (INS_SLOT in the
     * secondary table) on a periodic basis. If the event is EVENT_NONE, no
     * action is taken. If an INS_xxx event is scheduled, inspect() is called
     * on a certain Amiga component.
     */
    EventID inspectionTarget = INS_NONE;

    // Result of the latest inspection
    AmigaInfo info;

     
    //
    // Sub components
    //
    
public:
    
    // Core components
    CPU cpu = CPU(*this);
    CIAA ciaA = CIAA(*this);
    CIAB ciaB = CIAB(*this);
    RTC rtc = RTC(*this);
    Memory mem = Memory(*this);
    Agnus agnus = Agnus(*this);
    Denise denise = Denise(*this);
    Paula paula = Paula(*this);
    ZorroManager zorro = ZorroManager(*this);
    
    // Ports
    ControlPort controlPort1 = ControlPort(PORT_1, *this);
    ControlPort controlPort2 = ControlPort(PORT_2, *this);
    SerialPort serialPort = SerialPort(*this);

    // Peripherals
    Keyboard keyboard = Keyboard(*this);

    // Floppy drives
    Drive df0 = Drive(0, *this);
    Drive df1 = Drive(1, *this);
    Drive df2 = Drive(2, *this);
    Drive df3 = Drive(3, *this);
    
    // Shortcuts to all four drives
    Drive *df[4] = { &df0, &df1, &df2, &df3 };
    
    //
    // Message queue
    //
    
    /* Communication channel to the GUI. The GUI registers a listener and a
     * callback function to retrieve messages.
     */
    MessageQueue messageQueue;

    
    //
    // Emulator thread
    //
    
private:
    
    /* Run loop control. This variable is checked at the end of each runloop
     * iteration. Most of the time, the variable is 0 which causes the runloop
     * to repeat. A value greater than 0 means that one or more runloop control
     * flags are set. These flags are flags processed and the loop either
     * repeats or terminates depending on the provided flags.
     */
    u32 runLoopCtrl = 0;
    
    // The invocation counter for implementing suspend() / resume()
    unsigned suspendCounter = 0;
    
    // The emulator thread
    pthread_t p = NULL;
    
    /* Mutex to coordinate the ownership of the emulator thread.
     *
     */
    pthread_mutex_t threadLock;
    
    /* Lock to synchronize the access to all state changing methods such as
     * run(), pause(), etc.
     */
    pthread_mutex_t stateChangeLock;
    
    
    //
    // Emulation speed
    //
    
private:
    
    /* System timer information. Used to match the emulation speed with the
     * speed of a real Amiga.
     */
    mach_timebase_info_data_t tb;
    
    /* Inside restartTimer(), the current time and the DMA clock cylce
     * are recorded in these variables. They are used in sychronizeTiming()
     * to determine how long the thread has to sleep.
     */
    Cycle clockBase = 0;
    u64 timeBase = 0;

        
    //
    // Snapshot storage
    //
    
private:
    
    Snapshot *autoSnapshot = NULL;
    Snapshot *userSnapshot = NULL;

    
    //
    // Initializing
    //
    
public:
    
    Amiga();
    ~Amiga();

    void prefix() override;

    void reset(bool hard);
    void hardReset() { reset(true); }
    void softReset() { reset(false); }

private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }

    
    //
    // Configuring
    //
    
public:
    
    // Returns the current configuration
    AmigaConfiguration getConfig();
    
    // Gets a single configuration item
    long getConfigItem(ConfigOption option);
    long getConfigItem(unsigned dfn, ConfigOption option);
    
    // Sets a single configuration item
    bool configure(ConfigOption option, long value);
    bool configure(unsigned dfn, ConfigOption option, long value);
    
    
    //
    // Analyzing
    //
    
public:
    
    AmigaInfo getInfo() { return HardwareComponent::getInfo(info); }
    
    void setInspectionTarget(EventID id);
    void clearInspectionTarget();
    
private:
    
    void _inspect() override;
    void _dump() override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker & clockBase;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Controlling
    //
    
public:
    
    void powerOn();
    void powerOff();
    void run();
    void pause();
    
    void setWarp(bool enable);
    bool inWarpMode() { return warpMode; }
    void enableWarpMode() { setWarp(true); }
    void disableWarpMode() { setWarp(false); }

    void enableDebugMode() { setDebug(true); }
    void disableDebugMode() { setDebug(false); }
    bool inDebugMode() { return debugMode; }

private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _run() override;
    void _pause() override;
    void _setWarp(bool enable) override;

    
    //
    // Working with the emulator thread
    //
    
public:
    
    /* Requests the emulator thread to stop and locks the threadLock. The
     * function is called in all state changing methods to obtain ownership
     * of the emulator thread. After returning, the emulator is either powered
     * off (if it was powered off before) or paused (if it was running before).
     */
    void acquireThreadLock();
    
    /* Returns true if a call to powerOn() will be successful. It returns false,
     * e.g., if no Kickstart Rom or Boot Rom is installed.
     */
    bool isReady(ErrorCode *error = NULL);
    
    /* Pauses the emulation thread temporarily. Because the emulator is running
     * in a separate thread, the GUI has to pause the emulator before changing
     * it's internal state. This is done by embedding the code inside a
     * suspend / resume block:
     *
     *            suspend();
     *            do something with the internal state;
     *            resume();
     *
     *  It it safe to nest multiple suspend() / resume() blocks.
     */
    void suspend();
    void resume();
    
    /* Sets or clears a run loop control flag. The functions are thread-safe
     * and can be called from inside or outside the emulator thread.
     */
    void setControlFlags(u32 flags);
    void clearControlFlags(u32 flags);
    
    // Convenience wrappers for controlling the run loop
    void signalAutoSnapshot() { setControlFlags(RL_AUTO_SNAPSHOT); }
    void signalUserSnapshot() { setControlFlags(RL_USER_SNAPSHOT); }
    void signalInspect() { setControlFlags(RL_INSPECT); }
    void signalStop() { setControlFlags(RL_STOP); }


    //
    // Running the emulator
    //
    
public:
    
    // Runs or pauses the emulator
    void stopAndGo();
    
    /* Executes a single instruction. This function is used for single-stepping
     * through the code inside the debugger. It starts the execution thread and
     * terminates it after the next instruction has been executed.
     */
    void stepInto();
    
    /* Runs the emulator until the instruction following the current one is
     * reached. This function is used for single-stepping through the code
     * inside the debugger. It sets a soft breakpoint to PC+n where n is the
     * length bytes of the current instruction and starts the emulator thread.
     */
    void stepOver();
    
    /* The thread enter function. This (private) method is invoked when the
     * emulator thread launches. It has to be declared public to make it
     * accessible by the emulator thread.
     */
    void threadWillStart();
    
    /* The thread exit function. This (private) method is invoked when the
     * emulator thread terminates. It has to be declared public to make it
     * accessible by the emulator thread.
     */
    void threadDidTerminate();
    
    /* The Amiga run loop. This function is one of the most prominent ones. It
     * implements the outermost loop of the emulator and therefore the place
     * where emulation starts. If you want to understand how the emulator works,
     * this function should be your starting point.
     */
    void runLoop();

    
    //
    // Managing emulation speed
    //
    
public:

    /* Restarts the synchronization timer. This function is invoked at launch
     * time to initialize the timer and reinvoked when the synchronization timer
     * got out of sync.
     */
    void restartTimer();
    
private:
    
    // Converts kernel time to nanoseconds
    u64 abs_to_nanos(u64 abs) { return abs * tb.numer / tb.denom; }
    
    // Converts nanoseconds to kernel time
    u64 nanos_to_abs(u64 nanos) { return nanos * tb.denom / tb.numer; }
    
    // Returns the current time in nanoseconds
    u64 time_in_nanos() { return abs_to_nanos(mach_absolute_time()); }
    
    /* Returns the delay between two frames in nanoseconds. As long as we only
     * emulate PAL machines, the frame rate is 50 Hz and this function returns
     * a constant.
     */
    u64 frameDelay() { return u64(1000000000) / 50; }
    
public:
    
    // Puts the emulator thread to sleep
    void synchronizeTiming();
    
    
    //
    // Handling snapshots
    //
    
public:
    
    /* Requests a snapshot to be taken. Once the snapshot is ready, a message
     * is written into the message queue. The snapshot can then be picked up by
     * calling latestAutoSnapshot() or latestUserSnapshot(), depending on the
     * requested snapshot type.
     */
    void requestAutoSnapshot();
    void requestUserSnapshot();
     
    // Returns the most recent snapshot or NULL if none was taken
    Snapshot *latestAutoSnapshot();
    Snapshot *latestUserSnapshot();

    /* Loads the current state from a snapshot file. There is an thread-unsafe
     * and thread-safe version of this function. The first one can be unsed
     * inside the emulator thread or from outside if the emulator is halted.
     * The second one can be called any time.
     */
    void loadFromSnapshotUnsafe(Snapshot *snapshot);
    void loadFromSnapshotSafe(Snapshot *snapshot);
};

#endif
