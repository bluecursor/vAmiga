// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Copper::Copper()
{
    setDescription("Copper");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &copList,   sizeof(copList),   0 },
        { &skip,      sizeof(skip),      0 },
        { &cop1lc,    sizeof(cop1lc),    0 },
        { &cop2lc,    sizeof(cop2lc),    0 },
        { &cop1end,   sizeof(cop1end),   0 },
        { &cop2end,   sizeof(cop2end),   0 },
        { &cdang,     sizeof(cdang),     0 },
        { &cop1ins,   sizeof(cop1ins),   0 },
        { &cop2ins,   sizeof(cop2ins),   0 },
        { &coppc,     sizeof(coppc),     0 },
        // { &coppcBase, sizeof(coppcBase), 0 },
    });
}

void
Copper::_initialize()
{
    mem = &amiga->mem;
    agnus = &amiga->agnus;
    events = &amiga->agnus.events;
    colorizer = &amiga->denise.colorizer;
}

void
Copper::_powerOn()
{
    
}

void
Copper::_powerOff()
{
    
}

void
Copper::_reset()
{
    
}

void
Copper::_ping()
{
    
}

void
Copper::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.cdang   = cdang;
    info.active  = events->isPending<COP_SLOT>();
    info.coppc   = coppc; // coppcBase;
    info.cop1ins = cop1ins;
    info.cop2ins = cop2ins;
    info.cop1lc  = cop1lc;
    info.cop2lc  = cop2lc;
    info.length1 = cop1end - cop1lc;
    info.length2 = cop2end - cop2lc;

    pthread_mutex_unlock(&lock);
}

void
Copper::_dump()
{
    bool active = events->isPending<COP_SLOT>();
    plainmsg("    cdang: %d\n", cdang);
    plainmsg("   active: %s\n", active ? "yes" : "no");
    if (active) plainmsg("    state: %d\n", events->slot[COP_SLOT].id);
    plainmsg("    coppc: %X\n", coppc);
    plainmsg("  copins1: %X\n", cop1ins);
    plainmsg("  copins2: %X\n", cop2ins);
    plainmsg("   cop1lc: %X\n", cop1lc);
    plainmsg("   cop2lc: %X\n", cop2lc);
    plainmsg("  cop1end: %X\n", cop1end);
    plainmsg("  cop2end: %X\n", cop2end);

    // verbose = !verbose;

    /*
    plainmsg("\n");
    plainmsg("First Copper list\n");
    plainmsg("-----------------\n");

    uint32_t addr = cop1lc;
    for (int i = 0; i < 100; i++, addr += 4) {
        plainmsg("%04X %04X: ", mem->spypeek16(addr), mem->spypeek16(addr+2));
        plainmsg(disassemble(addr));
        plainmsg("\n");
    }
    */
}

CopperInfo
Copper::getInfo()
{
    CopperInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
Copper::pokeCOPCON(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOPCON(%04X)\n", value);
    
    /* "This is a 1-bit register that when set true, allows the Copper to
     *  access the blitter hardware. This bit is cleared by power-on reset, so
     *  that the Copper cannot access the blitter hardware." [HRM]
     */
    cdang = (value & 0b10) != 0;
}

void
Copper::pokeCOPJMP1()
{
    debug(COPREG_DEBUG, "pokeCOPJMP1(): Jumping to %X\n", cop1lc);
    switchToCopperList(1);
}

void
Copper::pokeCOPJMP2()
{
    debug(COPREG_DEBUG, "pokeCOPJMP2(): Jumping to %X\n", cop2lc);
    switchToCopperList(2);
}

void
Copper::pokeCOPINS(uint16_t value)
{
    debug(COPREG_DEBUG, "COPPC: %X pokeCOPINS(%04X)\n", coppc, value);

    /* COPINS is a dummy address that can be used to write the first or
     * the secons instruction register, depending on the current state.
     */

    // TODO: The following is almost certainly wrong...
    /* if (state == COP_MOVE || state == COP_WAIT_SKIP) {
        cop2ins = value;
    } else {
        cop1ins = value;
    }
    */
    cop1ins = value;
}

void
Copper::pokeCOP1LCH(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCH(%04X)\n", value);

    if (HI_WORD(cop1lc) != value) {
        cop1lc = REPLACE_HI_WORD(cop1lc, value);
        cop1end = cop1lc;
    }

    // Update program counter if DMA is off
    /* THIS IS NOT 100% CORRECT. IN WINFELLOW, THE PC IS ONLY WRITTEN TO IF
     * DMA WAS OFF SINCE THE LAST VSYNC EVENT (?!). NEED A TEST CASE FOR THIS.
     */
    if (!agnus->copDMA()) switchToCopperList(1);
}

void
Copper::pokeCOP1LCL(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCL(%04X)\n", value);

    if (LO_WORD(cop1lc) != value) {
        cop1lc = REPLACE_LO_WORD(cop1lc, value & 0xFFFE);
        cop1end = cop1lc;
    }

    // Update program counter if DMA is off
    /* THIS IS NOT 100% CORRECT. IN WINFELLOW, THE PC IS ONLY WRITTEN TO IF
     * DMA WAS OFF SINCE THE LAST VSYNC EVENT (?!). NEED A TEST CASE FOR THIS.
     */
    if (!agnus->copDMA()) switchToCopperList(1);
}

void
Copper::pokeCOP2LCH(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCH(%04X)\n", value);

    if (HI_WORD(cop2lc) != value) {
        cop2lc = REPLACE_HI_WORD(cop2lc, value);
        cop2end = cop2lc;
    }
}

void
Copper::pokeCOP2LCL(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCL(%04X)\n", value);

    if (LO_WORD(cop2lc) != value) {
        cop2lc = REPLACE_LO_WORD(cop2lc, value & 0xFFFE);
        cop2end = cop2lc;
    }
}

void
Copper::pokeNOOP(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeNOOP(%04X)\n", value);

    // REMOVE ASAP
    verbose = !verbose; 
}

void
Copper::switchToCopperList(int nr)
{
    assert(nr == 1 || nr == 2);

    coppc = (nr == 1) ? cop1lc : cop2lc;
    copList = nr;
}

bool
Copper::findMatch(Beam &result)
{
    int16_t vMatch, hMatch;

    // Get the current beam position
    Beam b = agnus->beamPosition();

    // Advance to the position where the comparator circuit gets active
    b = agnus->addToBeam(b, 4);

    // Set up the comparison positions
    int16_t vComp = getVP();
    int16_t hComp = getHP();

    // Set up the comparison masks
    int16_t vMask = getVM() | 0x80;
    int16_t hMask = getHM() & 0xFE;

    // Check if the current line matches the vertical trigger position
    if ((b.y & vMask) >= (vComp & vMask)) {

        // Check if we find a horizontal match in this line
        if (findHorizontalMatch(b.x, hComp, hMask, hMatch)) {

            // Success. We've found a match in the current line
            result.y = b.y;
            result.x = hMatch;
            return true;
        }
    }

    // Find the first vertical match below the current line
    if (!findVerticalMatch(b.y + 1, vComp, vMask, vMatch)) return false;

    // Find the first horizontal match in that line
    if (!findHorizontalMatch(0, hComp, hMask, hMatch)) return false;

    // Success. We've found a match below the current line
    result.y = vMatch;
    result.x = hMatch;
    return true;
}

bool
Copper::findVerticalMatch(int16_t vStrt, int16_t vComp, int16_t vMask, int16_t &result)
{
    int16_t vStop = agnus->frameInfo.numLines;

    // Iterate through all vertical positions
    for (int v = vStrt; v < vStop; v++) {

        // Check if the comparator triggers at this position
        if ((v & vMask) >= (vComp & vMask)) {
            result = v;
            return true;
        }
    }
    return false;
}

bool
Copper::findHorizontalMatch(int16_t hStrt, int16_t hComp, int16_t hMask, int16_t &result)
{
    int16_t hStop = agnus->DMACyclesPerLine(); 

    // Iterate through all horizontal positions
    for (int h = hStrt; h < hStop; h++) {

        // Check if the comparator triggers at this position
        if ((h & hMask) >= (hComp & hMask)) {
            result = h;
            return true;
        }
    }
    return false;
}

void
Copper::move(int addr, uint16_t value)
{
    debug(COP_DEBUG, "COPPC: %X move(%s, $%X) (%d)\n", coppc, customReg[addr >> 1], value, value);

    assert(IS_EVEN(addr));
    assert(addr < 0x1FF);

    // Catch registers with special timing needs

    if (addr >= 0x180 && addr <= 0x1BE) {

         // Color registers
        int reg = (addr - 0x180) / 2;
        colorizer->recordColorRegisterChange(reg, value, 4 * agnus->hpos);
        return;
    }

    // Write the value
    agnus->copperWrite(addr, value);
}

bool
Copper::comparator(uint32_t beam, uint32_t waitpos, uint32_t mask)
{
    // Get comparison bits for the vertical beam position
    uint8_t vBeam = (beam >> 8) & 0xFF;
    uint8_t vWaitpos = (waitpos >> 8) & 0xFF;
    uint8_t vMask = (mask >> 8) | 0x80;
    
    // debug(" * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);
    // Compare vertical positions
    if ((vBeam & vMask) < (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x FALSE\n", beam, waitpos, mask);
        return false;
    }
    if ((vBeam & vMask) > (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x TRUE\n", beam, waitpos, mask);
        return true;
    }

    // Get comparison bits for horizontal position
    uint8_t hBeam = beam & 0xFE;
    uint8_t hWaitpos = waitpos & 0xFE;
    uint8_t hMask = mask & 0xFE;
    
    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(uint32_t beam)
{
    return comparator(beam, getVPHP(), getVMHM());
}

bool
Copper::comparator()
{
    return comparator(getVPHP());
}

uint32_t
Copper::nextTriggerPosition()
{
    // Get the current beam position
    Beam b = agnus->beamPosition();

    // Advance two cycles to get to the first possible trigger position
    b = agnus->addToBeam(b, 2);

    // Translate position to a binary 17-bit representation
    uint32_t beam = (b.y << 8) + b.x;

    /* We are going to compute the smallest beam position satisfying
     *
     *   1) computed position >= current beam position + 2,
     *   2) the comparator circuit triggers.
     *
     * We do this by starting with the maximum possible value:
     */
    uint32_t pos = 0x1FFE2;

    /* Now, we iterate through bit from left to right and set the bit we see
     * to 0 as long as conditions 1) and 2) hold.
     */
    for (int i = 16; i >= 0; i--) {
        uint32_t newPos = pos & ~(1 << i);
        if (newPos >= beam && comparator(newPos)) {
            pos = newPos;
        }
    }

    return pos;
}

bool
Copper::isMoveCmd()
{
    return !(cop1ins & 1);
}

bool Copper::isMoveCmd(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return !(HI_WORD(instr) & 1);
}

bool Copper::isWaitCmd()
{
     return (cop1ins & 1) && !(cop2ins & 1);
}

bool Copper::isWaitCmd(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return (HI_WORD(instr) & 1) && !(LO_WORD(instr) & 1);
}

bool
Copper::isSkipCmd()
{
    return (cop1ins & 1) && (cop2ins & 1);
}

bool
Copper::isSkipCmd(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return (HI_WORD(instr) & 1) && (LO_WORD(instr) & 1);
}

uint16_t
Copper::getRA()
{
    return cop1ins & 0x1FE;
}

uint16_t
Copper::getRA(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return HI_WORD(instr) & 0x1FE;
}

uint16_t
Copper::getDW()
{
    return cop1ins;
}

uint16_t
Copper::getDW(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return LO_WORD(instr);
}

bool
Copper::getBFD()
{
    return (cop2ins & 0x8000) != 0;
}

bool
Copper::getBFD(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return (LO_WORD(instr) & 0x8000) != 0;
}

uint16_t
Copper::getVPHP()
{
    return cop1ins & 0xFFFE;
}

uint16_t
Copper::getVPHP(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return HI_WORD(instr) & 0xFFFE;
}

uint16_t
Copper::getVMHM()
{
    return (cop2ins & 0x7FFE) | 0x8001;
}

uint16_t
Copper::getVMHM(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return (LO_WORD(instr) & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(uint32_t addr)
{
    addr &= 0x1FE;
    return addr < (cdang ? 0x40 : 0x80);
}

bool
Copper::isIllegalInstr(uint32_t addr)
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::serviceEvent(EventID id)
{
    uint16_t reg;

    debug(2, "(%d,%d): ", agnus->vpos, agnus->hpos);

    servicing = true;

    switch (id) {
            
        case COP_REQ_DMA:

            if (verbose) debug("COP_REQ_DMA\n");

            // Wait for the next free DMA cycle
            if (!agnus->copperCanHaveBus()) { reschedule(); break; }

            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;

        case COP_FETCH:

            if (verbose) debug("COP_FETCH\n");

            // Wait for the next free DMA cycle
            if (!agnus->copperCanHaveBus()) { reschedule(); break; }

            // Load the first instruction word
            cop1ins = agnus->copperRead(coppc);
            // coppcBase = coppc;
            advancePC();

            // Dynamically determine the end of the Copper list
            if (copList == 1) {
                if (coppc > cop1end) cop1end = coppc;
            } else {
                if (coppc > cop2end) cop2end = coppc;
            }

            // Fork execution depending on the instruction type
            schedule(isMoveCmd() ? COP_MOVE : COP_WAIT_SKIP);
            break;
            
        case COP_MOVE:

            if (verbose) debug("COP_MOVE\n");

            // Wait for the next free DMA cycle
            if (!agnus->copperCanHaveBus()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus->copperRead(coppc);
            advancePC();

            // Extract register number from the first instruction word
            reg = (cop1ins & 0x1FE);

            // Stop the Copper if address is illegal
            if (isIllegalAddress(reg)) { events->cancel<COP_SLOT>(); break; }

            // Write into the custom register
            if (!skip) move(reg, cop2ins);
            skip = false;

            // Schedule next event
            schedule(COP_FETCH);
            break;
            
        case COP_WAIT_SKIP:

            if (verbose) debug("COP_WAIT_SKIP\n");

            // Wait for the next free DMA cycle
            if (!agnus->copperCanHaveBus()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus->copperRead(coppc);
            advancePC();
                
            // Is it a WAIT command?
            if (isWaitCmd()) {

                // Clear the skip flag
                skip = false;

                // Find the trigger position for this WAIT command
                Beam trigger;
                if (findMatch(trigger)) {

                    // In how many cycles do we get there?
                    Cycle delay = agnus->beamDiff(trigger.y, trigger.x);
                    assert(delay < NEVER);

                    if (verbose) debug("FOUND MATCH in %d cycles\n", delay);

                    // Copper wakes up 2 cycles earlier...
                    delay -= DMA_CYCLES(2);

                    // ... with a COP_REQ_DMA event.
                    events->scheduleRel<COP_SLOT>(delay, COP_REQ_DMA);

                } else {

                    events->cancel<COP_SLOT>();
                }

            } else {

                if (verbose) debug("SKIP\n");

                // It must be a SKIP command then.
                assert(isSkipCmd());

                // Run the comparator to see if the next command is skipped
                skip = comparator();

                // Continue with fetching the next command
                schedule(COP_FETCH);
            }
            break;
            
        case COP_JMP1:

            if (verbose) debug("COP_JMP1\n");

            // Perform the jump
            switchToCopperList(1);

            // Request a free bus slot
            schedule(COP_REQ_DMA);
            break;

        case COP_JMP2:

            if (verbose) debug("COP_JMP2\n");

            // Perform the jump
            switchToCopperList(2);

            // Request a free bus slot
            schedule(COP_REQ_DMA);
            break;

        default:
            
            assert(false);
            break;
    }

    servicing = false;
}

void
Copper::schedule(EventID next)
{
    events->scheduleInc<COP_SLOT>(DMA_CYCLES(2), next);
}

void
Copper::reschedule()
{
    events->rescheduleInc<COP_SLOT>(DMA_CYCLES(2));
}

void
Copper::vsyncAction()
{
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COPlLC." [HRM]
     */

    // TODO: What is the exact timing here?
    if (agnus->copDMA()) {
        events->scheduleRel<COP_SLOT>(DMA_CYCLES(4), COP_JMP1);
    } else {
        events->cancel<COP_SLOT>();
    }
}

int
Copper::instrCount(int nr)
{
    assert(nr == 1 || nr == 2);

    int strt = (nr == 1) ? cop1lc  : cop2lc;
    int stop = (nr == 1) ? cop1end : cop2end;

    return 1 + (stop - strt) / 4;
}


char *
Copper::disassemble(uint32_t addr)
{
    char pos[16];
    char mask[16];
    
    if (isMoveCmd(addr)) {
        
        uint16_t reg = getRA(addr) >> 1;
        assert(reg <= 0xFF);
        sprintf(disassembly, "MOVE $%04X, %s", getDW(addr), customReg[reg]);
        return disassembly;
    }
    
    const char *mnemonic = isWaitCmd(addr) ? "WAIT" : "SKIP";
    const char *suffix = getBFD(addr) ? "*" : "";
    
    sprintf(pos, "($%02X,$%02X)", getVP(addr), getHP(addr));
    
    if (getVM(addr) == 0xFF && getHM(addr) == 0xFF) {
        sprintf(mask, "");
    } else {
        sprintf(mask, ", ($%02X,$%02X)", getHM(addr), getVM(addr));
    }
    
    sprintf(disassembly, "%s%s %s%s", mnemonic, suffix, pos, mask);
    return disassembly;
}

char *
Copper::disassemble(unsigned list, uint32_t offset)
{
    assert(list == 1 || list == 2);
    
    uint32_t addr = (list == 1) ? cop1lc : cop2lc;
    addr = (addr + 2 * offset) & 0x7FFFF;

    return disassemble(addr);
}
