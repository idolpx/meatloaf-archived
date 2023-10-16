#include "flash.h"
#include "flash_hal.h"
#include "MIOException.h"
/********************************************************
 * MFileSystem implementations
 ********************************************************/

lfs_t FlashFileSystem::lfsStruct;

bool FlashFileSystem::handles(std::string apath) 
{
    return true; // fallback fs, so it must be last on FS list
}

MFile* FlashFileSystem::getFile(std::string apath)
{
    if(m_isMounted)
        return new LittleFile(apath);
    else
        return nullptr;
        //throw IllegalStateException();
}

bool FlashFileSystem::mount()
{
    if (m_isMounted)
        return true;

    if (_size <= 0) {
        DEBUGV("LittleFS size is <= zero");
        return false;
    }

    if (_tryMount()) {
        return true;
    }

    if (/*!_cfg._autoFormat ||*/ !format()) {
        return false;
    }

    return _tryMount();
}

bool FlashFileSystem::umount()
{
    if (m_isMounted) {
        lfs_unmount(&lfsStruct);
    }
    return true;
}

bool FlashFileSystem::_tryMount() {
    if (m_isMounted) {
        lfs_unmount(&lfsStruct);
        m_isMounted = false;
    }
    memset(&lfsStruct, 0, sizeof(lfsStruct));
    int rc = lfs_mount(&lfsStruct, &_lfs_cfg);
    if (rc==0) {
        m_isMounted = true;
    }
    return m_isMounted;
}

bool FlashFileSystem::format() {
    if (_size == 0) {
        DEBUGV("lfs size is zero\n");
        return false;
    }

    bool wasMounted = m_isMounted;
    if (m_isMounted) {
        lfs_unmount(&lfsStruct);
        m_isMounted = false;
    }

    memset(&lfsStruct, 0, sizeof(lfsStruct));
    int rc = lfs_format(&lfsStruct, &_lfs_cfg);
    if (rc != 0) {
        DEBUGV("lfs_format: rc=%d\n", rc);
        return false;
    }

    if (wasMounted) {
        return _tryMount();
    }

    return true;
}

int FlashFileSystem::lfs_flash_read(const struct lfs_config *c,
    lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    FlashFileSystem *me = reinterpret_cast<FlashFileSystem*>(c->context); // nie wiem, czy ten reinterpret prawidlowo zadziala
    uint32_t addr = me->_start + (block * me->_blockSize) + off;
    return flash_hal_read(addr, size, static_cast<uint8_t*>(dst)) == FLASH_HAL_OK ? 0 : -1;
}

int FlashFileSystem::lfs_flash_prog(const struct lfs_config *c,
    lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    FlashFileSystem *me = reinterpret_cast<FlashFileSystem*>(c->context);
    uint32_t addr = me->_start + (block * me->_blockSize) + off;
    const uint8_t *src = reinterpret_cast<const uint8_t *>(buffer);
    return flash_hal_write(addr, size, static_cast<const uint8_t*>(src)) == FLASH_HAL_OK ? 0 : -1;
}

int FlashFileSystem::lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    FlashFileSystem *me = reinterpret_cast<FlashFileSystem*>(c->context);
    uint32_t addr = me->_start + (block * me->_blockSize);
    uint32_t size = me->_blockSize;
    return flash_hal_erase(addr, size) == FLASH_HAL_OK ? 0 : -1;
}

int FlashFileSystem::lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}



/********************************************************
 * MFile implementations
 ********************************************************/

// MFile* LittleFile::cd(std::string newDir) {
//     if(newDir[0]=='/' && newDir[1]=='/') {
//         if(newDir.size()==2) {
//             // user entered: CD:// or CD//
//             // means: change to the root of roots
//             return MFSOwner::File("/"); // chedked, works ad flash root!
//         }
//         else {
//             // user entered: CD://DIR or CD//DIR
//             // means: change to a dir in root of roots
//             return root(mstr::drop(newDir,2));
//         }
//     }
//     else if(newDir[0]=='/') {
//         if(newDir.size()==1) {
//             // user entered: CD:/ or CD/
//             // means: change to container root
//             // *** might require a fix for flash fs!
//             return MFSOwner::File("/");
//         }
//         else {
//             // user entered: CD:/DIR or CD/DIR
//             // means: change to a dir in container root
//             return MFSOwner::File("/"+newDir);
//         }
//     }
//     else
//         return MFile::cd(newDir);
// };


bool LittleFile::pathValid(std::string path) 
{
    auto apath = path.c_str();
    while (*apath) {
        const char *slash = strchr(apath, '/');
        if (!slash) {
            if (strlen(apath) >= LFS_NAME_MAX) {
                // Terminal filename is too long
                return false;
            }
            break;
        }
        if ((slash - apath) >= LFS_NAME_MAX) {
            // This subdir name too long
            return false;
        }
        apath = slash + 1;
    }
    return true;
}

bool LittleFile::isDirectory()
{
    if(path=="/" || path=="")
        return true;

    lfs_info info;
    int rc = lfs_stat(&FlashFileSystem::lfsStruct, path.c_str(), &info);
    return (rc == 0) && (info.type == LFS_TYPE_DIR);
}

MStream* LittleFile::createIStream(std::shared_ptr<MStream> is) {
    return is.get(); // we don't have to process this stream in any way, just return the original stream
}


time_t LittleFile::getLastWrite()
{
    time_t ftime = 0;
    int rc = lfs_getattr(&FlashFileSystem::lfsStruct, path.c_str(), 't', (void *)&ftime, sizeof(ftime));
    if (rc != sizeof(ftime))
        ftime = 0; // Error, so clear read value
    return ftime;
}

time_t LittleFile::getCreationTime()
{
    return 0;
}

bool LittleFile::mkDir()
{
    if (m_isNull) {
        return false;
    }
    int rc = lfs_mkdir(&FlashFileSystem::lfsStruct, path.c_str());
    return (rc==0);
}

bool LittleFile::exists()
{
    if (m_isNull) {
        return false;
    }
    if (path=="/" || path=="") {
        return true;
    }
    lfs_info info;
    int rc = lfs_stat(&FlashFileSystem::lfsStruct, path.c_str(), &info);
    return rc == 0;
}

size_t LittleFile::size() {
    if(m_isNull || path=="/" || path=="")
        return 0;
    else if(isDirectory()) {
        return 0;
    }
    else {
        auto handle = std::make_unique<LittleHandle>();
        handle->obtain(LFS_O_RDONLY, path);
        size_t size = lfs_file_size(&FlashFileSystem::lfsStruct, &handle->lfsFile);
        return size;
    }
}

bool LittleFile::remove() {
    // musi obslugiwac usuwanie plikow i katalogow!
    if(path.empty())
        return false;

    int rc = lfs_remove(&FlashFileSystem::lfsStruct, path.c_str());
    if (rc != 0) {
        DEBUGV("lfs_remove: rc=%d path=`%s`\n", rc, path);
        return false;
    }
    // Now try and remove any empty subdirs this makes, silently
    char *pathStr = new char[path.length()];
    strncpy(pathStr, path.data(), path.length());

    char *ptr = strrchr(pathStr, '/');
    while (ptr) {
        *ptr = 0;
        lfs_remove(&FlashFileSystem::lfsStruct, pathStr); // Don't care if fails if there are files left
        ptr = strrchr(pathStr, '/');
    }
    delete[] pathStr;

    return true;
}

// bool LittleFile::truncate(size_t size) {
//     auto handle = std::make_unique<LittleHandle>();
//     handle->obtain(LFS_O_WRONLY, path);
//     int rc = lfs_file_truncate(&FlashFileSystem::lfsStruct, &handle->lfsFile, size);
//     if (rc < 0) {
//         DEBUGV("lfs_file_truncate rc=%d\n", rc);
//         return false;
//     }
//     return true;
// }

bool LittleFile::rename(std::string pathTo) {
    if(pathTo.empty())
        return false;

    int rc = lfs_rename(&FlashFileSystem::lfsStruct, path.c_str(), pathTo.c_str());
    if (rc != 0) {
        return false;
    }
    return true;
}



void LittleFile::closeDir() {
    if(dirOpened) {
        dirOpened = false;
        lfs_dir_close(&FlashFileSystem::lfsStruct, &dir);
    }
}

void LittleFile::openDir(std::string apath) {
    if (!isDirectory()) { 
        dirOpened = false;
        return;
    }

    lfs_info info;
    int rc = -1;

    if(apath.empty()) {
        rc = lfs_dir_open(&FlashFileSystem::lfsStruct, &dir, "/");
    }
    else if (lfs_stat(&FlashFileSystem::lfsStruct, apath.c_str(), &info) >= 0) {
        rc = lfs_dir_open(&FlashFileSystem::lfsStruct, &dir, apath.c_str());
    }
    if (rc < 0) {
        dirOpened = false;
    }
    else {
        // Skip the . and .. entries
        lfs_info dirent;
        lfs_dir_read(&FlashFileSystem::lfsStruct, &dir, &dirent);
        lfs_dir_read(&FlashFileSystem::lfsStruct, &dir, &dirent);

        dirOpened = true;
    }
}

bool LittleFile::rewindDirectory()
{
    _valid = false;
    int rc = lfs_dir_rewind(&FlashFileSystem::lfsStruct, &dir);
    // Skip the . and .. entries
    lfs_info dirent;
    lfs_dir_read(&FlashFileSystem::lfsStruct, &dir, &dirent);
    lfs_dir_read(&FlashFileSystem::lfsStruct, &dir, &dirent);

    media_blocks_free = 0;
    return (rc == 0);
}

MFile* LittleFile::getNextFileInDir()
{
    lfs_info _dirent;

    if(!dirOpened)
        openDir(path.c_str());

    memset(&_dirent, 0, sizeof(_dirent));
    _dirent.name[0] = 0;

    if(lfs_dir_read(&FlashFileSystem::lfsStruct, &dir, &_dirent) != 1) {
        closeDir();
        return nullptr;
    }
    else
        return new LittleFile(this->path + ((this->path == "/") ? "" : "/") + std::string(_dirent.name)); // due to EdUrlParser shittiness
}





/********************************************************
 * MStreams implementations
 ********************************************************/
// MStream methods
// error list: enum lfs_error
bool LittleStream::isOpen() {
    return handle->rc >= 0;
}

size_t LittleStream::position() {
    if(!isOpen()) return 0;
    else return lfs_file_tell(&FlashFileSystem::lfsStruct, &handle->lfsFile);
};

void LittleStream::close() {
    if(isOpen()) {
        handle->dispose();
    }
};

bool LittleStream::open() {
    if(!isOpen()) {
        handle->obtain(LFS_O_WRONLY | LFS_O_CREAT, localPath);
        //handle->obtain(LFS_O_RDONLY, localPath);
    }
    return isOpen();
};

size_t LittleStream::write(const uint8_t *buf, size_t size) {
    if (!isOpen() || !buf) {
        return 0;
    }
    // procki w LittleFS sa nakladka na niskopoziomowe API z lfs.h
    // więc sa dokladnie na tym poziomie, co ten
    // musze sie odwolywac do funkcji lfs_*, a nie do funkcji z LittleFS.h

    // ponizszy fs jest inicjalizowany jako drugi arg LittleFSDirImpl
    //  i jest typu lfs_t

    //Serial.println("before lfs_file_write");

    int result = lfs_file_write(&FlashFileSystem::lfsStruct, &handle->lfsFile, (void*) buf, size);

    //Serial.printf("in byteWrite '%c'\n", buf[0]);
    //Serial.println("after lfs_file_write");

    if (result < 0) {
        DEBUGV("lfs_write rc=%d\n", result);
    }
    return result;
};

// MStream methods
size_t LittleStream::available() {
    if(!isOpen()) return 0;
    return lfs_file_size(&FlashFileSystem::lfsStruct, &handle->lfsFile) - position();
};

size_t LittleStream::size() {
    return available();
};

// uint8_t LittleStream::read() {
//     return 0;
// };

size_t LittleStream::read(uint8_t* buf, size_t size) {
    if (!isOpen() || !buf) {
        Debug_printv("Not open");
        return 0;
    }
    
    int result = lfs_file_read(&FlashFileSystem::lfsStruct, &handle->lfsFile, (void*) buf, size);
    if (result < 0) {
        DEBUGV("lfs_read rc=%d\n", result);
        return 0;
    }

    return result;
};

bool LittleStream::seek(uint32_t pos) {
    // Debug_printv("pos[%d]", pos);
    return seek(pos, SEEK_SET);
};

bool LittleStream::seek(uint32_t pos, int mode) {
    // Debug_printv("pos[%d] mode[%d]", pos, mode);
    if (!isOpen()) {
        Debug_printv("Not open");
        return false;
    }
    return (lfs_file_seek(&FlashFileSystem::lfsStruct, &handle->lfsFile, pos, mode))? true: false;
}


/********************************************************
 * LittleHandle implementations
 ********************************************************/

/*
lfs_open_flags

    LFS_O_RDONLY = 1,         // Open a file as read only
    LFS_O_WRONLY = 2,         // Open a file as write only
    LFS_O_RDWR   = 3,         // Open a file as read and write
    LFS_O_CREAT  = 0x0100,    // Create a file if it does not exist
    LFS_O_EXCL   = 0x0200,    // Fail if a file already exists
    LFS_O_TRUNC  = 0x0400,    // Truncate the existing file to zero size
    LFS_O_APPEND = 0x0800,    // Move to end of file on every write

z lfs.h
*/

LittleHandle::~LittleHandle() {
    dispose();
    //Serial.printf("*** deleting littlehandle for \n");
}

void LittleHandle::dispose() {
    if (rc >= 0) {
        //Serial.println("*** closing little handle");

        lfs_file_close(&FlashFileSystem::lfsStruct, &lfsFile);
        DEBUGV("lfs_file_close: fd=%p\n", _getFD());
        // if (timeCallback && (flags & LFS_O_WRONLY)) {
        //     // If the file opened with O_CREAT, write the creation time attribute
        //     if (_creation) {
        //         // int lfs_setattr(lfs_t *lfsStruct, const char *loclaPath, uint8_t type, const void *buffer, lfs_size_t size);
        //         int rc = lfs_setattr(&FlashFileSystem::lfsStruct, loclaPath.c_str(), 'c', (const void *)&_creation, sizeof(_creation));
        //         if (rc < 0) {
        //             DEBUGV("Unable to set creation time on '%s' to %d\n", _name.get(), _creation);
        //         }
        //     }
        //     // Add metadata with last write time
        //     time_t now = timeCallback();
        //     int rc = lfs_setattr(&FlashFileSystem::lfsStruct, loclaPath.c_str(), 't', (const void *)&now, sizeof(now));
        //     if (rc < 0) {
        //         DEBUGV("Unable to set last write time on '%s' to %d\n", _name.get(), now);
        //     }
        // }
        rc = -255;
    }
}

void LittleHandle::obtain(int fl, std::string m_path) {
    flags = fl;

    //Serial.printf("*** Atempting opening littlefs  handle'%s'\n", m_path.c_str());

    if ((flags & LFS_O_CREAT) && strchr(m_path.c_str(), '/')) {
        // For file creation, silently make subdirs as needed.  If any fail,
        // it will be caught by the real file open later on

        char *pathStr = new char[m_path.length()];
        strncpy(pathStr, m_path.data(), m_path.length());

        if (pathStr) {
            // Make dirs up to the final fnamepart
            char *ptr = strchr(pathStr, '/');
            while (ptr) {
                *ptr = 0;
                lfs_mkdir(&FlashFileSystem::lfsStruct, pathStr);
                *ptr = '/';
                ptr = strchr(ptr+1, '/');
            }
        }
        delete[] pathStr;
    }

    // time_t creation = 0;
    // // if (timeCallback && (flags & LFS_O_CREAT)) {
    //     // O_CREATE means we *may* make the file, but not if it already exists.
    //     // See if it exists, and only if not update the creation time
    //     int rc = lfs_file_open(&FlashFileSystem::lfsStruct, fd.get(), loclaPath.c_str(), LFS_O_RDONLY);

    // 	if (rc == 0) {
    //         lfs_file_close(&FlashFileSystem::lfsStruct, fd.get()); // It exists, don't update create time
    //     } else {
    //         creation = timeCallback();  // File didn't exist or otherwise, so we're going to create this time
    //     }
    // }

    rc = lfs_file_open(&FlashFileSystem::lfsStruct, &lfsFile, m_path.c_str(), flags);

    //Serial.printf("FSTEST: lfs_file_open file rc:%d\n",rc);

    if (rc == LFS_ERR_ISDIR) {
        // To support the SD.openNextFile, a null FD indicates to the LittleFSFile this is just
        // a directory whose name we are carrying around but which cannot be read or written
    } else if (rc == 0) {
        lfs_file_sync(&FlashFileSystem::lfsStruct, &lfsFile);
    } else {
        DEBUGV("LittleFile::open: unknown return code rc=%d fd=%p path=`%s` openMode=%d accessMode=%d err=%d\n",
               rc, fd, localPath, openMode, accessMode, rc);
    }    
}
