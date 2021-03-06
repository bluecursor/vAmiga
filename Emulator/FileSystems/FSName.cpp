// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSName.h"
#include "Utils.h"

FSName::FSName(const char *str)
{
    assert(str != nullptr);
    
    // Amiga file and volume names are limited to 30 characters
    strncpy(name, str, 30);
    
    // Replace all symbols that are not permitted in Amiga filenames
    for (size_t i = 0; i < sizeof(name); i++) {
        if (name[i] == ':' || name[i] == '/') name[i] = '_';
    }
    
    // Make sure the string terminates
    name[30] = 0;
}

char
FSName::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSName::operator== (FSName &rhs)
{
    int n = 0;
    
    while (name[n] != 0 || rhs.name[n] != 0) {
        if (capital(name[n]) != capital(rhs.name[n])) return false;
        n++;
    }
    return true;
}

u32
FSName::hashValue()
{
    size_t length = strlen(name);
    u32 result = (u32)length;
    
    for (size_t i = 0; i < length; i++) {
        char c = capital(name[i]);
        result = (result * 13 + (u32)c) & 0x7FF;
    }
    return result % 72;
}

void
FSName::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(name) < sizeof(name));

    // Write name as BCPL string (first byte is string length)
    p[0] = strlen(name);
    strncpy((char *)(p + 1), name, strlen(name));
}

FSComment::FSComment(const char *str)
{
    assert(str != nullptr);
    
    // Comments are limited to 91 characters
    strncpy(name, str, 91);
        
    // Make sure the string terminates
    name[30] = 0;
}

void
FSComment::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(name) < sizeof(name));
    
    // Write name as BCPL string (first byte is string length)
    p[0] = strlen(name);
    strncpy((char *)(p + 1), name, strlen(name));
}
