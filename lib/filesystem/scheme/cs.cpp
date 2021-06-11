#include "cs.h"
#include "../../include/make_unique.h"


/********************************************************
 * Client impls
 ********************************************************/

void CServerSessionMgr::connect() {
    m_wifi.connect("commodoreserver.com", 1541);
}

void CServerSessionMgr::disconnect() {
    // QUIT - might be close stream
    if(m_wifi.connected()) {
        command("quit");
        m_wifi.stop();
    }
}

bool CServerSessionMgr::command(std::string command) {
    // 13 (CR) sends the command
    if(!m_wifi.connected())
        connect();

    if(m_wifi.connected()) {
        m_wifi.write((command+'\n').c_str());
        return true;
    }
    else
        return false;
}

size_t CServerSessionMgr::read(uint8_t* buf, size_t size) {
    return m_wifi.read(buf, size);
}

size_t CServerSessionMgr::write(std::string &fileName, const uint8_t *buf, size_t size) {
    if(!m_wifi.connected())
        connect();

    if(m_wifi.connected())
        return m_wifi.write(buf, size);
    else
        return 0;
}

bool CServerSessionMgr::traversePath(MFile* path) {
    // tricky. First we have to
    // CF / - to go back to root
    command("cf /");
    // CF xxx - to browse into subsequent dirs
    // each CF should return: 00-OK
    // or: ?500 - CANNOT CHANGE TO dupa
    // THEN we have to mount the image INSERT image_name
    command("insert disk_name");
    // should reply with: 00 - OK
    // or: ?500 - DISK NOT FOUND.

}

/********************************************************
 * I Stream impls
 ********************************************************/

bool CServerIStream::seek(uint32_t pos, SeekMode mode) {
    return false;
};

bool CServerIStream::seek(uint32_t pos)  {
    return false;
};

size_t CServerIStream::position() {
    return m_position;
};

void CServerIStream::close() {
    m_isOpen = false;
};

bool CServerIStream::open() {
    auto file = std::make_unique<CServerFile>(m_path);

    if(CServerFileSystem::session.traversePath(file.get())) {

        // then we can LOAD and get available count from first 2 bytes in (LH) endian
        CServerFileSystem::session.command("load name_here");
        // read first 2 bytes with size, low first, but may also reply with: ?500 - ERROR
        m_bytesAvailable = 6666; // put len here

        // if everything was ok
        m_isOpen = true;
    }
    else
        m_isOpen = false;

    return m_isOpen;
};

// MIstream methods
int CServerIStream::available() {
    return m_bytesAvailable;
};

size_t CServerIStream::read(uint8_t* buf, size_t size)  {
    auto bytesRead = CServerFileSystem::session.read(buf, size);
    m_bytesAvailable-=bytesRead;
    m_position+=bytesRead;
    ledToggle(true);
    return bytesRead;
};

bool CServerIStream::isOpen() {
    return m_isOpen;
}

/********************************************************
 * O Stream impls
 ********************************************************/

bool CServerOStream::seek(uint32_t pos, SeekMode mode) {
    return false;
};

bool CServerOStream::seek(uint32_t pos) {
    return false;
};

size_t CServerOStream::position() {
    return 0;
};

void CServerOStream::close() {
    m_isOpen = false;
};

bool CServerOStream::open() {
    auto file = std::make_unique<CServerFile>(m_path);

    if(CServerFileSystem::session.traversePath(file.get())) {
        m_isOpen = true;
    }
    else
        m_isOpen = false;

    return m_isOpen;
};

// MOstream methods
size_t CServerOStream::write(const uint8_t *buf, size_t size) {
    // we have to write all at once... sorry...
    auto file = std::make_unique<CServerFile>(m_path);

    CServerFileSystem::session.command("save fileName,size[,type=PRG,SEQ]");
    m_isOpen = false; // c64 server supports only writing all at once, so this channel has to be marked closed
    return CServerFileSystem::session.write(file->filename, buf, size);
};

void CServerOStream::flush() {
    CServerFileSystem::session.flush();
};

bool CServerOStream::isOpen() {
    return m_isOpen;
};

/********************************************************
 * File impls
 ********************************************************/

bool CServerFile::isDirectory() {
    // CS files [named like this] are directories
};

MIstream* CServerFile::inputStream() {
    MIstream* istream = new CServerIStream(m_path);
    istream->open();   
    return istream;
}; 

MOstream* CServerFile::outputStream() {
    MOstream* ostream = new CServerOStream(m_path);
    ostream->open();   
    return ostream;
};

bool CServerFile::rewindDirectory() {
    // DISKS - disks is our de facto directory listing command, unless previous path part is .d64, then we have to use $ command to list the dir!
    // DISKS and $ ends with 4 (EOT)
    // format:
    //  >[PUBLIC ROOT]
    // [APPS]
    // [CLUBS]
    // [COMMS]
    // [DEMOS]

};

MFile* CServerFile::getNextFileInDir() {

};

bool CServerFile::exists() {} ;

size_t CServerFile::size() {} ;

bool CServerFile::mkDir() { 
    // but it does support creating dirs = MD FOLDER
    return false; 
};

bool remove() { 
    // but it does support remove = SCRATCH FILENAME
    return false; 
};
