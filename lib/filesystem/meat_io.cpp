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

MFile* MFSOwner::File(std::string name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
            return fs->getFile(name);
        }
    }
    
    return nullptr;
}

bool MFSOwner::mount(std::string name) {
    uint i = 0;
    Serial.print("MFSOwner::mount fs:");
    Serial.print(name.c_str());

    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->handles(name)) {
                Serial.println("MFSOwner found a proper fs");

            bool ok = fs->mount();

            if(ok)
                Serial.print("Mounted fs:");
            else
                Serial.print("Couldn't mount fs:");

            Serial.print(name.c_str());

            return ok;
        }
    }
    return false;
}

bool MFSOwner::umount(std::string name) {
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

// bool MFileSystem::handles(std::string path) 
// {
//     return path.startsWith(protocol);
// }

/********************************************************
 * MFile implementations
 ********************************************************/

MFile::MFile(std::string path) {
    if(path.back()=='\\')
        path.erase(path.length()-1,1);

    m_path = path;
}

MFile::MFile(std::string path, std::string name) : MFile(path + "/" + name) {}

MFile::MFile(MFile* path, std::string name) : MFile(path->m_path + "/" + name) {}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}

std::string MFile::name() {
    int lastSlash = m_path.find_last_of('/');

    std::string test = m_path.substr(lastSlash+1);    

    Serial.printf("last slash in %s=%d --> %s\n", m_path.c_str(), lastSlash, test.c_str());
    return test;
}    

std::string MFile::path() {
    return m_path;
}    

std::string MFile::extension() {
    int lastPeriod = m_path.find_last_of(".");
    return m_path.substr(lastPeriod+1);
}