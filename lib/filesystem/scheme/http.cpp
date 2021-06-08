#include "http.h"

/********************************************************
 * File impls
 ********************************************************/

bool HttpFile::isDirectory() {
    // hey, why not?
    //return m_path.endsWith("/");
    return m_path.back() == '/';
};

MIstream* HttpFile::inputStream() {
    // has to return OPENED stream
    MIstream* istream = new HttpIStream(m_path);
    istream->open();   
    return istream;
} ; 

MIstream* HttpFile::createIStream(MIstream* is) {
    return is; // we don't have to process this stream in any way, just return the original stream
}

MOstream* HttpFile::outputStream() {
    // has to return OPENED stream
    MOstream* ostream = new HttpOStream(m_path);
    ostream->open();   
    return ostream;

} ; 

time_t HttpFile::getLastWrite() {
    return 0;    
} ;

time_t HttpFile::getCreationTime() {
    return 0;
} ;

bool HttpFile::exists() {
    Serial.printf("\nHttpFile::exists: [%s]\n", m_path.c_str());
    // we may try open the stream to check if it exists
    std::unique_ptr<MIstream> test(inputStream());

    return test->isOpen();
    // remember that MIStream destuctor should close the stream!
} ; 

size_t HttpFile::size() {
    // we may take content-lenght from header if exists
    return 0;
} ; 



/********************************************************
 * Ostream impls
 ********************************************************/

bool HttpOStream::seek(uint32_t pos, SeekMode mode) {};
bool HttpOStream::seek(uint32_t pos) {};
size_t HttpOStream::position() {};
void HttpOStream::close() {};
bool HttpOStream::open() {
    // obbiously since we have to know the size we send for post data
    // we will deal with it via a writer, not raw stream
    auto someRc = m_http.begin(m_client, m_path.c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //auto httpCode = http.POST(post_data); //Send the request
};
size_t HttpOStream::write(uint8_t) {};
size_t HttpOStream::write(const uint8_t *buf, size_t size) {};
void HttpOStream::flush() {};
bool HttpOStream::isOpen() {};


/********************************************************
 * Istream impls
 ********************************************************/

bool HttpIStream::seek(uint32_t pos, SeekMode mode) {
    // maybe we can use http resume here?
};

bool HttpIStream::seek(uint32_t pos) {
    // maybe we can use http resume here?
};

size_t HttpIStream::position() {
    return m_position;
};

void HttpIStream::close() {
    m_http.end();
};

bool HttpIStream::open() {
    Serial.printf("\nHttpIStream::open: [%s]\n", m_path.c_str());
    auto someRc = m_http.begin(m_client, m_path.c_str());
    auto httpCode = m_http.GET(); //Send the request
    m_file = m_http.getStream();  //Get the response payload as Stream
    m_isOpen = m_file.available();
    m_length = m_http.getSize();
    m_bytesAvailable = m_length;
};

int HttpIStream::available() {
    return m_bytesAvailable;
};

uint8_t HttpIStream::read() {};

size_t HttpIStream::read(uint8_t* buf, size_t size) {
    auto bytesRead= m_file.readBytes((char *) buf, size);
    m_bytesAvailable-=bytesRead;
    m_position+=bytesRead;
    return bytesRead;
};

bool HttpIStream::isOpen() {
    return m_isOpen;
};
