#ifndef MEATFILE_DEFINES_FSLITTLE_H
#define MEATFILE_DEFINES_FSLITTLE_H

#include "meat_io.h"
#if defined(ESP8266)
#include "../lib/littlefs/lfs.h"
#elif defined(ESP32)
#include "lfs.h"
#endif
#include "../../include/global_defines.h"
#include "../../include/make_unique.h"
//#include "EdUrlParser.h"

#include <string.h>


/********************************************************
 * MFileSystem
 ********************************************************/

class LittleFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override;
    bool mount() override;
    bool umount() override;


public:
    LittleFileSystem(uint32_t start, uint32_t size, uint32_t pageSize, uint32_t blockSize, uint32_t maxOpenFds)
        : MFileSystem("littleFS"), _start(start) , _size(size) , _pageSize(pageSize) , _blockSize(blockSize) , _maxOpenFds(maxOpenFds)
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
        mount();
    }

    bool handles(std::string path);

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
    LittleFile(std::string path) {
        parseUrl(path);
        if(!pathValid(path.c_str()))
            m_isNull = true;
        else
            m_isNull = false;
    };
    ~LittleFile() {
        //Serial.printf("*** Destroying littlefile %s\n", url.c_str());
        closeDir();
    }

    //MFile* cd(std::string newDir);
    bool isDirectory() override;
    MIStream* inputStream() override ; // has to return OPENED stream
    MOStream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool exists() override ;
    size_t size() override ;
    bool remove() override ;
    bool rename(std::string dest);
    MIStream* createIStream(std::shared_ptr<MIStream> src);

private:
    void openDir(std::string path);
    void closeDir();
    lfs_dir_t dir;
    bool dirOpened = false;
    bool _valid;
    std::string _pattern;

    bool pathValid(std::string path);

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
        //Serial.println("*** Creating little handle");
        memset(&lfsFile, 0, sizeof(lfsFile));
    };
    ~LittleHandle();
    void obtain(int flags, std::string localPath);
    void dispose();

private:
    int flags;
};

/********************************************************
 * MStreams O
 ********************************************************/

class LittleOStream: public MOStream {
public:
    // MStream methods
    LittleOStream(std::string& path) {
        localPath = path;
        handle = std::make_unique<LittleHandle>();
    }
    size_t position() override;
    void close() override;
    bool open() override;
    ~LittleOStream() override {
        close();
    }

    // MOStream methods
    //size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    bool isOpen();

protected:
    std::string localPath;

    std::unique_ptr<LittleHandle> handle;    
};


/********************************************************
 * MStreams I
 ********************************************************/


class LittleIStream: public MIStream {
public:
    LittleIStream(std::string& path) {
        localPath = path;
        handle = std::make_unique<LittleHandle>();
    }
    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~LittleIStream() override {
        close();
    }

    // MIStream methods
    size_t available() override;
    size_t size() override;
    //uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();
    virtual bool seek(size_t pos) override;
    virtual bool seek(size_t pos, SeekMode mode) override;

protected:
    std::string localPath;

    std::unique_ptr<LittleHandle> handle;    
};



#endif
