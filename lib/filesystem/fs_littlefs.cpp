#include "fs_littlefs.h"
#include "flash_hal.h"

/********************************************************
 * MFileSystem implementations
 ********************************************************/

lfs_t LittleFileSystem::lfs;


MFile* LittleFileSystem::create(String path)
{
    return new LittleFile(path); // dodamy globalne struktury littlefs
}

bool LittleFileSystem::mount()
{
    // tresc tej funkcji jest w begin
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
    if (_mounted) {
        lfs_unmount(&lfs);
    }
    return true;
}

bool LittleFileSystem::_tryMount() {
        if (_mounted) {
            lfs_unmount(&lfs);
            _mounted = false;
        }
        memset(&lfs, 0, sizeof(lfs));
        int rc = lfs_mount(&lfs, &_lfs_cfg);
        if (rc==0) {
            _mounted = true;
        }
        return _mounted;
}

bool LittleFileSystem::format() {
        if (_size == 0) {
            DEBUGV("lfs size is zero\n");
            return false;
        }

        bool wasMounted = _mounted;
        if (_mounted) {
            lfs_unmount(&lfs);
            _mounted = false;
        }

        memset(&lfs, 0, sizeof(lfs));
        int rc = lfs_format(&lfs, &_lfs_cfg);
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

bool LittleFile::isFile()
{
    lfs_info info;
    // _fs->getFS() zastępujemy &_lfs
    int rc = lfs_stat(&LittleFileSystem::lfs, m_path.c_str(), &info);
    return (rc == 0) && (info.type == LFS_TYPE_REG);
}

bool LittleFile::isDirectory()
{
    lfs_info info;
    // _fs->getFS() zastępujemy &_lfs
    int rc = lfs_stat(&LittleFileSystem::lfs, m_path.c_str(), &info);
    return (rc == 0) && (info.type == LFS_TYPE_DIR);
}

MIstream* LittleFile::inputStream()
{
    // return OPENED stream
    MIstream* istream = new LittleIStream(m_path);
    istream->open();   
    return istream;
}

MOstream* LittleFile::outputStream()
{
    // return OPENED stream
    MOstream* ostream = new LittleOStream(m_path);
    ostream->open();   
    return ostream;
}

time_t LittleFile::getLastWrite()
{
    time_t ftime = 0;
        int rc = lfs_getattr(&LittleFileSystem::lfs, m_path.c_str(), 't', (void *)&ftime, sizeof(ftime));
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

bool LittleFile::rewindDirectory()
{
    
}

MFile* LittleFile::getNextFileInDir()
{
    // TODO!   
}

bool LittleFile::mkDir()
{
    // TODO!   
}

bool LittleFile::mkDirs()
{
    
}

bool LittleFile::exists()
{
    // TODO!
}

size_t LittleFile::size() {
    // TODO!
}

bool LittleFile::remove() {
    // TODO!
    // musi obslugiwac usuwanie plikow i katalogow!
}

bool LittleFile::truncate(size_t size) {
    // TODO!
}

bool LittleFile::rename(const char* dest) {
    // TODO!
}

/********************************************************
 * MStreams implementations
 ********************************************************/
    // MStream methods
    bool LittleOStream::seek(uint32_t pos, SeekMode mode) {

    };
    bool LittleOStream::seek(uint32_t pos) {

    };
    size_t LittleOStream::position() const {

    };
    void LittleOStream::close() {
        if(isOpen()) {

        }
    };
    bool LittleOStream::open() {
        // remember to set m_isOpen if succesful
        // kod można ściągnąć z LittleFSImpl::open
        // tamże otrzymamy file handle, którym wypełnimy lfsFile
        lfsFile = std::make_shared<lfs_file_t>();
        
        m_isOpen = true;
        return true;
    };
    // LittleOStream::~LittleOStream(

    // );

    // MOstream methods
    size_t LittleOStream::write(uint8_t) {

    };
    size_t LittleOStream::write(const uint8_t *buf, size_t size) {
        if (!isOpen() || !getLfsFileHandle() || !buf) {
            return 0;
        }
        // procki w LittleFS sa nakladka na niskopoziomowe API z lfs.h
        // więc sa dokladnie na tym poziomie, co ten
        // musze sie odwolywac do funkcji lfs_*, a nie do funkcji z LittleFS.h

        // ponizszy fs jest inicjalizowany jako drugi arg LittleFSDirImpl
        //  i jest typu lfs_t

        int result = lfs_file_write(&LittleFileSystem::lfs, getLfsFileHandle(), (void*) buf, size);
        if (result < 0) {
            DEBUGV("lfs_write rc=%d\n", result);
            return 0;
        }
        return result;
    };
    void LittleOStream::flush() {

    };

    // MStream methods
    bool LittleIStream::seek(uint32_t pos, SeekMode mode) {

    };
    bool LittleIStream::seek(uint32_t pos) {

    };
    size_t LittleIStream::position() const {

    };
    void LittleIStream::close() {
        if(isOpen()) {
            
        }
    };
    bool LittleIStream::open() {
        // remember to set m_isOpen if succesful

        // remember to set m_isOpen if succesful
        // kod można ściągnąć z LittleFSImpl::open
        // tamże otrzymamy file handle, którym wypełnimy lfsFile
        lfsFile = std::make_shared<lfs_file_t>();

        m_isOpen = true;
        return true;
    };
    // LittleIStream::~LittleIStream(

    // );

    // MIstream methods
    int LittleIStream::available() {
        // TODO!
    };
    int LittleIStream::read() {

    };
    int LittleIStream::peek() {

    };
    size_t LittleIStream::readBytes(char *buffer, size_t length) {

    };
    size_t LittleIStream::read(uint8_t* buf, size_t size) {
        // TODO!
    };
