#ifndef MEATFILE_STREAMS_H
#define MEATFILE_STREAMS_H

//#include "../../include/global_defines.h"

/********************************************************
 * Universal streams
 ********************************************************/

// enum SeekMode {
//     SeekSet = 0,
//     SeekCur = 1,
//     SeekEnd = 2
// };

class MStream {
public:
    virtual size_t position() = 0;
    virtual void close() = 0;
    virtual bool open() = 0;
    virtual ~MStream() {};
    virtual bool isOpen() = 0;

    bool isText = false;
};

class MOStream: public MStream {
public:
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
};


class MIStream: public MStream {
public:
    virtual bool seek(size_t pos, SeekMode mode) {
        if(mode == SeekSet) {
            return seek(pos);
        }
        else if(mode == SeekCur) {
            return seek(position()+pos);
        }
        else {
            return seek(size() - pos);
        }
    }
    virtual bool seek(size_t pos) = 0;

    virtual size_t available() = 0;
    virtual size_t size() = 0;
    virtual size_t read(uint8_t* buf, size_t size) = 0;

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

    virtual bool isBrowsable() { return false; };
    virtual bool isRandomAccess() { return false; };
};


#endif