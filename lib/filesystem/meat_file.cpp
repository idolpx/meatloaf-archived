#include "meat_file.h"
#include "fs_smb.h"
#include "fs_d64.h"

/********************************************************
 * MFSOwner implementations
 ********************************************************/

MFileSystem* MFSOwner::availableFS[FS_COUNT] = { /*new D64FileSystem(""), new SMBFileSystem("smb:")*/ };

MFile* MFSOwner::File(String name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->services(name)) {
            return fs->create(name);
        }
    }
    return nullptr;
}

/********************************************************
 * MFileSystem implementations
 ********************************************************/

MFileSystem::MFileSystem(char* prefix)
{
    m_prefix = prefix;
}

bool MFileSystem::services(String name) 
{
    return name.startsWith(m_prefix);
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
