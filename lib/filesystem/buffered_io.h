#ifndef MEATFILE_BUFFERED_H
#define MEATFILE_BUFFERED_H

//#define RECORD_SIZE 256
#define BUFFER_SIZE 512

#include "meat_io.h"
#include "meat_stream.h"

/********************************************************
 * A buffer
 ********************************************************/

class MBuffer {
    bool handmade = false;

protected:
    int len = 0;
    char* buffer; // will point to buffered reader internal buffer

public:
    int length() {
        return len;
    }

    MBuffer() {};

    uint8_t getByte() {
        auto b = buffer[0];
        buffer++;
        len--;
        return b;
    }    

    // bool seek(uint32_t pos, SeekMode mode) {
    //     switch (mode) {
    //         case SeekSet:
    //             buffer = &buffer[0] + pos;
    //             break;

    //         case SeekCur:
    //             buffer = *&buffer + pos;
    //             break;
            
    //         case SeekEnd:
    //             buffer = &buffer[len] - pos;
    //             break;
    //     }
    // };
    // bool seek(uint32_t pos) {
    //     return seek(pos, SeekSet);
    // }

    char& operator [](int idx) {
        return buffer[idx];
    }

    MBuffer(int size) {
        handmade = true;
        buffer = new char[size];
    };

    ~MBuffer() {
        if(handmade)
            delete [] buffer;
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
    MIstream* istream = nullptr;
    std::function<int(uint8_t* buf, size_t size)> readFn;

    uint8_t rawBuffer[BUFFER_SIZE] = { 0 };

protected:
    MBuffer smartBuffer;
    bool eofOccured = false;

    void refillBuffer() {
        //secondHalf = !secondHalf;
        //uint8_t* window = (secondHalf) ? (uint8_t*)buffer : (uint8_t*)(buffer+RECORD_SIZE);

           // Serial.println("Refill buffer before read!");

        if(istream != nullptr) {
                //        Serial.println("read from stream!");

            smartBuffer.len = istream->read((uint8_t*)rawBuffer, BUFFER_SIZE);
        }
        else {
                  //      Serial.println("read from lambda!");
            smartBuffer.len = readFn((uint8_t*)rawBuffer, BUFFER_SIZE);
    ///        Serial.println("Read lambda launched ok!");
        }

        smartBuffer.buffer = (char *)rawBuffer;
        eofOccured = (smartBuffer.len == 0) || (istream != nullptr && istream->available() == 0);
       //             Serial.println("refillBuffer ok!");

    }

public:
    BufferedReader(MIstream* is) : istream(is) { 
    };

    BufferedReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : readFn(fn) {

    };

    MBuffer* read() {
        if(!eofOccured)
            refillBuffer();

        return &smartBuffer;
    }

    uint8_t readByte() {
        if(smartBuffer.length()==0 && !eofOccured)
            refillBuffer();

        return smartBuffer.getByte();
    }

    // uint8_t readByte() {
    //     if(!eofOccured)
    //         refillBuffer();

    //     uint32_t i = 1;
    //     uint8_t b = mbuffer[0];
    //     mbuffer.seek(i, SeekCur);
    //     return b;
    // }

    bool eof() {
        return eofOccured;
    }
};

/********************************************************
 * Buffered writer
 ********************************************************/

class BufferedWriter {
    MOstream* ostream;
    //bool secondHalf = false;

public:
    BufferedWriter(MOstream* os) : ostream(os) { 
    };
    int write(MBuffer* buffer) {
        return ostream->write((uint8_t*)buffer->buffer, buffer->length());
    }
};


#endif