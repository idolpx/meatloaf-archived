#include "meat_io.h"

#include <flash_hal.h>

#include "MIOException.h"
#include "fs_littlefs.h"
#include "media/dnp.h"
#include "scheme/http.h"
#include "scheme/smb.h"
#include "scheme/ml.h"
#include <vector>
#include <sstream>
#include "utils.h"


std::string joinNamesToPath(std::vector<std::string>::iterator* start, std::vector<std::string>::iterator* end) {
    std::string res;

    if((*start)>=(*end))
        return std::string();

    for(auto i = (*start); i<(*end); i++) {
        res+=(*i);
        if(i<(*end))
            res+="/";
    }

    return res.erase(res.length()-1,1);
}

/********************************************************
 * MFSOwner implementations
 ********************************************************/

// initialize other filesystems here
LittleFileSystem littleFS(FS_PHYS_ADDR, FS_PHYS_SIZE, FS_PHYS_PAGE, FS_PHYS_BLOCK, 5);
HttpFileSystem httpFS;
DNPFileSystem dnpFS;
MLFileSystem mlFS;

// put all available filesystems in this array
// put littleFS as last, fallback system so it can be used if nothing matches
//MFileSystem* MFSOwner::availableFS[FS_COUNT] = { &httpFS, &littleFS };
std::vector<MFileSystem*> MFSOwner::availableFS{  &dnpFS, &httpFS, &mlFS };


// MFile* MFSOwner::File(std::string name) {
//     for(auto i = availableFS.begin(); i < availableFS.end() ; i ++) {
//         Serial.printf("FSTEST: trying to find fs for %s = %s\n", name.c_str(), (*i)->protocol);
        
//         if((*i)->handles(name)) {
//             Serial.println("FSTEST: found a proper fs");
//             return (*i)->getFile(name);
//         }
//     }
    
//     return nullptr;
// }

bool MFSOwner::mount(std::string name) {
    Serial.print("MFSOwner::mount fs:");
    Serial.print(name.c_str());

    for(auto i = availableFS.begin(); i < availableFS.end() ; i ++) {
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
    for(auto i = availableFS.begin(); i < availableFS.end() ; i ++) {
        auto fs = (*i);

        if(fs->handles(name)) {
            return fs->umount();
        }
    }
    return true;
}


void MFile::fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last) {
    //Serial.println("w fillpaths");   

    (*matchedElement)++;

    //Serial.println("w fillpaths stream pths");
    delay(500);   
    streamPath = joinNamesToPath(fromStart, matchedElement);
    //Serial.println("w fillpaths path in stream");   
    delay(500);   
    pathInStream = joinNamesToPath(matchedElement, last);

    Serial.printf("streamSrc='%s'\npathInStream='%s'\n", streamPath.c_str(), pathInStream.c_str());
}

MFile* MFSOwner::File(std::string path) {
    std::vector<std::string> paths;
    
    std::string line;
    std::stringstream ss(path);

    while(std::getline(ss, line, '/')) {
        paths.push_back(line);
    }

    auto pathIterator = paths.end();
    auto begin = paths.begin();
    auto end = paths.end();

    while (pathIterator != paths.begin()) {
        pathIterator--;

        auto part = *pathIterator;
        util_string_toupper(part);

        //Serial.printf("testing part '%s'\n", part.c_str());

        auto foundIter=find_if(availableFS.begin(), availableFS.end(), [&part](MFileSystem* fs){ return fs->handles(part); } );

        if(foundIter != availableFS.end()) {
Serial.printf("matched fs: %s [%s]\n", (*foundIter)->symbol, path.c_str());
            auto newFile = (*foundIter)->getFile(path);
            newFile->fillPaths(&pathIterator, &begin, &end);
            newFile->parseUrl(path);

            return newFile;
         }
    };

    Serial.printf("Little fs fallback\n");

    MFile* newFile = new LittleFile(path);
    newFile->streamPath = path;
    newFile->pathInStream = "";

    return newFile;
}


/********************************************************
 * MFileSystem implementations
 ********************************************************/

MFileSystem::MFileSystem(char* s)
{
    symbol = s;
}

MFileSystem::~MFileSystem() {}

// bool MFileSystem::handles(std::string path) 
// {
//     return path.startsWith(protocol);
// }

/********************************************************
 * MFile implementations
 ********************************************************/

MFile::MFile(std::string path) {
    m_path = path;
}

MFile::MFile(std::string path, std::string name) : MFile(path + "/" + name) {}

MFile::MFile(MFile* path, std::string name) : MFile(path->m_path + "/" + name) {}

bool MFile::operator!=(nullptr_t ptr) {
    return m_isNull;
}

// std::string MFile::name() {
//     int lastSlash = m_path.find_last_of('/');
//     return m_path.substr(lastSlash+1);;
// }

std::string MFile::path() {
    return m_path;
}

// std::string MFile::extension() {
//     int lastPeriod = m_path.find_last_of(".");
//     if(lastPeriod < 0)
//         return "";
//     else
//         return m_path.substr(lastPeriod+1);
// }

MIstream* MFile::inputStream() {
    ; // has to return OPENED stream
    std::shared_ptr<MFile> srcStreamProvider(MFSOwner::File(streamPath)); // get the base file that knows how to handle this kind of container
    std::shared_ptr<MIstream> srcStream(srcStreamProvider->inputStream()); // get its base stream, i.e. zip raw file contents

    std::shared_ptr<MIstream> thisStreamPtr(createIStream(srcStream.get())); // wrap this stream into decodec stream, i.e. unpacked zip files

    auto thisStream = thisStreamPtr.get();

    if(pathInStream != "" && isBrowsable()) {
        // stream is browsable and path was requested, let's skip the stream to requested file
        std::unique_ptr<MFile> pointedFile(getNextEntry());
        while (pointedFile != nullptr)
        {
            if(pointedFile->path() == this->pathInStream)
                return thisStream;

            pointedFile.reset(getNextEntry());
        }
        
        return nullptr; // there was no file with that name in this stream!
    }
    else {
        return nullptr; // path requested for unbrowsable stream
    }

    return thisStream;
};

/********************************************************
 * MStream implementations
 ********************************************************/

MStream::~MStream() {};