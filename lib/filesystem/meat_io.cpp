#include "meat_io.h"

#include <flash_hal.h>

#include "MIOException.h"
#include "fs_littlefs.h"
#include "media/d64.h"
#include "media/dnp.h"
#include "media/url.h"
#include "scheme/http.h"
#include "scheme/smb.h"
#include "scheme/ml.h"
#include "scheme/cs.h"
#include <vector>
#include <sstream>
#include "utils.h"
#include "string_utils.h"


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
D64FileSystem d64FS;
DNPFileSystem dnpFS;
URLFileSystem urlFS;
MLFileSystem mlFS;
CServerFileSystem csFS;


// put all available filesystems in this array - first matching system gets the file!
std::vector<MFileSystem*> MFSOwner::availableFS{ &d64FS, &dnpFS, &urlFS, &mlFS, &httpFS };

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

MFile* MFSOwner::File(std::string path) {
    std::vector<std::string> paths = mstr::split(path,'/');

    auto pathIterator = paths.end();
    auto begin = paths.begin();
    auto end = paths.end();

    if(mstr::startsWith(path,"cs:", false)) {
        //Serial.printf("CServer path found!\n");
        return csFS.getFile(path);
    }

    while (pathIterator != paths.begin()) {
        pathIterator--;

        auto part = *pathIterator;

        //Serial.printf("testing part '%s'\n", part.c_str());

        auto foundIter=find_if(availableFS.begin(), availableFS.end(), [&part](MFileSystem* fs){ 
            //Serial.printf("calling handles for '%s'\n", fs->symbol);
            return fs->handles(part); 
        } );

        if(foundIter != availableFS.end()) {
Serial.printf("matched fs: %s [%s]\n", (*foundIter)->symbol, path.c_str());
            auto newFile = (*foundIter)->getFile(path);
            newFile->fillPaths(&pathIterator, &begin, &end);

            return newFile;
         }
    };

    //Serial.printf("Little fs fallback\n");

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

MIstream* MFile::inputStream() {
    ; // has to return OPENED stream
    std::shared_ptr<MFile> containerFile(MFSOwner::File(streamPath)); // get the base file that knows how to handle this kind of container, i.e 7z
    std::shared_ptr<MIstream> containerStream(containerFile->inputStream()); // get its base stream, i.e. zip raw file contents

    std::shared_ptr<MIstream> decodedStream(createIStream(containerStream.get())); // wrap this stream into decodec stream, i.e. unpacked zip files

    if(pathInStream != "" && decodedStream->isBrowsable()) {
        // stream is browsable and path was requested, let's skip the stream to requested file
        auto pointedFile = decodedStream->seekNextEntry();

        while (!pointedFile.empty())
        {
            if(pointedFile == this->pathInStream)
                return decodedStream.get();

            pointedFile = decodedStream->seekNextEntry();
        }
        return nullptr; // there was no file with that name in this stream!
    }
    else {
        return nullptr; // path requested for unbrowsable stream
    }

    return decodedStream.get();
};


MFile* MFile::parent(std::string plus) {
    // drop last dir
    // add plus
    int lastSlash = url().find_last_of('/');
    std::string newDir = mstr::dropLast(url(), lastSlash) + "/" + plus;
    return MFSOwner::File(newDir);
};

MFile* MFile::localParent(std::string plus) {
    // drop last dir
    // check if it isn't shorter than streamPath
    // add plus
    int lastSlash = url().find_last_of('/');
    std::string parent = mstr::dropLast(url(), lastSlash);
    if(parent.length()-streamPath.length()>1)
        parent = streamPath;
    return MFSOwner::File(parent+"/"+plus);
};

MFile* MFile::root2(std::string plus) {
    return new LittleFile("/"+plus);
};

MFile* MFile::localRoot(std::string plus) {
    return MFSOwner::File(streamPath+"/"+plus);
};

MFile* MFile::cd(std::string newDir) {
    if(newDir[0]=='/' && newDir[1]=='/') {
        return root2(mstr::drop(newDir,2));
    }
    else if(newDir[0]=='/') {
        return localRoot(mstr::drop(newDir,1));
    }
    else if(newDir[0]=='^') {
        return localParent(mstr::drop(newDir,1));
    }
    if(newDir[0]=='.' && newDir[1]=='.') {
        return localParent(mstr::drop(newDir,3));
    }
    else {
        return MFSOwner::File(url()+"/"+newDir);
    }
};


/********************************************************
 * MStream implementations
 ********************************************************/

MStream::~MStream() {};