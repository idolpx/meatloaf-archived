#include "cs.h"
#include "../../include/make_unique.h"
#include "utils.h"

/********************************************************
 * Client impls
 ********************************************************/
// fajna sciezka do sprawdzenia:
// utilities/disk tools/cie.d64

#define OK_REPLY "00 - OK\x0d\x0a\x04"

void CServerSessionMgr::connect() {
    int rc = m_wifi.connect("commodoreserver.com", 1541);
    Serial.printf("CServer: connect: %d\n", rc);

    if(breader==nullptr) {
        // do not initialize in constructor - compiler bug!
        Serial.println("breader ---- INIT!");
        breader = new StreamReader([this](uint8_t* buffer, size_t size)->int  {
            int x = this->read(buffer, size);
            Serial.printf("Lambda read %d\n",x);
            return x;
        });
        breader->delimiter = 10;
    }
}

void CServerSessionMgr::disconnect() {
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
        Serial.printf("CServer: send command: %s\n", command.c_str());
        m_wifi.write((command+'\r').c_str());
        return true;
    }
    else
        return false;
}

size_t CServerSessionMgr::read(uint8_t* buf, size_t size) {
    auto rd = m_wifi.read(buf, size);
    int attempts = 3;
    while(rd == 0 && (attempts--)>0) {
        delay(500);
        rd = m_wifi.read(buf, size);
    } 
    return rd;
}

size_t CServerSessionMgr::write(std::string &fileName, const uint8_t *buf, size_t size) {
    if(!m_wifi.connected())
        connect();

    if(m_wifi.connected())
        return m_wifi.write(buf, size);
    else
        return 0;
}

std::string CServerSessionMgr::readReply() {
    uint8_t buffer[40] = { 0 };
    memset(buffer, 0, sizeof(buffer));
    int rd = read(buffer, 40);
    for(int i=0; i<rd; i++) {
        Serial.printf("%d ", buffer[i]);
    }
    Serial.printf("CServer: replies %d bytes: '%s'\n", rd, buffer);
    return std::string((char *)buffer);
}

bool CServerSessionMgr::isOK() {
    return readReply() == OK_REPLY;
}


bool CServerSessionMgr::traversePath(MFile* path) {
    // tricky. First we have to
    // CF / - to go back to root
    command("cf /");

    if(isOK()) {
        auto chopped = path->chop();
        auto second = (chopped.begin())+2; // skipping scheme and empty 

        for(auto i = second; i < chopped.end(); i++) {
            auto part = (*i);
            
            if(/*MFileSystem::byExtension(".d64", part) || */MFileSystem::byExtension(".d64", part)) {
                // THEN we have to mount the image INSERT image_name
                command("insert "+part);
                // disk image is the end, so return
                if(isOK()) {
                    return true;
                }
                else {
                    // or: ?500 - DISK NOT FOUND.
                    return false;
                }
            }
            else {
                // CF xxx - to browse into subsequent dirs
                command("cf "+part);
                if(!isOK()) {
                    // or: ?500 - CANNOT CHANGE TO dupa
                    return false;
                }
            }
        }
    }
    else
        return false; // shouldn't really happen, right?
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
    m_isOpen = false;

    if(file->isDirectory())
        return false; // or do we want to stream whole d64 image? :D

    if(CServerFileSystem::session.traversePath(file.get())) {
        // should we allow loading of * in any directory?
        // then we can LOAD and get available count from first 2 bytes in (LH) endian
        // name here MUST BE UPPER CASE
        util_string_toupper(file->filename);
        CServerFileSystem::session.command("load "+file->filename);
        // read first 2 bytes with size, low first, but may also reply with: ?500 - ERROR
        uint8_t buffer[2] = { 0, 0 };
        read(buffer, 2);
        // hmmm... should we check if they're "?5" for error?!
        if(buffer[0]=='?' && buffer[1]=='5') {
            Serial.println("CServer: open file failed");
            CServerFileSystem::session.readReply();
        }
        else {
            m_bytesAvailable = buffer[0] + buffer[1]*256; // put len here
            // if everything was ok
            Serial.printf("CServer: file open, size: %d\n", m_bytesAvailable);
            m_isOpen = true;
        }
    }

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
    // if penultimate part is .d64 - it is a file
    // otherwise - false
    auto chopped = chop();
    auto second = (chopped.end())-2; // skipping scheme and empty 
    //auto x = (*second);
    //Serial.printf("isDirectory second from right:%s\n", x.c_str());

    return !MFileSystem::byExtension(".d64", *second);
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
    if(!isDirectory())
        return false;

    CServerFileSystem::session.traversePath(this);
    
    if(MFileSystem::byExtension(".d64", path())) {
        dirIsImage = true;
        dirIsOpen = true;
        // to list image contents we have to run
        Serial.println("cserver: this is a d64 img!");
        CServerFileSystem::session.command("$");

        return true;
    }
    else {
        dirIsImage = false;
        dirIsOpen = true;
        // to list directory contents we use
        Serial.println("cserver: this is a directory!");
        CServerFileSystem::session.command("disks");

        return true;
    }
};

MFile* CServerFile::getNextFileInDir() {
    Serial.println("cserver: getNextFileInDir0");
    if(!dirIsOpen)
        rewindDirectory();

    Serial.println("cserver: getNextFileInDir1");


    if(!dirIsOpen)
        return nullptr;

    Serial.println("cserver: getNextFileInDir2");


    if(dirIsImage) {
        auto line = CServerFileSystem::session.breader->readLn();
        // 'ot line:'0 ␒"CIE�������������" 00�2A�
        // 'ot line:'2   "CIE+SERIAL      " PRG   2049
        // 'ot line:'1   "CIE-SYS31801    " PRG   2049
        // 'ot line:'1   "CIE-SYS31801S   " PRG   2049
        // 'ot line:'1   "CIE-SYS52281    " PRG   2049
        // 'ot line:'1   "CIE-SYS52281S   " PRG   2049
        // 'ot line:'658 BLOCKS FREE.
        Serial.printf("cserver: got dir line: '%s'\n", line.c_str());

        if(line.find('\x04')>=0) {
            Serial.println("No more!");
            dirIsOpen = false;
            return nullptr;
        }
        else
            return new CServerFile("xxxxx");
    } else {
        auto line = CServerFileSystem::session.breader->readLn();
        // Got line:''
        // Got line:''
        // 'ot line:'FAST-TESTER DELUXE EXCESS.D64
        // 'ot line:'EMPTY.D64
        // 'ot line:'CMD UTILITIES D1.D64
        // 'ot line:'CBMCMD22.D64
        // 'ot line:'NAV96.D64
        // 'ot line:'NAV92.D64
        // 'ot line:'SINGLE DISKCOPY 64 (1983)(KEVIN PICKELL).D64
        // 'ot line:'LYNX (19XX)(-).D64
        // 'ot line:'GEOS DISK EDITOR (1990)(GREG BADROS).D64
        // 'ot line:'FLOPPY REPAIR KIT (1984)(ORCHID SOFTWARE LABORATOR
        // 'ot line:'1541 DEMO DISK (19XX)(-).D64

        Serial.printf("cserver: got dir line: '%s'\n", line.c_str());

        auto xx = line.c_str();

            for(int i=0; i<line.length(); i++) {
        Serial.printf("%d ", line[i]);
    }

        if(line.find('\x04')>=0) {
            Serial.println("No more!");
            dirIsOpen = false;
            return nullptr;
        }
        else
            return new CServerFile("xxxxx");
    }
};

bool CServerFile::exists() {} ;

size_t CServerFile::size() {} ;

bool CServerFile::mkDir() { 
    // but it does support creating dirs = MD FOLDER
    return false; 
};

bool CServerFile::remove() { 
    // but it does support remove = SCRATCH FILENAME
    return false; 
};

CServerSessionMgr CServerFileSystem::session;
