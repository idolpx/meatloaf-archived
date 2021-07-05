#ifndef MEATFILE_BUFFERED_H
#define MEATFILE_BUFFERED_H

//#define RECORD_SIZE 256
#define BUFFER_SIZE 512

#include "../meat_io.h"
#include "meat_stream.h"
#include "../../include/global_defines.h"

/********************************************************
 * A buffer
 ********************************************************/

class MBuffer {
    bool handmade = false;

protected:
    int len = 0;
    char* buffer; // will point to buffered reader internal buffer

public:
    MBuffer() {};

    MBuffer(char* start, size_t size) {
        buffer = start;
        len = size;
    }

    MBuffer(int size) {
        handmade = true;
        buffer = new char[size];
    };

    ~MBuffer() {
        if(handmade)
            delete [] buffer;
    }

    char& operator [](int idx) {
        return buffer[idx];
    }

    uint8_t getByte() {
        auto b = buffer[0];
        buffer++;
        len--;
        return b;
    }    

    int length() {
        return len;
    }

    char* getBuffer() {
        return buffer;
    }

    friend class BufferedReader;
    friend class BufferedWriter;
};

class MBufferConst: public MBuffer {
protected:
    const char* buffer; // will point to buffered reader internal buffer

public:
    MBufferConst(std::string str): buffer(str.c_str()) {
        len = str.length();
    };
};

/********************************************************
 * Buffered reader
 ********************************************************/

class BufferedReader {
    MIStream* istream = nullptr;
    std::function<int(uint8_t* buf, size_t size)> readFn;

    uint8_t rawBuffer[BUFFER_SIZE] = { 0 };

protected:
    MBuffer smartBuffer;
    bool eofOccured = false;
    size_t m_available = 0;

    void refillBuffer() {
        if(istream != nullptr) {
            smartBuffer.len = istream->read((uint8_t*)rawBuffer, BUFFER_SIZE);
            Debug_printv("Refilling buffer by stream, got %d bytes!", smartBuffer.len);

        }
        else {
            smartBuffer.len = readFn((uint8_t*)rawBuffer, BUFFER_SIZE);
            Debug_printv("Refilling buffer by lambda, got %d bytes!", smartBuffer.len);
        }

        smartBuffer.buffer = (char *)rawBuffer;
        eofOccured = (smartBuffer.len == 0) || (istream != nullptr && istream->available() == 0);
    }

public:
    BufferedReader(MIStream* is) : istream(is), m_available(is->available()) { 
    };

    BufferedReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : readFn(fn) {

    };

    MBuffer* read() {
        if(!eofOccured)
            refillBuffer();

        m_available-=smartBuffer.length();

        return &smartBuffer;
    }

    uint8_t readByte() {
        if(smartBuffer.length()==0 && !eofOccured)
            refillBuffer();

        m_available--;

        return smartBuffer.getByte();
    }

    bool eof() {
        return eofOccured;
    }

    size_t available() {
        return m_available;
    }
};

/********************************************************
 * Buffered writer
 ********************************************************/

class BufferedWriter {
    MOStream* ostream;

public:
    BufferedWriter(MOStream* os) : ostream(os) { 
    };

    int write(MBuffer* buffer) {
        return ostream->write((uint8_t*)buffer->buffer, buffer->length());
    }
    
    bool writeByte(uint8_t byteToWrite) 
    {
                    //Debug_printv("in wrapper overflow '%c'", ch);

        ostream->write(&byteToWrite, 1);
        return true;
    }
};

#endif