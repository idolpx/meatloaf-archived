#ifndef MEATFILE_DEFINES_ZIP_STREAMS_H
#define MEATFILE_DEFINES_ZIP_STREAMS_H

#include "meat_io.h"

class ZipInputStream: MIstream {
public:
    ZipInputStream(std::string& path) {
        m_path = path;
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~ZipInputStream() {
        close();
    }

    // MIstream methods
    int available() override;
    uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

    // Zip-specific methods
    MFile* getNextEntry(); // skips the stream until the beginnin of next file

protected:
    std::string m_path;

};

#endif