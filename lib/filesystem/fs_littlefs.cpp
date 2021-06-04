#include "fs_littlefs.h"
#include "flash_hal.h"
#include "MIOException.h"

/********************************************************
 * MFileSystem implementations
 ********************************************************/

lfs_t LittleFileSystem::lfsStruct;

bool LittleFileSystem::handles(String path) 
{
    Serial.println("FSTEST: shecking if littlefs handles this");

    return true; // fallback fs, so it must be last on FS list
}

MFile* LittleFileSystem::getFile(String path)
{
    if(m_isMounted)
        return new LittleFile(path);
    else
        return nullptr;
        //throw IllegalStateException();
}

bool LittleFileSystem::mount()
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

bool LittleFileSystem::umount()
{
    if (m_isMounted) {
        lfs_unmount(&lfsStruct);
    }
    return true;
}

bool LittleFileSystem::_tryMount() {
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

bool LittleFileSystem::format() {
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

int LittleFileSystem::lfs_flash_read(const struct lfs_config *c,
    lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    LittleFileSystem *me = reinterpret_cast<LittleFileSystem*>(c->context); // nie wiem, czy ten reinterpret prawidlowo zadziala
    uint32_t addr = me->_start + (block * me->_blockSize) + off;
    return flash_hal_read(addr, size, static_cast<uint8_t*>(dst)) == FLASH_HAL_OK ? 0 : -1;
}

int LittleFileSystem::lfs_flash_prog(const struct lfs_config *c,
    lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    LittleFileSystem *me = reinterpret_cast<LittleFileSystem*>(c->context);
    uint32_t addr = me->_start + (block * me->_blockSize) + off;
    const uint8_t *src = reinterpret_cast<const uint8_t *>(buffer);
    return flash_hal_write(addr, size, static_cast<const uint8_t*>(src)) == FLASH_HAL_OK ? 0 : -1;
}

int LittleFileSystem::lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    LittleFileSystem *me = reinterpret_cast<LittleFileSystem*>(c->context);
    uint32_t addr = me->_start + (block * me->_blockSize);
    uint32_t size = me->_blockSize;
    return flash_hal_erase(addr, size) == FLASH_HAL_OK ? 0 : -1;
}

int LittleFileSystem::lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}



/********************************************************
 * MFile implementations
 ********************************************************/

bool LittleFile::pathValid(const char *path) 
{
    while (*path) {
        const char *slash = strchr(path, '/');
        if (!slash) {
            if (strlen(path) >= LFS_NAME_MAX) {
                // Terminal filename is too long
                return false;
            }
            break;
        }
        if ((slash - path) >= LFS_NAME_MAX) {
            // This subdir name too long
            return false;
        }
        path = slash + 1;
    }
    return true;
}


bool LittleFile::isFile()
{
    lfs_info info;
    int rc = lfs_stat(&LittleFileSystem::lfsStruct, m_path.c_str(), &info);
    return (rc == 0) && (info.type == LFS_TYPE_REG);
}

bool LittleFile::isDirectory()
{
    lfs_info info;
    int rc = lfs_stat(&LittleFileSystem::lfsStruct, m_path.c_str(), &info);
    return (rc == 0) && (info.type == LFS_TYPE_DIR);
}

MIstream* LittleFile::inputStream()
{
    MIstream* istream = new LittleIStream(m_path);
    istream->open();   
    return istream;
}

MOstream* LittleFile::outputStream()
{
               Serial.println("FSTEST: attempting LittleFile::outputStream 1");

    MOstream* ostream = new LittleOStream(m_path);
            Serial.println("FSTEST: attempting LittleFile::outputStream 2");

    ostream->open();   
    return ostream;
}

time_t LittleFile::getLastWrite()
{
    time_t ftime = 0;
    int rc = lfs_getattr(&LittleFileSystem::lfsStruct, m_path.c_str(), 't', (void *)&ftime, sizeof(ftime));
    if (rc != sizeof(ftime))
        ftime = 0; // Error, so clear read value
    return ftime;
}

time_t LittleFile::getCreationTime()
{
    
}

void LittleFile::setTimeCallback(time_t (*cb)(void))
{
    
}

bool LittleFile::mkDir()
{
    if (m_isNull) {
        return false;
    }
    int rc = lfs_mkdir(&LittleFileSystem::lfsStruct, m_path.c_str());
    return (rc==0);
}

bool LittleFile::exists()
{
    if (m_isNull) {
        return false;
    }
    lfs_info info;
    int rc = lfs_stat(&LittleFileSystem::lfsStruct, m_path.c_str(), &info);
    return rc == 0;
}

size_t LittleFile::size() {
    auto handle = std::make_unique<LittleHandle>();
    handle->obtain(LFS_O_RDONLY, m_path);
    size_t size = lfs_file_size(&LittleFileSystem::lfsStruct, &handle->lfsFile);
    return size;
}

bool LittleFile::remove() {
    // musi obslugiwac usuwanie plikow i katalogow!
    int rc = lfs_remove(&LittleFileSystem::lfsStruct, m_path.c_str());
    if (rc != 0) {
        DEBUGV("lfs_remove: rc=%d path=`%s`\n", rc, path);
        return false;
    }
    // Now try and remove any empty subdirs this makes, silently
    char *pathStr = strdup(m_path.c_str());
    if (pathStr) {
        char *ptr = strrchr(pathStr, '/');
        while (ptr) {
            *ptr = 0;
            lfs_remove(&LittleFileSystem::lfsStruct, pathStr); // Don't care if fails if there are files left
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
    return true;
}

bool LittleFile::truncate(size_t size) {
    auto handle = std::make_unique<LittleHandle>();
    handle->obtain(LFS_O_WRONLY, m_path);
    int rc = lfs_file_truncate(&LittleFileSystem::lfsStruct, &handle->lfsFile, size);
    if (rc < 0) {
        DEBUGV("lfs_file_truncate rc=%d\n", rc);
        return false;
    }
    return true;
}

bool LittleFile::rename(const char* pathTo) {
    if (!pathTo || !pathTo[0]) {
        return false;
    }
    int rc = lfs_rename(&LittleFileSystem::lfsStruct, m_path.c_str(), pathTo);
    if (rc != 0) {
        DEBUGV("lfs_rename: rc=%d, from=`%s`, to=`%s`\n", rc, pathFrom, pathTo);
        return false;
    }
    return true;
}

void LittleFile::openDir(const char *path) {
    if (!isDirectory()) { 
        dirOpened = false;
        return;
    }
    
    char *pathStr = strdup(path); // Allow edits on our scratch copy
    // Get rid of any trailing slashes
    while (strlen(pathStr) && (pathStr[strlen(pathStr)-1]=='/')) {
        pathStr[strlen(pathStr)-1] = 0;
    }
    // At this point we have a name of "blah/blah/blah" or "blah" or ""
    // If that references a directory, just open it and we're done.
    lfs_info info;
    //auto dir = std::make_shared<lfs_dir_t>();
    int rc;
    const char *filter = "";
    if (!pathStr[0]) {
        // openDir("") === openDir("/")
        rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, "/");
        filter = "";
    } else if (lfs_stat(&LittleFileSystem::lfsStruct, pathStr, &info) >= 0) {
        if (info.type == LFS_TYPE_DIR) {
            // Easy peasy, path specifies an existing dir!
            rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, pathStr);
	    filter = "";
        } else {
            // This is a file, so open the containing dir
            char *ptr = strrchr(pathStr, '/');
            if (!ptr) {
                // No slashes, open the root dir
                rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, "/");
		        filter = pathStr;
            } else {
                // We've got slashes, open the dir one up
                *ptr = 0; // Remove slash, truncate string
                rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, pathStr);
		        filter = ptr + 1;
            }
        }
    } else { 
        // Name doesn't exist, so use the parent dir of whatever was sent in
        // This is a file, so open the containing dir
        char *ptr = strrchr(pathStr, '/');
        if (!ptr) {
            // No slashes, open the root dir
            rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, "/");
	        filter = pathStr;
        } else {
            // We've got slashes, open the dir one up
            *ptr = 0; // Remove slash, truncate string
            rc = lfs_dir_open(&LittleFileSystem::lfsStruct, &dir, pathStr);
	        filter = ptr + 1;
        }
    }
    if (rc < 0) {
        DEBUGV("LittleFSImpl::openDir: path=`%s` err=%d\n", path, rc);
        free(pathStr);
        dirOpened = false;
    }
    else {
        // Skip the . and .. entries
        lfs_info dirent;
        lfs_dir_read(&LittleFileSystem::lfsStruct, &dir, &dirent);
        lfs_dir_read(&LittleFileSystem::lfsStruct, &dir, &dirent);

        //auto ret = std::make_shared<LittleFSDirImpl>(filter, this, dir, pathStr);
        free(pathStr);
        dirOpened = true;
    }
}

bool LittleFile::rewindDirectory()
{
    _valid = false;
    int rc = lfs_dir_rewind(&LittleFileSystem::lfsStruct, &dir);
    // Skip the . and .. entries
    lfs_info dirent;
    lfs_dir_read(&LittleFileSystem::lfsStruct, &dir, &dirent);
    lfs_dir_read(&LittleFileSystem::lfsStruct, &dir, &dirent);
    return (rc == 0);
}

MFile* LittleFile::getNextFileInDir()
{
    if(!dirOpened)
        openDir(m_path.c_str());

    memset(&_dirent, 0, sizeof(_dirent));

    const int n = _pattern.length();
    bool match;
    //do {
        _dirent.name[0] = 0;
        int rc = lfs_dir_read(&LittleFileSystem::lfsStruct, &dir, &_dirent);
        _valid = (rc == 1);

Serial.print("inside getNextFileInDir - got: ");
Serial.println(_dirent.name);


        //match = (!n || !strncmp((const char*) _dirent.name, _pattern.c_str(), n));
    //} while (_valid/* && !match*/);

    if(!_valid)
        return nullptr;
    else
        return new LittleFile(this->m_isNull+"/"+String(_dirent.name));
}





/********************************************************
 * MOStreams implementations
 ********************************************************/
// MStream methods
bool LittleOStream::isOpen() {
    return handle->rc >= 0;
}

bool LittleOStream::seek(uint32_t pos, SeekMode mode) {

};
bool LittleOStream::seek(uint32_t pos) {

};
size_t LittleOStream::position() {
    if(!isOpen()) return 0;
    else return lfs_file_tell(&LittleFileSystem::lfsStruct, &handle->lfsFile);
};

void LittleOStream::close() {
    if(isOpen()) {
        handle->dispose();
    }
};

bool LittleOStream::open() {
    if(!isOpen()) {
        handle->obtain(LFS_O_WRONLY | LFS_O_CREAT, m_path);
    }
    return isOpen();
};

// MOstream methods
size_t LittleOStream::write(uint8_t) {

};

size_t LittleOStream::write(const uint8_t *buf, size_t size) {
    if (!isOpen() || !buf) {
        return 0;
    }
    // procki w LittleFS sa nakladka na niskopoziomowe API z lfs.h
    // więc sa dokladnie na tym poziomie, co ten
    // musze sie odwolywac do funkcji lfs_*, a nie do funkcji z LittleFS.h

    // ponizszy fs jest inicjalizowany jako drugi arg LittleFSDirImpl
    //  i jest typu lfs_t

    int result = lfs_file_write(&LittleFileSystem::lfsStruct, &handle->lfsFile, (void*) buf, size);
    if (result < 0) {
        DEBUGV("lfs_write rc=%d\n", result);
    }
    return result;
};

void LittleOStream::flush() {

};


/********************************************************
 * MIStreams implementations
 ********************************************************/

bool LittleIStream::isOpen() {
    return handle->rc >= 0;
}

bool LittleIStream::seek(uint32_t pos, SeekMode mode) {

};
bool LittleIStream::seek(uint32_t pos) {

};

size_t LittleIStream::position() {
    if(!isOpen()) return 0;
    else return lfs_file_tell(&LittleFileSystem::lfsStruct, &handle->lfsFile);
};

void LittleIStream::close() {
    if(isOpen()) handle->dispose();
};

bool LittleIStream::open() {
    if(!isOpen()) {
        handle->obtain(LFS_O_RDONLY, m_path);
    }
    return isOpen();
};

// MIstream methods
int LittleIStream::available() {
    if(!isOpen()) return 0;
    return lfs_file_size(&LittleFileSystem::lfsStruct, &handle->lfsFile) - position();
};

int LittleIStream::read() {

};
int LittleIStream::peek() {

};
size_t LittleIStream::readBytes(char *buffer, size_t length) {

};
size_t LittleIStream::read(uint8_t* buf, size_t size) {
    if (!isOpen() | !buf) {
        return 0;
    }
    int result = lfs_file_read(&LittleFileSystem::lfsStruct, &handle->lfsFile, (void*) buf, size);
    if (result < 0) {
        DEBUGV("lfs_read rc=%d\n", result);
        return 0;
    }

    return result;
};


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
}

void LittleHandle::dispose() {
    if (rc >= 0) {
        lfs_file_close(&LittleFileSystem::lfsStruct, &lfsFile);
        DEBUGV("lfs_file_close: fd=%p\n", _getFD());
        // if (timeCallback && (flags & LFS_O_WRONLY)) {
        //     // If the file opened with O_CREAT, write the creation time attribute
        //     if (_creation) {
        //         // int lfs_setattr(lfs_t *lfsStruct, const char *path, uint8_t type, const void *buffer, lfs_size_t size);
        //         int rc = lfs_setattr(&LittleFileSystem::lfsStruct, m_path.c_str(), 'c', (const void *)&_creation, sizeof(_creation));
        //         if (rc < 0) {
        //             DEBUGV("Unable to set creation time on '%s' to %d\n", _name.get(), _creation);
        //         }
        //     }
        //     // Add metadata with last write time
        //     time_t now = timeCallback();
        //     int rc = lfs_setattr(&LittleFileSystem::lfsStruct, m_path.c_str(), 't', (const void *)&now, sizeof(now));
        //     if (rc < 0) {
        //         DEBUGV("Unable to set last write time on '%s' to %d\n", _name.get(), now);
        //     }
        // }
        rc = -255;
    }
}

void LittleHandle::obtain(int fl, String m_path) {
    flags = fl;

    if ((flags & LFS_O_CREAT) && strchr(m_path.c_str(), '/')) {
        // For file creation, silently make subdirs as needed.  If any fail,
        // it will be caught by the real file open later on
        char *pathStr = strdup(m_path.c_str());
        if (pathStr) {
            // Make dirs up to the final fnamepart
            char *ptr = strchr(pathStr, '/');
            while (ptr) {
                *ptr = 0;
                lfs_mkdir(&LittleFileSystem::lfsStruct, pathStr);
                *ptr = '/';
                ptr = strchr(ptr+1, '/');
            }
        }
        free(pathStr);
    }

    // time_t creation = 0;
    // // if (timeCallback && (flags & LFS_O_CREAT)) {
    //     // O_CREATE means we *may* make the file, but not if it already exists.
    //     // See if it exists, and only if not update the creation time
    //     int rc = lfs_file_open(&LittleFileSystem::lfsStruct, fd.get(), m_path.c_str(), LFS_O_RDONLY);

    // 	if (rc == 0) {
    //         lfs_file_close(&LittleFileSystem::lfsStruct, fd.get()); // It exists, don't update create time
    //     } else {
    //         creation = timeCallback();  // File didn't exist or otherwise, so we're going to create this time
    //     }
    // }

    rc = lfs_file_open(&LittleFileSystem::lfsStruct, &lfsFile, m_path.c_str(), flags);

    Serial.printf("FSTEST: lfs_file_open file rc:%d\n",rc);

    if (rc == LFS_ERR_ISDIR) {
        // To support the SD.openNextFile, a null FD indicates to the LittleFSFile this is just
        // a directory whose name we are carrying around but which cannot be read or written
    } else if (rc == 0) {
        lfs_file_sync(&LittleFileSystem::lfsStruct, &lfsFile);
    } else {
        DEBUGV("LittleFile::open: unknown return code rc=%d fd=%p path=`%s` openMode=%d accessMode=%d err=%d\n",
               rc, fd, path, openMode, accessMode, rc);
    }    
}

