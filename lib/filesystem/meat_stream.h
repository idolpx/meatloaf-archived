#ifndef MEATFILE_STREAMS_H
#define MEATFILE_STREAMS_H

#include <Arduino.h>
//#include "FS.h"

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
    virtual bool seek(uint32_t pos, SeekMode mode) {
        if(mode == SeekSet) {
            return seek(pos);
        }
        else if(mode == SeekCur) {
            return seek(position()+pos);
        }
        else {
            return seek(/*length-*/pos);
        }
    }
    virtual bool seek(uint32_t pos) = 0;
    virtual size_t position() = 0;
    virtual void close() = 0;
    virtual bool open() = 0;
    virtual ~MStream() {};
    virtual bool isOpen() = 0;

    bool isText = false;
};

// template <class T>
// class MFileStream: public MStream {
//     std::unique_ptr<T> file;

// protected:
//     T* getFile() {
//         return file.get();
//     }
// public:
//     MFileStream(std::shared_ptr<T> f): file(f) {};
// };


class MOStream: public MStream {
public:
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
};


class MIStream: public MStream {
public:
    virtual int available() = 0;
    virtual size_t size() = 0;
    virtual size_t read(uint8_t* buf, size_t size) = 0;
    std::string seekNextEntry() {
        return "";
    };

    virtual bool isBrowsable() { return false; };
    virtual bool isRandomAccess() { return false; };
};


#endif