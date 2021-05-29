#include "fs_littlefs.h"

/********************************************************
 * MFileSystem implementations
 ********************************************************/

MFile* LittleFileSystem::create(String path)
{
    return new LittleFile(path);
}

/********************************************************
 * MFile implementations
 ********************************************************/

bool LittleFile::isFile()
{

}

bool LittleFile::isDirectory()
{
    // TODO!
}

MIstream* LittleFile::inputStream()
{
     // return OPENED stream
     MIstream* istream = new LittleIStream();
     istream->open();   
     return istream;
}

MOstream* LittleFile::outputStream()
{
    // return OPENED stream
    MOstream* ostream = new LittleOStream();
    ostream->open();   
    return ostream;
}

time_t LittleFile::getLastWrite()
{
    
}

time_t LittleFile::getCreationTime()
{
    
}

void LittleFile::setTimeCallback(time_t (*cb)(void))
{
    
}

bool LittleFile::rewindDirectory()
{
    
}

MFile* LittleFile::getNextFileInDir()
{
    
}

bool LittleFile::mkDir()
{
    // TODO!   
}

bool LittleFile::mkDirs()
{
    
}

bool LittleFile::exists()
{
    // TODO!
}

size_t LittleFile::size() {
    // TODO!
}

bool LittleFile::remove() {
    // TODO!
    // musi obslugiwac usuwanie plikow i katalogow!
}

bool LittleFile::truncate(size_t size) {
    // TODO!
}

bool LittleFile::rename(const char* dest) {
    // TODO!
}

/********************************************************
 * MStreams implementations
 ********************************************************/
    // MStream methods
    bool LittleOStream::seek(uint32_t pos, SeekMode mode) {

    };
    bool LittleOStream::seek(uint32_t pos) {

    };
    size_t LittleOStream::position() const {

    };
    void LittleOStream::close() {
        if(isOpen()) {

        }
    };
    bool LittleOStream::open() {
        // remember to set m_isOpen if succesful
    };
    // LittleOStream::~LittleOStream(

    // );

    // MOstream methods
    size_t LittleOStream::write(uint8_t) {

    };
    size_t LittleOStream::write(const uint8_t *buf, size_t size) {

    };
    void LittleOStream::flush() {

    };

    // MStream methods
    bool LittleIStream::seek(uint32_t pos, SeekMode mode) {

    };
    bool LittleIStream::seek(uint32_t pos) {

    };
    size_t LittleIStream::position() const {

    };
    void LittleIStream::close() {
        if(isOpen()) {
            
        }
    };
    bool LittleIStream::open() {
        // remember to set m_isOpen if succesful
    };
    // LittleIStream::~LittleIStream(

    // );

    // MIstream methods
    int LittleIStream::available() {
        // TODO!
    };
    int LittleIStream::read() {

    };
    int LittleIStream::peek() {

    };
    size_t LittleIStream::readBytes(char *buffer, size_t length) {

    };
    size_t LittleIStream::read(uint8_t* buf, size_t size) {

    };
