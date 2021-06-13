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
    int len = 0;

protected:
    char* buffer; // will point to buffered reader internal buffer

public:
    int length() {
        return len;
    }

    MBuffer() {};

    char operator [](int idx) {
        return buffer[idx];
    }

    // MBuffer(int size) {
    //     buffer
    // };

    friend class BufferedReader;
    friend class BufferedWriter;
};

class MBufferConst: public MBuffer {
protected:
    const char* buffer; // will point to buffered reader internal buffer

public:
    MBufferConst(std::string str): buffer(str.c_str()) {

    };
};

/********************************************************
 * Buffered reader
 ********************************************************/

class BufferedReader {
    MIstream* istream = nullptr;
    std::function<int(uint8_t* buf, size_t size)> readFn;

    //bool secondHalf = false;

    #if defined(ESP32)
    uint8_t buffer[BUFFER_SIZE] = { 0 };
    #else //defined(ESP8266)
    char buffer[BUFFER_SIZE] = { 0 };
    #endif
protected:
    MBuffer mbuffer;
    bool eofOccured = false;

    void refillBuffer() {
        //secondHalf = !secondHalf;
        //uint8_t* window = (secondHalf) ? (uint8_t*)buffer : (uint8_t*)(buffer+RECORD_SIZE);

           // Serial.println("Refill buffer before read!");

        if(istream != nullptr) {
                //        Serial.println("read from stream!");

            mbuffer.len = istream->read((uint8_t*)buffer, BUFFER_SIZE);
        }
        else {
                  //      Serial.println("read from lambda!");
            mbuffer.len = readFn((uint8_t*)buffer, BUFFER_SIZE);
    ///        Serial.println("Read lambda launched ok!");
        }

        mbuffer.buffer = (char *)buffer;
        eofOccured = (mbuffer.len == 0) || (istream != nullptr && istream->available() == 0);
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

        return &mbuffer;
    }
    bool eof() {
        return eofOccured;
    }
};

/********************************************************
 * Buffered writer
 ********************************************************/

class BufferedWriter {
    MOstream* ostream;
    bool secondHalf = false;

public:
    BufferedWriter(MOstream* os) : ostream(os) { 
    };
    int write(MBuffer* buffer) {
        return ostream->write((uint8_t*)buffer->buffer, buffer->length());
    }
};


#endif