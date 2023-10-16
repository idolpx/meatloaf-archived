#include "cs.h"

/********************************************************
 * Client impls
 ********************************************************/
// fajna sciezka do sprawdzenia:
// utilities/disk tools/cie.d64

#define OK_REPLY "00 - OK\x0d"

CServerSessionMgr CServerFileSystem::session;

bool CServerSessionMgr::establishSession() {
    if(!buf.is_open()) {
        currentDir = "cs:/";
        buf.open();
    }
    
    return buf.is_open();
}

std::string CServerSessionMgr::readLn() {
    char buffer[80];
    // telnet line ends with 10;
    getline(buffer, 80, 10);
    //Debug_printv("Inside readLn: %s", buffer);
    return std::string((char *)buffer);
}

bool CServerSessionMgr::sendCommand(std::string command) {
    // 13 (CR) sends the command
    if(establishSession()) {
        Serial.printf("CServer: send command: %s\n", command.c_str());
        (*this) << (command+'\r');
        (*this).flush();
        return true;
    }
    else
        return false;
}

bool CServerSessionMgr::isOK() {
    // auto a = readLn();

    // for(int i = 0 ; i<a.length(); i++)
    //     Debug_printv("'%d'", a[i]);

    return readLn() == OK_REPLY;
}

bool CServerSessionMgr::traversePath(MFile* path) {
    // tricky. First we have to
    // CF / - to go back to root

    //Debug_printv("Traversing path: [%s]", path->path.c_str());

    if(buf.is_open()) {
        // if we are still connected we can smart change dir by just going up or down
        // but for time being, we stick to traversing from root
        if(!sendCommand("cf /"))
            return false;
    }
    else {
        // if we aren't, change dir to root (alos connects the session);
        if(!sendCommand("cf /"))
            return false;
    }

    if(isOK()) {
        if(path->path.compare("/") == 0) {
            currentDir = path->url;
            return true;
        }

        std::vector<std::string> chopped = mstr::split(path->path, '/');

        //MFile::parsePath(&chopped, path->path); - nope this doessn't work and crases in the loop!

        Debug_printv("Before loop");
        //Debug_printv("Chopped size:%d\n", chopped.size());
        delay(500);

        for(size_t i = 1; i < chopped.size(); i++) {
            //Debug_printv("Before chopped deref");

            auto part = chopped[i];
            
            //Debug_printv("traverse path part: [%s]\n", part.c_str());
            if(mstr::endsWith(part, ".d64", false)) 
            {
                // THEN we have to mount the image INSERT image_name
                sendCommand("insert "+part);

                // disk image is the end, so return
                if(isOK()) {
                    currentDir = path->url;
                    return true;
                }
                else {
                    // or: ?500 - DISK NOT FOUND.
                    return false;
                }
            }
            else 
            {
                // CF xxx - to browse into subsequent dirs
                sendCommand("cf "+part);
                if(!isOK()) {
                    // or: ?500 - CANNOT CHANGE TO dupa
                    return false;
                }
            }
        }
        
        currentDir = path->url;
        return true;
    }
    else
        return false; // shouldn't really happen, right?
}

/********************************************************
 * I Stream impls
 ********************************************************/

size_t CServerIStream::position() {
    return m_position;
};

void CServerIStream::close() {
    m_isOpen = false;
};

bool CServerIStream::open() {
    auto file = std::make_unique<CServerFile>(url);
    m_isOpen = false;

    if(file->isDirectory())
        return false; // or do we want to stream whole d64 image? :D

    if(CServerFileSystem::session.traversePath(file.get())) {
        // should we allow loading of * in any directory?
        // then we can LOAD and get available count from first 2 bytes in (LH) endian
        // name here MUST BE UPPER CASE
        // trim spaces from right of name too
        mstr::rtrimA0(file->name);
        mstr::toPETSCII(file->name);
        CServerFileSystem::session.sendCommand("load "+file->name);
        // read first 2 bytes with size, low first, but may also reply with: ?500 - ERROR
        uint8_t buffer[2] = { 0, 0 };
        read(buffer, 2);
        // hmmm... should we check if they're "?5" for error?!
        if(buffer[0]=='?' && buffer[1]=='5') {
            Serial.println("CServer: open file failed");
            CServerFileSystem::session.readLn();
            m_isOpen = false;
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

// MIStream methods
size_t CServerIStream::available() {
    return m_bytesAvailable;
};

size_t CServerIStream::size() {
    return m_bytesAvailable;
};

size_t CServerIStream::read(uint8_t* buf, size_t size)  {
    //Serial.println("CServerIStream::read");
    auto bytesRead = CServerFileSystem::session.receive(buf, size);
    m_bytesAvailable-=bytesRead;
    m_position+=bytesRead;
    //ledTogg(true);
    return bytesRead;
};

bool CServerIStream::isOpen() {
    return m_isOpen;
}

/********************************************************
 * O Stream impls
 ********************************************************/

size_t CServerOStream::position() {
    return 0;
};

void CServerOStream::close() {
    m_isOpen = false;
};

bool CServerOStream::open() {
    auto file = std::make_unique<CServerFile>(url);

    if(CServerFileSystem::session.traversePath(file.get())) {
        m_isOpen = true;
    }
    else
        m_isOpen = false;

    return m_isOpen;
};

// MOStream methods
size_t CServerOStream::write(const uint8_t *buf, size_t size) {
    // we have to write all at once... sorry...
    auto file = std::make_unique<CServerFile>(url);

    CServerFileSystem::session.sendCommand("save fileName,size[,type=PRG,SEQ]");
    m_isOpen = false; // c64 server supports only writing all at once, so this channel has to be marked closed
    return CServerFileSystem::session.send(buf, size);
};


bool CServerOStream::isOpen() {
    return m_isOpen;
};

/********************************************************
 * File impls
 ********************************************************/


MFile* CServerFile::cd(std::string newDir) {
    // maah - don't really know how to handle this!

    // Drop the : if it is included
    if(newDir[0]==':') {
        Debug_printv("[:]");
        newDir = mstr::drop(newDir,1);
    }

    Debug_printv("cd in CServerFile! New dir [%s]\n", newDir.c_str());
    if(newDir[0]=='/' && newDir[1]=='/') {
        if(newDir.size()==2) {
            // user entered: CD:// or CD//
            // means: change to the root of roots
            return MFSOwner::File("/"); // chedked, works ad flash root!
        }
        else {
            // user entered: CD://DIR or CD//DIR
            // means: change to a dir in root of roots
            Debug_printv("[//]");
            return root(mstr::drop(newDir,2));
        }
    }
    else if(newDir[0]=='/') {
        if(newDir.size()==1) {
            // user entered: CD:/ or CD/
            // means: change to container root
            // *** might require a fix for flash fs!
            return MFSOwner::File(streamFile->path);
        }
        else {
            Debug_printv("[/]");
            // user entered: CD:/DIR or CD/DIR
            // means: change to a dir in container root
            return MFSOwner::File("cs:/"+mstr::drop(newDir,1));
        }
    }
    else if(newDir[0]=='_') {
        if(newDir.size()==1) {
            // user entered: CD:_ or CD_
            // means: go up one directory
            return parent();
        }
        else {
            Debug_printv("[_]");
            // user entered: CD:_DIR or CD_DIR
            // means: go to a directory in the same directory as this one
            return parent(mstr::drop(newDir,1));
        }
    }
    if(newDir[0]=='.' && newDir[1]=='.') {
        if(newDir.size()==2) {
            // user entered: CD:.. or CD..
            // means: go up one directory
            return parent();
        }
        else {
            Debug_printv("[..]");
            // user entered: CD:..DIR or CD..DIR
            // meaning: Go back one directory
            return localParent(mstr::drop(newDir,2));
        }
    }

    // ain't that redundant?
    // if(newDir[0]=='.' && newDir[1]=='/') {
    //     Debug_printv("[./]");
    //     // Reference to current directory
    //     return localParent(mstr::drop(newDir,2));
    // }

    if(newDir[0]=='~' /*&& newDir[1]=='/' let's be consistent!*/) {
        if(newDir.size() == 1) {
            // user entered: CD:~ or CD~
            // meaning: go to the .sys folder
            return MFSOwner::File("/.sys");
        }
        else {
            Debug_printv("[~]");
            // user entered: CD:~FOLDER or CD~FOLDER
            // meaning: go to a folder in .sys folder
            return MFSOwner::File("/.sys/" + mstr::drop(newDir,1));
        }
    }    
    if(newDir.find(':') != std::string::npos) {
        // I can only guess we're CDing into another url scheme, this means we're changing whole path
        return MFSOwner::File(newDir);
    }
    else {
        // Add new directory to path
        if(mstr::endsWith(url,"/"))
            return MFSOwner::File(url+newDir);
        else
            return MFSOwner::File(url+"/"+newDir);
    }
};


bool CServerFile::isDirectory() {
    // if penultimate part is .d64 - it is a file
    // otherwise - false

    //Debug_printv("trying to chop [%s]", path.c_str());

    auto chopped = mstr::split(path,'/');

    if(path.empty()) {
        // rood dir is a dir
        return true;
    }
    if(chopped.size() == 1) {
        // we might be in an image in the root
        return mstr::endsWith((chopped[0]), ".d64", false);
    }
    if(chopped.size()>1) {
        auto second = chopped.end()-2;
        
        //auto x = (*second);
        // Debug_printv("isDirectory second from right: [%s]", (*second).c_str());
        if ( mstr::endsWith((*second), ".d64", false))
            return false;
        else
            return true;
    }
    return false;
};

MIStream* CServerFile::inputStream() {
    MIStream* istream = new CServerIStream(url);
    istream->open();   
    return istream;
}; 

MOStream* CServerFile::outputStream() {
    MOStream* ostream = new CServerOStream(url);
    ostream->open();   
    return ostream;
};

bool CServerFile::rewindDirectory() {    
    dirIsOpen = false;

    if(!isDirectory())
        return false;

    if(!CServerFileSystem::session.traversePath(this)) return false;

    if(mstr::endsWith(path, ".d64", false))
    {
        dirIsImage = true;
        // to list image contents we have to run
        //Serial.println("cserver: this is a d64 img!");
        CServerFileSystem::session.sendCommand("$");
        auto line = CServerFileSystem::session.readLn(); // mounted image name
        if(CServerFileSystem::session.is_open()) {
            dirIsOpen = true;
            media_image = line.substr(5);
            line = CServerFileSystem::session.readLn(); // dir header
            media_header = line.substr(2, line.find_last_of("\""));
            media_id = line.substr(line.find_last_of("\"")+2);
            return true;
        }
        else
            return false;
    }
    else 
    {
        dirIsImage = false;
        // to list directory contents we use
        //Serial.println("cserver: this is a directory!");
        CServerFileSystem::session.sendCommand("disks");
        auto line = CServerFileSystem::session.readLn(); // dir header
        if(CServerFileSystem::session.is_open()) {
            media_header = line.substr(2, line.find_last_of("]")-1);
            media_id = "C=SVR";
            dirIsOpen = true;

            return true;
        }
        else 
            return false;
    }
};

MFile* CServerFile::getNextFileInDir() {
    if(!dirIsOpen)
        rewindDirectory();

    if(!dirIsOpen)
        return nullptr;

    std::string name;
    size_t size;
    std::string new_url = url;

    if(url.size()>4) // If we are not at root then add additional "/"
        new_url += "/";

    if(dirIsImage) {
        auto line = CServerFileSystem::session.readLn();
        Debug_printv("next file in dir got %s", line);
        // 'ot line:'0 ␒"CIE�������������" 00�2A�
        // 'ot line:'2   "CIE+SERIAL      " PRG   2049
        // 'ot line:'1   "CIE-SYS31801    " PRG   2049
        // 'ot line:'1   "CIE-SYS31801S   " PRG   2049
        // 'ot line:'1   "CIE-SYS52281    " PRG   2049
        // 'ot line:'1   "CIE-SYS52281S   " PRG   2049
        // 'ot line:'658 BLOCKS FREE.

        if(line.find('\x04')!=std::string::npos) {
            Serial.println("No more!");
            dirIsOpen = false;
            return nullptr;
        }
        if(line.find("BLOCKS FREE.")!=std::string::npos) {
            media_blocks_free = atoi(line.substr(0, line.find_first_of(" ")).c_str());
            dirIsOpen = false;
            return nullptr;
        }
        else {
            name = line.substr(5,15);
            size = atoi(line.substr(0, line.find_first_of(" ")).c_str());
            mstr::rtrim(name);
            Debug_printv("xx: %s -- %s %d", line.c_str(), name.c_str(), size);
            //return new CServerFile(path() +"/"+ name);
            new_url += name;
            return new CServerFile(new_url, size);
        }
    } else {
        auto line = CServerFileSystem::session.readLn();
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

        // 32 62 91 68 73 83 75 32 84 79 79 76 83 93 13 No more! = > [DISK TOOLS]

        if(line.find('\x04')!=std::string::npos) {
            Serial.println("No more!");
            dirIsOpen = false;
            return nullptr;
        }
        else {

            if((*line.begin())=='[') {
                name = line.substr(1,line.length()-3);
                size = 0;
            }
            else {
                name = line.substr(0, line.length()-1);
                size = 683;
            }

            // Debug_printv("\nurl[%s] name[%s] size[%d]\n", url.c_str(), name.c_str(), size);
            if(name.size() > 0)
            {
                new_url += name;
                return new CServerFile(new_url, size);                
            }
            else
                return nullptr;
        }
    }
};

bool CServerFile::exists() {
    return true;
} ;

size_t CServerFile::size() {
    return m_size;
};

bool CServerFile::mkDir() { 
    // but it does support creating dirs = MD FOLDER
    return false; 
};

bool CServerFile::remove() { 
    // but it does support remove = SCRATCH FILENAME
    return false; 
};

 