#ifndef MEATLINK_DEFINES_H
#define MEATLINK_DEFINES_H

#include <memory>
#include <Arduino.h>

enum SeekMode {
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

class MStream {
    MStream(MFile file);
    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos);
    size_t position() const;
    void close();
};

class MIstream: MStream {
    MIstream(MFile file);
    int available();
    int read();
    int peek();
    size_t readBytes(char *buffer, size_t length);
    size_t read(uint8_t* buf, size_t size);
};

class MOstream: MStream {
    MOstream(MFile file);
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    void flush();
};


class MFile {
    MFile(String name);
    MFile(String name, String subDir);
    MFile(MFile name, String subDir);
    const char* name() const;
    const char* fullName() const; // Includes path
    const char* extension() const;
    bool isFile() const;
    bool isDirectory() const;
    MIstream inputStream();
    MOstream outputStream();
    time_t getLastWrite();
    time_t getCreationTime();
    void setTimeCallback(time_t (*cb)(void));
    bool rewindDirectory();
    MFile openNextFile();
    bool mkDir();
    bool mkDirs();
};

#endif