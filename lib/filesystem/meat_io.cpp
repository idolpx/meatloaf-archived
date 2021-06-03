#include "meat_io.h"
#include "fs_smb.h"
#include "fs_d64.h"
#include "fs_littlefs.h"
#include "flash_hal.h"
#include "MIOException.h"
/********************************************************
 * MFSOwner implementations
 ********************************************************/

std::shared_ptr<LittleFileSystem> littleFS(new LittleFileSystem("/",FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, 5));

MFileSystem* MFSOwner::availableFS[FS_COUNT] = { littleFS.get() };

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
    Serial.print("MFSOwner::mount fs:");
    Serial.print(name);

    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
                Serial.println("MFSOwner found a proper fs");

            bool ok = fs->mount();

            if(ok)
                Serial.print("Mounted fs:");
            else
                Serial.print("Couldn't mount fs:");

            Serial.print(name);

            return ok;
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

MFile::MFile(String path) {
    if(path.endsWith("/"))
        path.remove(path.length()-1);

    m_path = path;
}

MFile::MFile(String path, String name) : MFile(path + "/" + name) {}

MFile::MFile(MFile* path, String name) : MFile(path->m_path + "/" + name) {}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}

const char* MFile::name() const {
    int lastSlash = m_path.lastIndexOf("/");
    return m_path.substring(lastSlash+1).c_str();
}    

const char* MFile::path() const {
    return m_path.c_str();
}    

const char* MFile::extension() const {
    String name = this->name();
    int lastPeriod = name.lastIndexOf(".");
    return m_path.substring(lastPeriod+1).c_str();
}    

