#include "meat_io.h"
#include "fs_smb.h"
#include "fs_d64.h"
#include "fs_littlefs.h"
#include "flash_hal.h"
#include "MIOException.h"
/********************************************************
 * MFSOwner implementations
 ********************************************************/

LittleFileSystem* littleFS = new LittleFileSystem("/",FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, 5);

MFileSystem* MFSOwner::availableFS[FS_COUNT] = { littleFS, /*new D64FileSystem(""), new SMBFileSystem("smb:")*/ };

MFile* MFSOwner::File(String name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
            return fs->getFile(name);
        }
    }
    
    return nullptr;
}

bool MFSOwner::mount(String name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
            return fs->mount();
        }
    }
    return false;
}

bool MFSOwner::umount(String name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
            return fs->umount();
        }
    }
    return true;
}


/********************************************************
 * MFileSystem implementations
 ********************************************************/

MFileSystem::MFileSystem(char* prefix)
{
    protocol = prefix;
}

bool MFileSystem::handles(String path) 
{
    return path.startsWith(protocol);
}

/********************************************************
 * MFile implementations
 ********************************************************/

MFile::MFile(String path, String name) {
    // create this instance from path and fileName
}

MFile::MFile(MFile* path, String name) {
    // create this instance from MFile and fileName
}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}

const char* MFile::name() const {
    // return just the file name
    // TODO!
}    

const char* MFile::path() const {
    // return full path
    return m_path.c_str();
}    

const char* MFile::extension() const {
    // return the file extension
}    

/********************************************************
 * MStream implementations
 ********************************************************/

bool MStream::isOpen() {
    return m_isOpen;
}
