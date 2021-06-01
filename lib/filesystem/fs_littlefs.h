#ifndef MEATFILE_DEFINES_FSLITTLE_H
#define MEATFILE_DEFINES_FSLITTLE_H

#include "meat_io.h"
#include "../lib/littlefs/lfs.h"
#include "../make_unique.h"

/********************************************************
 * MFileSystem
 ********************************************************/

class LittleFileSystem: public MFileSystem 
{
    MFile* getFile(String path) override;
    bool mount() override;
    bool umount() override;


public:
    LittleFileSystem(char* prefix, uint32_t start, uint32_t size, uint32_t pageSize, uint32_t blockSize, uint32_t maxOpenFds)
        : MFileSystem(prefix), _start(start) , _size(size) , _pageSize(pageSize) , _blockSize(blockSize) , _maxOpenFds(maxOpenFds)
    {
        memset(&lfsStruct, 0, sizeof(lfsStruct));
        memset(&_lfs_cfg, 0, sizeof(_lfs_cfg));
        _lfs_cfg.context = (void*) this;
        _lfs_cfg.read = lfs_flash_read;
        _lfs_cfg.prog = lfs_flash_prog;
        _lfs_cfg.erase = lfs_flash_erase;
        _lfs_cfg.sync = lfs_flash_sync;
        _lfs_cfg.read_size = 64;
        _lfs_cfg.prog_size = 64;
        _lfs_cfg.block_size =  _blockSize;
        _lfs_cfg.block_count =_blockSize? _size / _blockSize: 0;
        _lfs_cfg.block_cycles = 16; // TODO - need better explanation
        _lfs_cfg.cache_size = 64;
        _lfs_cfg.lookahead_size = 64;
        _lfs_cfg.read_buffer = nullptr;
        _lfs_cfg.prog_buffer = nullptr;
        _lfs_cfg.lookahead_buffer = nullptr;
        _lfs_cfg.name_max = 0;
        _lfs_cfg.file_max = 0;
        _lfs_cfg.attr_max = 0;
        m_isMounted = false;
    }

    ~LittleFileSystem()
    {
        umount();
    }

    static lfs_t lfsStruct;

private:
    static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                              lfs_off_t off, void *buffer, lfs_size_t size);
    static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                              lfs_off_t off, const void *buffer, lfs_size_t size);
    static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block);
    static int lfs_flash_sync(const struct lfs_config *c);

    bool format();
    bool _tryMount();

    lfs_config  _lfs_cfg;
    uint32_t _start;
    uint32_t _size;
    uint32_t _pageSize;
    uint32_t _blockSize;
    uint32_t _maxOpenFds;
};






/********************************************************
 * MFile
 ********************************************************/

class LittleFile: public MFile
{
friend class LittleOStream;
friend class LittleIStream;

public:
    LittleFile(String path) : MFile(path) {
        if(!pathValid(path.c_str()))
            m_isNull = true;
    };

    bool isFile() override;
    bool isDirectory() override;
    MIstream* inputStream() override ; // has to return OPENED stream
    MOstream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    void setTimeCallback(time_t (*cb)(void)) override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool exists() override ;
    size_t size() override ;
    bool remove() override ;
    bool truncate(size_t size) override;
    bool rename(const char* dest);

private:
    lfs_dir_t dir;
    bool dirOpened = false;
    lfs_info _dirent;
    bool _valid;
    String _pattern;

    bool pathValid(const char *path);
    bool openDir(const char *path);
};


/********************************************************
 * LittleHandle
 ********************************************************/

class LittleHandle {
public:
    int rc;
    lfs_file_t lfsFile;

    LittleHandle() : rc(-255) 
    {
        memset(&lfsFile, 0, sizeof(lfsFile));
    };
    ~LittleHandle();
    void obtain(enum lfs_open_flags flags, String m_path);
    void dispose();

private:
    enum lfs_open_flags flags;
};

/********************************************************
 * MStreams O
 ********************************************************/

class LittleOStream: public MOstream {
public:
    // MStream methods
    LittleOStream(String& path) {
        m_path = path;
        handle = std::make_unique<LittleHandle>();
    }
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~LittleOStream() {
        close();
    }

    // MOstream methods
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    void flush() override;
    bool isOpen();

protected:
    String m_path;

    std::unique_ptr<LittleHandle> handle;    
};


/********************************************************
 * MStreams I
 ********************************************************/


class LittleIStream: public MIstream {
public:
    LittleIStream(String& path) {
        m_path = path;
        handle = std::make_unique<LittleHandle>();
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~LittleIStream() {
        close();
    }

    // MIstream methods
    int available() override;
    int read() override;
    int peek() override;
    size_t readBytes(char *buffer, size_t length) override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    String m_path;

    std::unique_ptr<LittleHandle> handle;    
};



#endif
