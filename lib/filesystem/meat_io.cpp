#include "meat_io.h"
#include "fs_smb.h"
#include "fs_littlefs.h"
#include "fs_http.h"
#include "flash_hal.h"
#include "MIOException.h"
/********************************************************
 * MFSOwner implementations
 ********************************************************/

// initialize other filesystems here
LittleFileSystem littleFS("/",FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, 5);
HttpFileSystem httpFS("http://");

// put all available filesystems in this array
// put littleFS as last, fallback system so it can be used if nothing matches
MFileSystem* MFSOwner::availableFS[FS_COUNT] = { &httpFS, &littleFS };

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

// bool MFileSystem::handles(String path) 
// {
//     return path.startsWith(protocol);
// }

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

String MFile::name() {
    int lastSlash = m_path.lastIndexOf("/");

    String test = m_path.substring(lastSlash+1);    

    Serial.printf("last slash in %s=%d --> %s\n", m_path.c_str(), lastSlash, test.c_str());
    return test;
}    

String MFile::path() {
    return m_path;
}    

String MFile::extension() {
    int lastPeriod = m_path.lastIndexOf(".");
    return m_path.substring(lastPeriod+1);
}