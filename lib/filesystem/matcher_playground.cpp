#include <vector>
#include <string>
#include <algorithm>
#include "meat_io.h"
#include "../filesystem/fs_littlefs.h"

using namespace std;

string joinToString(vector<string>::iterator start, vector<string>::iterator end) {
    string res;

    auto i = start;

    while(i != end) {
        res+=(i->data());
        res+="/";
        i++;
    };

    return res;
}

void fillPaths(MFile* newFile, vector<string>::iterator i) {
    // // 	newFile.streamSource = człony[0:i].joinToString("/")
    // joinToString(i->begin(), i);

    // // 	newFile.streamPath = człony[i+1:nrOstatniegoCzłonu].joinToString()  
    // joinToString(i, i->end());
}

MFile* File(vector<MFileSystem*>* fsList, vector<string>* folders) {
//     i = nrOstatniegoCzłonu
    auto i = folders->end();
// człon = człony[i]

    do {
        auto part = i->data();
        // znal = filesystem.firstOrNull { it.macz(człon) }
        auto foundIter=find_if(fsList->begin(), fsList->end(), [&part](MFileSystem* fs){ return fs->handles(part); } );
        //    foundIter->data();
        // if(znall != nullptr) 
        
        if(foundIter != fsList->end()) {
            // 	nowy = znal.makePath(całaścieżka);
            auto newFile = (*foundIter)->getFile("jeifjei");
            fillPaths(newFile, i);
            return newFile;
        }
        // else i--
        else {
            i->back();
        }
    } while (i != folders->begin());

    // if (i==0) then znal = LittleFS
    auto newFile = new LittleFile("weffw");
    fillPaths(newFile, i);

    return newFile;
}

