// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include <dirent.h>
#include "DIRFile.h"
#include "FSVolume.h"

namespace fs = std::filesystem;

DIRFile::DIRFile()
{
    setDescription("DIRFile");
}


bool
DIRFile::isDIRFile(const char *path)
{
    assert(path != nullptr);
    return isDirectory(path);
}

bool
DIRFile::bufferHasSameType(const u8 *buffer, size_t length)
{
    assert(false);
    return false;
}

DIRFile *
DIRFile::makeWithFile(const char *path)
{
    DIRFile *dir = new DIRFile();
    
    if (!dir->readFromFile(path)) {
        delete dir;
        return nullptr;
    }
    
    return dir;
}

bool
DIRFile::readFromBuffer(const u8 *buffer, size_t length)
{
    assert(false);
    return false;
}

bool
DIRFile::readFromFile(const char *filename)
{
    bool success = false;
    
    debug("DIRFile::readFromFile(%s)\n", filename);
              
    if (!isDIRFile(filename)) {
        warn("%s is not a directory\n", filename);
        return false;
    }
    
    // Create a new file system
    OFSVolume volume = OFSVolume("Disk", 2 * 880);
    
    // Make the volume bootable
    volume.installBootBlock();
    
    // Crawl through the given directory and add all files
    success = traverseDir(filename, volume);
    debug("traverseDir result = %d\n", success);
    
    // Check for file system errors
    volume.changeDir("/");
    volume.info();
    volume.walk(true);

    if (!volume.check(MFM_DEBUG)) {
        warn("DIRFile::readFromFile: Files system is corrupted.\n");
        // volume.dump();
    }

    // Convert the volume into an ADF
    assert(adf == nullptr);
    if (success) adf = ADFFile::makeWithVolume(volume);
    debug("adf = %p\n", adf); 
    return adf != nullptr;
}

bool 
DIRFile::traverseDir(const char *dir, FSVolume &vol) {
    
    assert(dir != nullptr);
    
    bool result = true;
    DIR *dp;
    struct dirent *dirp;

    if (!(dp = opendir(dir))) {
        warn("Error opening directory %s\n", dir);
        return false;
    }

    while ((dirp = readdir(dp))) {

        // Skip '.', '..', and all hidden files
        if (dirp->d_name[0] == '.') continue;

        // msg("%s/%s\n", dir, dirp->d_name);

        if (dirp->d_type == DT_DIR) {
            
            // Add subdirectory to volume
            if (vol.makeDir(dirp->d_name) == nullptr) result = false;
            vol.changeDir(dirp->d_name);

            // Recursively process the subdirectory
            char *subdir = new char [strlen(dir) + strlen(dirp->d_name) + 2];
            strcpy(subdir, dir);
            strcat(subdir, "/");
            strcat(subdir, dirp->d_name);
            result &= traverseDir(subdir, vol);
            delete [] subdir;
            continue;
        }
        
        // Load file from disk
        u8 *buffer; long size;
        if (loadFile(dir, dirp->d_name, &buffer, &size)) {

            // Add file to volume
            FSBlock *file = vol.makeFile(dirp->d_name);
            if (file) {
                result &= file->append(buffer, size);
            } else {
                result = false;
            }

            delete(buffer);
        }
    }
    
    closedir(dp);
    return result;
}
