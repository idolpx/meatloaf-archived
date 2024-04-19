#ifndef MEATLOAF_STREAM
#define MEATLOAF_STREAM

#include <unordered_map>

/********************************************************
 * Universal streams
 ********************************************************/

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

class MStream 
{
protected:
    uint32_t _size = 0;
    uint32_t _position = 0;
    uint8_t _load_address[2] = {0, 0};
    uint8_t _error = 0;

public:
    virtual ~MStream() {};

    std::ios_base::openmode mode;
    std::string url = "";

    bool has_subdirs = true;
    size_t block_size = 256;

    virtual std::unordered_map<std::string, std::string> info() {
        return {};
    }

    virtual uint32_t size() {
        return _size;
    };

    virtual uint32_t available() {
        return _size - _position;
    };

    virtual uint32_t position() {
        return _position;
    }
    virtual void position( uint32_t p) {
        _position = p;
    }

    virtual size_t error() {
        return _error;
    }

    virtual bool eos()  {
//        Debug_printv("_size[%d] m_bytesAvailable[%d] _position[%d]", _size, available(), _position);
        if ( available() == 0 )
            return true;
        
        return false;
    }
    virtual void reset() 
    {
        _size = block_size;
        _position = 0;
    };
    
    virtual bool isOpen() = 0;
    virtual bool isBrowsable() { return false; };
    virtual bool isRandomAccess() { return false; };

    virtual void close() = 0;
    virtual bool open() = 0;

    virtual uint32_t write(const uint8_t *buf, uint32_t size) { return 0; }; // = 0;
    virtual uint32_t read(uint8_t* buf, uint32_t size) { return 0; }; // = 0;

    virtual bool seek(uint32_t pos, int mode) {
        if(mode == SEEK_SET) {
            return seek(pos);
        }
        else if(mode == SEEK_CUR) {
            if(pos == 0) return true;
            return seek(position()+pos);
        }
        else {
            return seek(size() - pos);
        }
    }
    virtual bool seek(uint32_t pos) { return true; }; // = 0;

    // For files with a browsable random access directory structure
    // d64, d74, d81, dnp, etc.
    virtual bool seekPath(std::string path) {
        return false;
    };

    // For files with no directory structure
    // tap, crt, tar
    virtual std::string seekNextEntry() {
        return "";
    };

    virtual bool seekBlock( uint64_t index, uint8_t offset = 0 ) { return false; };
    virtual bool seekSector( uint8_t track, uint8_t sector, uint8_t offset = 0 ) { return false; };
    virtual bool seekSector( std::vector<uint8_t> trackSectorOffset ) { return false; };
};

#endif // MEATLOAF_STREAM
