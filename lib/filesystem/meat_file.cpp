#include "meat_file.h"
#include "fs_smb.h"
#include "fs_d64.h"

MFileSystem* MFSOwner::availableFS[FS_COUNT] = { /*new D64FileSystem(""), new SMBFileSystem("smb:")*/ };

MFileSystem::MFileSystem(char* prefix)
{
    m_prefix = prefix;
}

bool MFileSystem::services(String name) 
{
    return name.startsWith(m_prefix);
}

MFile MFSOwner::File(String name) {
    uint i = 0;
    for(auto fs = availableFS[i]; i < FS_COUNT ; i ++) {
        if(fs->services(name)) {
            return fs->create(name);
        }
    }
    return nullptr;
}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}
