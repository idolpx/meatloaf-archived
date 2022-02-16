#include "meat_io.h"

#include "MIOException.h"

// Scheme
#include "scheme/littlefs.h"
#include "scheme/http.h"
#include "scheme/smb.h"
#include "scheme/ml.h"
#include "scheme/cs.h"
#include "scheme/ws.h"

// Disk
#include "media/d64.h"
#include "media/d71.h"
#include "media/d80.h"
#include "media/d81.h"
#include "media/d82.h"
#include "media/d8b.h"
#include "media/dnp.h"

// Tape
#include "media/t64.h"
#include "media/tcrt.h"

#include <vector>
#include <sstream>
#include "utils.h"
#include "string_utils.h"



/********************************************************
 * MFSOwner implementations
 ********************************************************/

// initialize other filesystems here
LittleFileSystem defaultFS(FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, 5);

// Scheme
HttpFileSystem httpFS;
MLFileSystem mlFS;
CServerFileSystem csFS;
WSFileSystem wsFS;

// Disk
D64FileSystem d64FS;
D71FileSystem d71FS;
D80FileSystem d80FS;
D81FileSystem d81FS;
D82FileSystem d82FS;
D8BFileSystem d8bFS;
DNPFileSystem dnpFS;

// Tape
T64FileSystem t64FS;
TCRTFileSystem tcrtFS;

// Cartridge



// put all available filesystems in this array - first matching system gets the file!
// fist in list is default
std::vector<MFileSystem*> MFSOwner::availableFS{ &defaultFS, &d64FS, &d71FS, &d80FS, &d81FS, &d82FS, &d8bFS, &dnpFS, &t64FS, &tcrtFS, &mlFS, &httpFS, &wsFS };

bool MFSOwner::mount(std::string name) {
    Serial.print("MFSOwner::mount fs:");
    Serial.print(name.c_str());

    for(auto i = availableFS.begin() + 1; i < availableFS.end() ; i ++) {
        auto fs = (*i);

        if(fs->handles(name)) {
                Serial.println("MFSOwner found a proper fs");

            bool ok = fs->mount();

            if(ok)
                Serial.print("Mounted fs:");
            else
                Serial.print("Couldn't mount fs:");

            Serial.print(name.c_str());

            return ok;
        }
    }
    return false;
}

bool MFSOwner::umount(std::string name) {
    for(auto i = availableFS.begin() + 1; i < availableFS.end() ; i ++) {
        auto fs = (*i);

        if(fs->handles(name)) {
            return fs->umount();
        }
    }
    return true;
}

MFile* MFSOwner::File(MFile* file) {
    return File(file->url);
}

MFile* MFSOwner::File(std::shared_ptr<MFile> file) {
    return File(file->url);
}


MFile* MFSOwner::File(std::string path) {
    if(mstr::startsWith(path,"cs:", false)) {
        //Serial.printf("CServer path found!\n");
        return csFS.getFile(path);
    }

    std::vector<std::string> paths = mstr::split(path,'/');

    //Debug_printv("Trying to factory path [%s]", path.c_str());

    auto pathIterator = paths.end();
    auto begin = paths.begin();
    auto end = paths.end();

    auto foundFS = testScan(begin, end, pathIterator);

    if(foundFS != nullptr) {
        //Debug_printv("PATH: '%s' is in FS [%s]", path.c_str(), foundFS->symbol);
        auto newFile = foundFS->getFile(path);
        //Debug_printv("newFile: '%s'", newFile->url.c_str());

        pathIterator++;
        newFile->pathInStream = mstr::joinToString(&pathIterator, &end, "/");
        //Debug_printv("newFile->pathInStream: '%s'", newFile->pathInStream.c_str());

        auto endHere = pathIterator;
        pathIterator--;

        if(begin == pathIterator) {
            //Debug_printv("** LOOK DOWN PATH NOT NEEDED   path[%s]", path.c_str());
            newFile->streamFile = foundFS->getFile(mstr::joinToString(&begin, &pathIterator, "/"));
            //newFile->streamFile = foundFS->getFile(path);
        } 
        else {
            auto upperPath = mstr::joinToString(&begin, &pathIterator, "/");
            //Debug_printv("** LOOK DOWN PATH: %s", upperPath.c_str());

            auto upperFS = testScan(begin, end, pathIterator);

            if(upperFS != nullptr) {
                auto wholePath = mstr::joinToString(&begin, &endHere, "/");

                //auto cp = mstr::joinToString(&begin, &pathIterator, "/");
                //Debug_printv("CONTAINER PATH WILL BE: '%s' ", wholePath.c_str());
                newFile->streamFile = upperFS->getFile(wholePath); // skończy się na d64
                //Debug_printv("CONTAINER: '%s' is in FS [%s]", newFile->streamFile->url.c_str(), upperFS->symbol);
            }
            else {
                Debug_printv("WARNING!!!! CONTAINER FAILED FOR: '%s'", upperPath.c_str());
            }
        }

        return newFile;
    }

    return nullptr;
}


MFileSystem* MFSOwner::testScan(std::vector<std::string>::iterator &begin, std::vector<std::string>::iterator &end, std::vector<std::string>::iterator &pathIterator) {
    while (pathIterator != begin) {
        pathIterator--;

        auto part = *pathIterator;
        mstr::toLower(part);

        //Debug_printv("index[%d] pathIterator[%s] size[%d]", pathIterator, pathIterator->c_str(), pathIterator->size());

        auto foundIter=find_if(availableFS.begin() + 1, availableFS.end(), [&part](MFileSystem* fs){ 
            //Debug_printv("symbol[%s]", fs->symbol);
            return fs->handles(part); 
        } );

        if(foundIter != availableFS.end()) {
            //Debug_printv("matched part '%s'\n", part.c_str());
            return (*foundIter);
        }
    };

    auto fs = *availableFS.begin();
    //Debug_printv("return default file system [%s]", fs->symbol);
    return fs;
}

/********************************************************
 * MFileSystem implementations
 ********************************************************/

MFileSystem::MFileSystem(const char* s)
{
    symbol = s;
}

MFileSystem::~MFileSystem() {}

/********************************************************
 * MFile implementations
 ********************************************************/

MFile::MFile(std::string path) {
    parseUrl(path);
}

MFile::MFile(std::string path, std::string name) : MFile(path + "/" + name) {}

MFile::MFile(MFile* path, std::string name) : MFile(path->path + "/" + name) {}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}

MIStream* MFile::inputStream() {
    // has to return OPENED stream
    Debug_printv("pathInStream[%s] streamFile[%s]", pathInStream.c_str(), streamFile->url.c_str());
    //std::shared_ptr<MFile> containerFile(MFSOwner::File(streamPath)); // get the base file that knows how to handle this kind of container, i.e 7z

    std::shared_ptr<MIStream> containerStream(streamFile->inputStream()); // get its base stream, i.e. zip raw file contents
    Debug_printv("containerStream isRandomAccess[%d] isBrowsable[%d]", containerStream->isRandomAccess(), containerStream->isBrowsable());

    MIStream* decodedStream(createIStream(containerStream)); // wrap this stream into decodec stream, i.e. unpacked zip files
    Debug_printv("decodedStream isRandomAccess[%d] isBrowsable[%d]", decodedStream->isRandomAccess(), decodedStream->isBrowsable());

    if(decodedStream->isRandomAccess() && pathInStream != "") {
        bool foundIt = decodedStream->seekPath(this->pathInStream);

        if(!foundIt)
        {
            Debug_printv("path in stream not found");
            return nullptr;
        }        
    }
    else if(decodedStream->isBrowsable() && pathInStream != "") {
        auto pointedFile = decodedStream->seekNextEntry();

        while (!pointedFile.empty())
        {
            if(pointedFile == this->pathInStream)
            {
                Debug_printv("returning decodedStream");
                return decodedStream;                
            }

            pointedFile = decodedStream->seekNextEntry();
        }
        Debug_printv("path in stream not found!");
        if(pointedFile.empty())
            return nullptr;        
    }

    Debug_printv("returning decodedStream");
    return decodedStream;
};


MFile* MFile::parent(std::string plus) {
    // drop last dir
    // add plus
    if(!path.empty()) {
        // from here we can go only to flash root!
        return MFSOwner::File("/");
    }
    else {
        int lastSlash = url.find_last_of('/');
        std::string newDir = mstr::dropLast(url, lastSlash);
        if(!plus.empty())
            newDir+= ("/" + plus);
        return MFSOwner::File(newDir);
    }
};

MFile* MFile::localParent(std::string plus) {
    // drop last dir
    // check if it isn't shorter than streamFile
    // add plus
    int lastSlash = url.find_last_of('/');
    std::string parent = mstr::dropLast(url, lastSlash);
    if(parent.length()-streamFile->url.length()>1)
        parent = streamFile->url;
    return MFSOwner::File(parent+"/"+plus);
};

MFile* MFile::root(std::string plus) {
    return new LittleFile("/"+plus);
};

MFile* MFile::localRoot(std::string plus) {
    return MFSOwner::File(streamFile->url+"/"+plus);
};

MFile* MFile::cd(std::string newDir) {

    Debug_printv("cd requested: [%s]", newDir.c_str());

    // OK to clarify - coming here there should be ONLY path or magicSymbol-path combo!
    // NO "cd:xxxxx", no "/cd:xxxxx" ALLOWED here! ******************
    //
    // if you want to support LOAD"CDxxxxxx" just parse/drop the CD BEFORE calling this function
    // and call it ONLY with the path you want to change into!

    if(newDir[0]=='/' && newDir[1]=='/') {
        if(newDir.size()==2) {
            // user entered: CD:// or CD//
            // means: change to the root of roots
            return MFSOwner::File("/"); // chedked, works ad flash root!
        }
        else {
            // user entered: CD://DIR or CD//DIR
            // means: change to a dir in root of roots
            return root(mstr::drop(newDir,2));
        }
    }
    else if(newDir[0]=='/' || newDir[0]=='^') {
        if(newDir.size()==1) {
            // user entered: CD:/ or CD/
            // means: change to container root
            // *** might require a fix for flash fs!
            //return MFSOwner::File(streamPath);
            return MFSOwner::File("/");
        }
        else {
            // user entered: CD:/DIR or CD/DIR
            // means: change to a dir in container root
            return localRoot(mstr::drop(newDir,1));
        }
    }
    else if(newDir[0]=='_') {
        if(newDir.size()==1) {
            // user entered: CD:_ or CD_
            // means: go up one directory
            return parent();
        }
        else {
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
            // user entered: CD:..DIR or CD..DIR
            // meaning: Go back one directory
            return localParent(mstr::drop(newDir,2));
        }
    }

    if(newDir[0]=='@' /*&& newDir[1]=='/' let's be consistent!*/) {
        if(newDir.size() == 1) {
            // user entered: CD:@ or CD@
            // meaning: go to the .sys folder
            return MFSOwner::File("/.sys");
        }
        else {
            // user entered: CD:@FOLDER or CD@FOLDER
            // meaning: go to a folder in .sys folder
            return MFSOwner::File("/.sys/" + mstr::drop(newDir,1));
        }
    }

    if(newDir.find(':') != std::string::npos) {
        // I can only guess we're CDing into another url scheme, this means we're changing whole path
        return MFSOwner::File(newDir);
    }
    else {
        Debug_printv("> url[%s] newDir[%s]", url.c_str(), newDir.c_str());
        // Add new directory to path
        if ( !mstr::endsWith(url, "/") )
            url.push_back('/');

        return MFSOwner::File(url+newDir);
    }
};

bool MFile::copyTo(MFile* dst) {
    auto istream = Meat::ifstream(this);
    auto ostream = Meat::ofstream(dst);

    int rc;

    istream.open();
    ostream.open();

    //Debug_printv("in copyTo, iopen=%d oopen=%d", istream.is_open(), ostream.is_open());

    if(!istream.is_open() || !ostream.is_open())
        return false;

    //Debug_printv("commencing copy");

    while((rc = istream.get())!= EOF) {     
        //Serial.print(".");
        ostream.put(rc);
        if(ostream.bad() || istream.bad())
            return false;
    }

    //Debug_printv("copying finished, rc=%d", rc);

    return true;
};





