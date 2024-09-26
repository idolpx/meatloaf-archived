/*
    Copyright (c) 2018 Gurpreet Bal https://github.com/ardyesp/ESPWebDAV
    Copyright (c) 2020 David Gauchard https://github.com/d-a-v/ESPWebDAV
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
    SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.

*/

#include <FS.h>
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
#include <ESP8266WiFi.h>
#include <coredecls.h> // crc32()
#include <PolledTimeout.h>
#define FILENAME(f) f.fileName().c_str()
#define FILEFULLNAME(f) f.fullName()
#define FILESIZE(f) f.fileSize()
#define FILETIME(f) f.fileTime()
#define GETCREATIONTIME(f) f.getCreationTime()
#define FILECREATIONTIME(f) f.fileCreationTime()
#define ISFILE(f) f.isFile()
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include "PolledTimeout_esp32.h"
#include <rom/miniz.h>
const char * FileName(const char * path)
{
    String name = path;
    if (name == "/")return path;
    //path should not end by / if yes need to add a sanity check
    int p = name.lastIndexOf("/");
    if (p == -1) return path;
    return  &path[p + 1];
}
#undef crc32
#define crc32(a, len) mz_crc32( 0xffffffff,(const unsigned char *)a, len)
#define FILENAME(f) FileName(f.name())
#define FILEFULLNAME(f) f.name()
#define FILESIZE(f) f.size()
#define FILETIME(f) f.getLastWrite()
#define GETCREATIONTIME(f) f.getLastWrite()
#define FILECREATIONTIME(f) f.getLastWrite()
#define ISFILE(f) !f.isDirectory()
//in esp32 totalbytes and usedbytes are not part of FS class
//so need to call an helper to address directly SPIFFS/LITTLEFS/SD/etc...
//but due to https://support.microsoft.com/en-us/topic/webdav-mapped-drive-reports-incorrect-drive-capacity-fa101657-7448-1ce6-5999-5bcc59d6a8bd
//this is not used / working
//so let just send 0 but keep helper as comment if in futur it is working
//extern uint64_t TotalBytes();
//extern uint64_t UsedBytes();
#define TotalBytes() 0
#define UsedBytes() 0
#endif //ARDUINO_ARCH_ESP32

#include <time.h>
#include <ESPWebDAV.h>

// define cal constants
const char *months[]  = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *wdays[]  = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

#define ALLOW "PROPPATCH,PROPFIND,OPTIONS,DELETE" SCUNLOCK ",COPY" SCLOCK ",MOVE,HEAD,POST,PUT,GET"

#if WEBDAV_LOCK_SUPPORT
#define SLOCK "LOCK"
#define SCLOCK ",LOCK"
#define SUNLOCK "UNLOCK"
#define SCUNLOCK ",UNLOCK"
#else
#define SLOCK ""
#define SCLOCK ""
#define SUNLOCK ""
#define SCUNLOCK ""
#endif

#define DEBUG_LEN 160

#define PROC "proc" // simple virtual file. TODO XXX real virtual fs with user callbacks


void ESPWebDAVCore::stripSlashes(String& name)
{
    size_t i = 0;
    while (i < name.length())
        if (name[i] == '/' && name.length() > 1 && ((i == name.length() - 1) || name[i + 1] == '/'))
            name.remove(i, 1);
        else
            i++;
}

#if WEBDAV_LOCK_SUPPORT


void ESPWebDAVCore::makeToken(String& ret, uint32_t pash, uint32_t ownash)
{
    char lock_token[17];
    snprintf(lock_token, sizeof(lock_token), "%08x%08x", pash, ownash);
    ret = lock_token;
}


int ESPWebDAVCore::extractLockToken(const String& someHeader, const char* start, const char* end, uint32_t& pash, uint32_t& ownash)
{
    // If: (<46dd353d7e585af1>)
    // =>
    // IfToken: path:0x46dd353d / owner:0x7e585af1

    pash = 0;
    ownash = 0;

    DBG_PRINT("extracting lockToken from '%s'", someHeader.c_str());
    // extract "... <:[lock >
    int startIdx = someHeader.indexOf(start);
    if (startIdx < 0)
    {
        DBG_PRINT("lock: can't find '%s'", start);
        return 412; // fail with precondition failed
    }
    startIdx += strlen(start);
    int endIdx = someHeader.indexOf(end, startIdx);
    if (endIdx < 0)
    {
        DBG_PRINT("lock: can't find '%s'", end);
        return 412; // fail with precondition fail
    }
    DBG_PRINT("found in [%d..%d[ (%d)", startIdx, endIdx, endIdx - startIdx);
    int len = endIdx - startIdx;
    if (len < 1 || len > 16)
    {
        DBG_PRINT("lock: format error (1-16 hex chars)");
        return 423; // fail with lock
    }
    char cp [len + 1];
    memcpy(cp, &(someHeader.c_str()[startIdx]), len);
    cp[len] = 0;
    DBG_PRINT("IfToken: '%s'", cp);
    int ownIdx = std::max(len - 8, 0);
    ownash = strtoul(&cp[ownIdx], nullptr, 16);
    cp[ownIdx] = 0;
    pash = strtoul(cp, nullptr, 16);
    DBG_PRINT("IfToken: path:0x%08x / owner:0x%08x", pash, ownash);
    return 200;
}

#endif // WEBDAV_LOCK_SUPPORT


int ESPWebDAVCore::allowed(const String& uri, uint32_t ownash)
{
#if WEBDAV_LOCK_SUPPORT > 1

    String test = uri;
    while (test.length())
    {
        stripSlashes(test);
        DBG_PRINT("lock: testing '%s'", test.c_str());
        uint32_t hash = crc32(test.c_str(), test.length());
        const auto& lock = _locks.find(hash);
        if (lock != _locks.end())
        {
            DBG_PRINT("lock: found lock, %sowner!", lock->second == ownash ? "" : "not");
            return lock->second == ownash ? 200 : 423;
        }
        int s = test.lastIndexOf('/');
        if (s < 0)
            break;
        test.remove(s);
    }
    DBG_PRINT("lock: none found");
    return 200;

#else

    (void)uri;
    (void)ownash;
    return 200;

#endif
}


int ESPWebDAVCore::allowed(const String& uri, const String& xml /* = emptyString */)
{
    uint32_t hpash, anyownash;
    if (ifHeader.length())
    {
        int code = extractLockToken(ifHeader, "(<", ">", hpash, anyownash);
        if (code != 200)
            return code;
        if (anyownash == 0)
            // malformed
            return 412; // PUT failed with 423 not 412
    }
    else
    {
        int startIdx = xml.indexOf("<owner>");
        int endIdx = xml.indexOf("</owner>");
        anyownash = startIdx > 0 && endIdx > 0 ? crc32(&(xml.c_str()[startIdx + 7]), endIdx - startIdx - 7) : 0;
    }
    return allowed(uri, anyownash);
}


void ESPWebDAVCore::stripName(String& name)
{
    if (name.length() > (size_t)_maxPathLength)
    {
        int dot = name.lastIndexOf('.');
        int newDot = _maxPathLength - (name.length() - dot);
        if (dot <= 0 || newDot < 0)
            name.remove(_maxPathLength);
        else
            name.remove(newDot, dot - newDot);
    }
}


void ESPWebDAVCore::stripHost(String& name)
{
    int remove = name.indexOf(hostHeader);
    if (remove >= 0)
        name.remove(0, remove + hostHeader.length());
}


void ESPWebDAVCore::dir(const String& path, Print* out)
{
    dirAction(path, true, [out](int depth, const String & parent, Dir & entry)->bool
    {
        (void)parent;
        for (int i = 0; i < depth; i++)
            out->print("    ");
        if (entry.isDirectory())
            out->printf("[%s]\n", FILENAME(entry));
        else
            out->printf("%-40s%4dMiB %6dKiB %d\n",
                        FILENAME(entry),
                        ((int)FILESIZE(entry) + (1 << 19)) >> 20,
                        ((int)FILESIZE(entry) + (1 <<  9)) >> 10,
                        (int)FILESIZE(entry));
        return true;
    }, /*false=subdir first*/false);
}


size_t ESPWebDAVCore::makeVirtual(virt_e v, String& internal)
{
    if (v == VIRT_PROC)
    {
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
        internal = ESP.getFullVersion();
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        internal = "SDK:";
        internal += ESP.getSdkVersion();
#endif //ARDUINO_ARCH_ESP32
        internal += '\n';
    }
    return internal.length();
}


ESPWebDAVCore::virt_e ESPWebDAVCore::isVirtual(const String& uri)
{
    const char* n = &(uri.c_str()[0]);
    while (*n && *n == '/')
        n++;
    if (strcmp(n, PROC) == 0)
        return VIRT_PROC;
    return VIRT_NONE;
}


bool ESPWebDAVCore::getPayload(StreamString& payload)
{
    DBG_PRINT("content length=%d", (int)contentLengthHeader);
    payload.clear();
    if (contentLengthHeader > 0)
    {
        payload.reserve(contentLengthHeader);
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
        esp8266::polledTimeout::oneShotFastMs timeout(HTTP_MAX_POST_WAIT);
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        PolledTimeout timeout(HTTP_MAX_POST_WAIT);
#endif //ARDUINO_ARCH_ESP32

        while (payload.length() < (size_t)contentLengthHeader)
        {
            uint8_t buf[16];
            auto n = client->read(buf, std::min((size_t)client->available(), sizeof(buf)));
            if (n <= 0 && timeout)
            {
                DBG_PRINT("get content: short read (%d < %d)",
                          (int)payload.length(), (int)contentLengthHeader);
                return false;
            }
            if (n > 0)
            {
                payload.write(buf, n);
                timeout.reset();
            }
        }
        DBG_PRINT(">>>>>>>>>>> CONTENT:");
        DBG_PRINTSHORT("%s", payload.c_str());
        DBG_PRINTSHORT("\n");
        DBG_PRINT("<<<<<<<<<<< CONTENT");
    }
    return true;
}


bool ESPWebDAVCore::dirAction(const String& path,
                              bool recursive,
                              const std::function<bool(int depth, const String& parent, Dir& entry)>& cb,
                              bool callAfter,
                              int depth)
{
    DBG_PRINT("diraction: scanning dir '%s'", path.c_str());
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
    Dir entry = gfs->openDir(path);
    while (entry.next())
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        File root = gfs->open(path);
    File entry = root.openNextFile();
    while (entry)
#endif //ARDUINO_ARCH_ESP32
    {
        if (!entry.isDirectory())
        {
            DBG_PRINT("diraction: %s/%s (%d B): ", path.c_str(), FILENAME(entry), (int)FILESIZE(entry));
            if (cb(depth, path, entry))
            {
                DBG_PRINT("(file-OK)");
            }
            else
            {
                DBG_PRINT("(file-abort)");
                return false;
            }
        }
#if defined(ARDUINO_ARCH_ESP32)
        entry = root.openNextFile();
#endif //ARDUINO_ARCH_ESP32
    }
    if (recursive)
    {
#if defined(ARDUINO_ARCH_ESP32)
        root = gfs->open(path);
        entry = root.openNextFile();
        while (entry)
#endif //ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
            entry = gfs->openDir(path);
        while (entry.next())
#endif //ARDUINO_ARCH_ESP8266
        {
            if (entry.isDirectory())
            {
                DBG_PRINT("diraction: -------- %s/%s/", path.c_str(), FILENAME(entry));
                if ((callAfter || cb(depth, path, entry))
                        && dirAction(path + '/' + FILENAME(entry), recursive, cb, callAfter, depth + 1)
                        && (!callAfter || cb(depth, path, entry)))
                {
                    DBG_PRINT("(dir-OK)");
                }
                else
                {
                    DBG_PRINT("(dir-abort)");
                    return false;
                }
            }
#if defined(ARDUINO_ARCH_ESP32)
            entry = root.openNextFile();
#endif //ARDUINO_ARCH_ESP32
        }
    }

    return true;
}


void ESPWebDAVCore::handleIssue(int code, const char* text)
{
    String message;
    message.reserve(strlen(text) + uri.length() + method.length() + 32);
    message += text;
    message += "\nURI: ";
    message += uri;
    message += " Method: ";
    message += method;
    message += "\n";

    String err;
    err.reserve(strlen(text) + 32);
    err += code;
    err += ' ';
    err += text;

    DBG_PRINT("Issue:\ntext='%s'", text);
    DBG_PRINT("message='%s'", message.c_str());
    DBG_PRINT("err='%s'", err.c_str());

    send(err, "text/plain", message);
}


void ESPWebDAVCore::handleRequest()
{
    payload.clear();
    replaceFront(uri, _davRoot, _fsRoot);

    ResourceType resource = RESOURCE_NONE;

    // check depth header
    depth = DEPTH_NONE;
    if (depthHeader.length())
    {
        if (depthHeader.equals("1"))
            depth = DEPTH_CHILD;
        else if (depthHeader.equals("infinity"))
            depth = DEPTH_ALL;
        DBG_PRINT("Depth: %d", depth);
    }
    File file;
    if (gfs->exists(uri) || (uri == "/"))
    {
        // does uri refer to a file or directory or a null?
        file = gfs->open(uri, "r");
        if (file)
        {
            resource = file.isDirectory() ? RESOURCE_DIR : RESOURCE_FILE;
            DBG_PRINT("resource: '%s' is %s", uri.c_str(), resource == RESOURCE_DIR ? "dir" : "file");
        }
        else
            DBG_PRINT("resource: '%s': no file nor dir", uri.c_str());
    }
    else
    {
        DBG_PRINT("resource: '%s': not exists", uri.c_str());
    }

    DBG_PRINT("m: %s", method.c_str());
    DBG_PRINT(" r: %d", resource);
    DBG_PRINT(" u: %s", uri.c_str());

    // add header that gets sent everytime
#if WEBDAV_LOCK_SUPPORT
    sendHeader("DAV", "1, 2");
#else
    sendHeader("DAV", "1");
#endif
    sendHeader("Accept-Ranges", "bytes");
    sendHeader("Allow", ALLOW);

    // handle file create/uploads
    if (method.equals("PUT"))
        // payload is managed
        return handlePut(resource);

    // swallow content
    if (!getPayload(payload))
    {
        handleIssue(408, "Request Time-out");
        client->stop();
        return;
    }

    // handle properties
    if (method.equals("PROPFIND"))
        return handleProp(resource, file);

    if (method.equals("GET"))
        return handleGet(resource, file, true);

    if (method.equals("HEAD"))
        return handleGet(resource, file, false);

    // handle options
    if (method.equals("OPTIONS"))
        return handleOptions(resource);

#if WEBDAV_LOCK_SUPPORT
    // handle file locks
    if (method.equals("LOCK"))
        return handleLock(resource);

    if (method.equals("UNLOCK"))
        return handleUnlock(resource);
#endif

    if (method.equals("PROPPATCH"))
        return handlePropPatch(resource, file);

    // directory creation
    if (method.equals("MKCOL"))
        return handleDirectoryCreate(resource);

    // move a file or directory
    if (method.equals("MOVE"))
        return handleMove(resource, file);

    // delete a file or directory
    if (method.equals("DELETE"))
        return handleDelete(resource);

    // delete a file or directory
    if (method.equals("COPY"))
        return handleCopy(resource, file);

    // if reached here, means its a unhandled
    handleIssue(404, "Not found");

    //return false;
}


void ESPWebDAVCore::handleOptions(ResourceType resource)
{
    (void)resource;
    DBG_PRINT("Processing OPTION");

    send("200 OK", NULL, "");
}


#if WEBDAV_LOCK_SUPPORT

void ESPWebDAVCore::handleLock(ResourceType resource)
{
    DBG_PRINT("Processing LOCK");

    // does URI refer to an existing resource
    (void)resource;
    DBG_PRINT("r=%d/%d", resource, RESOURCE_NONE);

#if WEBDAV_LOCK_SUPPORT > 1
    // lock owner
    uint32_t hpash, ownash;
    if (ifHeader.length())
    {
        int code;
        if ((code = extractLockToken(ifHeader, "(<", ">", hpash, ownash)) != 200)
            return handleIssue(code, "Lock error");
    }
    else
    {
        int startIdx, endIdx;
        startIdx = payload.indexOf("<owner>");
        endIdx = payload.indexOf("</owner>");
        ownash = startIdx > 0 && endIdx > 0 ? crc32(&payload[startIdx + 7], endIdx - startIdx - 7) : 0;
    }

    if (!ownash)
    {
        /*  XXXFIXME xml extraction should be improved (on macOS)
            0:10:08.058253: <D:owner>
            0:10:08.058391: <D:href>http://www.apple.com/webdav_fs/</D:href>
            0:10:08.058898: </D:owner>
        */
        ownash = 0xdeadbeef;
    }
    uint32_t pash = crc32(uri.c_str(), uri.length());
    const auto& lock = _locks.find(pash);
    if (lock == _locks.end())
    {
        _locks[pash] = ownash;
    }
    else
    {
        if (lock->second != ownash)
        {
            DBG_PRINT("cannot relock '%s' (owner is 0x%08x)", uri.c_str(), lock->second);
            return handleIssue(423, "Locked");
        }
        DBG_PRINT("owner has relocked");
    }
#else
    const char* lock_token = "0";
#endif

    String lock_token;
    makeToken(lock_token, pash, ownash);
    sendHeader("Lock-Token", lock_token);

#if 1
    String resp;
    resp.reserve(500 + uri.length());
    resp += F("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
              "<D:prop xmlns:D=\"DAV:\">"
              "<D:lockdiscovery>"
              "<D:activelock>"
              "<D:locktoken>"
              "<D:href>");
    resp +=                           lock_token;
    resp += F("</D:href>"
              "</D:locktoken>"
#if 0
              "<D:locktype>"
              "<write/>"
              "</D:locktype>"
              "<D:lockscope>"
              "<exclusive/>"
              "</D:lockscope>"
              "<D:lockroot>"
              "<D:href>");
    resp +=                           uri;
    resp += F("</D:href>"
              "</D:lockroot>"
              "<D:depth>"
              "infinity"
              "</D:depth>");
#if 0
    if (href.length())
    {
        resp += F("<D:owner>"
                  "<a:href xmlns:a=\"DAV:\">");
        resp +=                       href;
        resp += F("</a:href>"
                  "</D:owner>");
    }
#endif
    resp += F("<D:timeout>"
              "Second-3600"
              "</D:timeout>"
#endif
              "</D:activelock>"
              "</D:lockdiscovery>"
              "</D:prop>");
    send("200 OK", "application/xml;charset=utf-8", resp);
#else
    send("200 OK", "application/xml;charset=utf-8", "");
#endif
}


void ESPWebDAVCore::handleUnlock(ResourceType resource)
{
#if WEBDAV_LOCK_SUPPORT > 1
    uint32_t pash = crc32(uri.c_str(), uri.length());

    uint32_t hpash, hownash;
    (void)extractLockToken(lockTokenHeader, "<", ">", hpash, hownash);

    auto lock = _locks.find(pash);
    if (lock == _locks.end())
    {
        DBG_PRINT("wasn't locked: '%s'", uri.c_str());
        return handleIssue(423, "Locked");
    }
    if (lock->second != hownash)
    {
        DBG_PRINT("lock found, bad owner 0x%08x != 0x%08x", hownash, lock->second);
        return handleIssue(423, "Locked");
    }
    _locks.erase(lock);
#endif

    (void)resource;
    DBG_PRINT("Processing UNLOCK");
    send("204 No Content", NULL, "");
}

#endif // WEBDAV_LOCK_SUPPORT


void ESPWebDAVCore::handlePropPatch(ResourceType resource, File& file)
{
    DBG_PRINT("PROPPATCH forwarding to PROPFIND");
    handleProp(resource, file);
}


void ESPWebDAVCore::handleProp(ResourceType resource, File& file)
{
    DBG_PRINT("Processing PROPFIND");
    auto v = isVirtual(uri);

    if (v)
        resource = RESOURCE_FILE;
    // does URI refer to an existing resource
    else if (resource == RESOURCE_NONE)
        return handleIssue(404, "Not found");

    int code;
    if (payload.indexOf("lockdiscovery") < 0 && (code = allowed(uri)) != 200)
        return handleIssue(code, "Locked");

    setContentLength(CONTENT_LENGTH_UNKNOWN);
    send("207 Multi-Status", "application/xml;charset=utf-8", "");
    sendContent(F("<?xml version=\"1.0\" encoding=\"utf-8\"?>"));
    sendContent(F("<D:multistatus xmlns:D=\"DAV:\">"));

    if (v)
    {
        // virtual file
        sendPropResponse(false, uri.c_str(), 1024, time(nullptr), 0);
    }
    else if (ISFILE(file) || depth == DEPTH_NONE)
    {
        DBG_PRINT("----- PROP FILE '%s':", uri.c_str());
        sendPropResponse(file.isDirectory(), uri.c_str(), file.size(), file.getLastWrite(), GETCREATIONTIME(file));
    }
    else
    {
        DBG_PRINT("----- PROP DIR '%s':", uri.c_str());
        sendPropResponse(true, uri, 0, time(nullptr), 0);

#if defined(ARDUINO_ARCH_ESP32)
        File root = gfs->open(uri);
        File entry = root.openNextFile();
        while (entry)
#endif //ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
        Dir entry = gfs->openDir(uri);
        while (entry.next())
#endif //ARDUINO_ARCH_ESP8266
        {
            yield();
            String path;
            path.reserve(uri.length() + 1 + strlen(FILENAME(entry)));
            path += uri;
            path += '/';
            path += FILENAME(entry);
            stripSlashes(path);
            DBG_PRINT("Path: %s", path.c_str());
            sendPropResponse(entry.isDirectory(), path.c_str(), FILESIZE(entry), FILETIME(entry), FILECREATIONTIME(entry));
#if defined(ARDUINO_ARCH_ESP32)
            entry = root.openNextFile();
#endif //ARDUINO_ARCH_ESP32
        }
    }

    if (payload.indexOf(F("quota-available-bytes")) >= 0 ||
            payload.indexOf(F("quota-used-bytes")) >= 0)
    {
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
        fs::FSInfo64 info;
        if (gfs->info64(info))
        {
            sendContentProp(F("quota-available-bytes"), String(1.0 * (info.totalBytes - info.usedBytes), 0));
            sendContentProp(F("quota-used-bytes"), String(1.0 * info.usedBytes, 0));
        }
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        //NEED TO BE not related to SPIFFS
        //use external functions
        //but SPIFFS/FAT size_t because in MB
        //and SD uint64_t because in GB
        //so use uint64_t
        sendContentProp(F("quota-available-bytes"), String(1.0 * (TotalBytes() - UsedBytes()), 0));
        sendContentProp(F("quota-used-bytes"), String(1.0 * UsedBytes(), 0));
#endif //ARDUINO_ARCH_ESP32

    }

    sendContent(F("</D:multistatus>"));
}


void ESPWebDAVCore::sendContentProp(const String& what, const String& response)
{
    String one;
    one.reserve(100 + 2 * what.length() + response.length());
    one += F("<esp:");
    one += what;
    one += F(">");
    one += response;
    one += F("</esp:");
    one += what;
    one += F(">");
    sendContent(one);
}


String ESPWebDAVCore::date2date(time_t date)
{
    // get & convert time to required format
    // Tue, 13 Oct 2015 17:07:35 GMT
    tm* gTm = gmtime(&date);
    char buf[40];
    snprintf(buf, sizeof(buf), "%s, %02d %s %04d %02d:%02d:%02d GMT", wdays[gTm->tm_wday], gTm->tm_mday, months[gTm->tm_mon], gTm->tm_year + 1900, gTm->tm_hour, gTm->tm_min, gTm->tm_sec);
    return buf;
}


void ESPWebDAVCore::sendPropResponse(bool isDir, const String& fullResPathFS, size_t size, time_t lastWrite, time_t creationDate)
{
    String fullResPath = fullResPathFS;
    replaceFront(fullResPath, _fsRoot, _davRoot);
    String displayName = basename(fullResPath.c_str());
    fullResPath = c2enc(fullResPath);

    String blah;
    blah.reserve(100);
    blah += F("<D:response xmlns:esp=\"DAV:\"><D:href>");
    blah += fullResPath;
    blah += F("</D:href><D:propstat><D:status>HTTP/1.1 200 OK</D:status><D:prop>");
    sendContent(blah);

    sendContentProp(F("getlastmodified"), date2date(lastWrite));
    sendContentProp(F("creationdate"), date2date(creationDate));

    DBG_PRINT("-----\nentry: '%s'(dir:%d)\n-----",
              fullResPath.c_str(), isDir);

    if (isDir)
    {
        sendContentProp(F("resourcetype"), F("<D:collection/>"));
    }
    else
    {
        sendContentProp(F("getcontentlength"), String(size));
        sendContentProp(F("getcontenttype"), contentTypeFn(fullResPathFS));

        sendContent("<resourcetype/>");

        char entityTag [uri.length() + 32];
        sprintf(entityTag, "%s%lu", uri.c_str(), (unsigned long)lastWrite);
        uint32_t crc = crc32(entityTag, strlen(entityTag));
        sprintf(entityTag, "\"%08x\"", crc);
        sendContentProp(F("getetag"), entityTag);
    }

    sendContentProp(F("displayname"), displayName);
    Serial.println(displayName);

    sendContent(F("</D:prop></D:propstat></D:response>"));
}


void ESPWebDAVCore::handleGet(ResourceType resource, File& file, bool isGet)
{
    DBG_PRINT("Processing GET (ressource=%d)", (int)resource);

    // does URI refer to an existing file resource
    auto v = isVirtual(uri);
    if (!v)
    {
        if (resource == RESOURCE_DIR)
            return handleIssue(200, "OK");
        if (resource != RESOURCE_FILE)
            return handleIssue(404, "Not found");
    }

    // no lock on GET

#if DBG_WEBDAV
    long tStart = millis();
#endif

    size_t fileSize = file.size();
    String contentType = contentTypeFn(uri);
    if (uri.endsWith(".gz") && contentType != "application/x-gzip" && contentType != "application/octet-stream")
        sendHeader("Content-Encoding", "gzip");

    String internal = emptyString;
    if (v)
    {
        fileSize = makeVirtual(v, internal);
    }
    else if (!fileSize)
    {
        setContentLength(0);
        send("200 OK", contentType.c_str(), "");
        DBG_PRINT("send empty file");
        return;
    }

    char buf[128]; /// XXX use stream::to(): file.to(client);

    // Content-Range: bytes 0-1023/146515
    // Content-Length: 1024

    constexpr bool chunked = false;

    int remaining;
    if (_rangeStart == 0 && (_rangeEnd < 0 || _rangeEnd == (int)fileSize - 1))
    {
        _rangeEnd = fileSize - 1;
        remaining = fileSize;
        setContentLength(chunked ? CONTENT_LENGTH_UNKNOWN : remaining);
        send("200 OK", contentType.c_str(), "");
    }
    else
    {
        if (_rangeEnd == -1 || _rangeEnd >= (int)fileSize)
        {
            _rangeEnd = _rangeStart + (2 * TCP_MSS - 100);
            if (_rangeEnd >= (int)fileSize)
                _rangeEnd = fileSize - 1;
        }
        snprintf(buf, sizeof(buf), "bytes %d-%d/%d", _rangeStart, _rangeEnd, (int)fileSize);
        sendHeader("Content-Range", buf);
        remaining = _rangeEnd - _rangeStart + 1;
        setContentLength(chunked ? CONTENT_LENGTH_UNKNOWN : remaining);
        send("206 Partial Content", contentType.c_str(), "");
    }

    if (isGet && (internal.length() || file.seek(_rangeStart, SeekSet)))
    {
        DBG_PRINT("GET: (%d bytes, chunked=%d, remain=%d)", remaining, chunked, remaining);

        if (internal.length())
        {
            if (transferStatusFn)
                transferStatusFn(file.name(), (100 * _rangeStart) / fileSize, false);
            if ((chunked && !sendContent(&internal.c_str()[_rangeStart], remaining))
                    || (!chunked && client->write(&internal.c_str()[_rangeStart], remaining) != (size_t)remaining))
            {
                DBG_PRINT("file->net short transfer");
            }
            else if (transferStatusFn)
                transferStatusFn(file.name(), (100 * (_rangeStart + remaining)) / fileSize, false);
        }
        else
        {
            if (transferStatusFn)
                transferStatusFn(file.name(), 0, false);
            int percent = 0;

            while (remaining > 0 && file.available())
            {
                size_t toRead = (size_t)remaining > sizeof(buf) ? sizeof(buf) : remaining;
                size_t numRead = file.read((uint8_t*)buf, toRead);
                DBG_PRINT("read %d bytes from file", (int)numRead);

                if ((chunked && !sendContent(buf, numRead))
                        || (!chunked && client->write(buf, numRead) != numRead))
                {
                    DBG_PRINT("file->net short transfer");
                    ///XXXX transmit error ?
                    //return handleWriteRead("Unable to send file content", &file);
                    break;
                }

#if DBG_WEBDAV
                for (size_t i = 0; i < 80 && i < numRead; i++)
                    DBG_PRINTSHORT("%c", buf[i] < 32 || buf[i] > 127 ? '.' : buf[i]);
#endif

                remaining -= numRead;
                if (transferStatusFn)
                {
                    int p = (100 * (file.size() - remaining)) / file.size();
                    if (p != percent)
                    {
                        transferStatusFn(file.name(), percent = p, false);
                    }
                }
                DBG_PRINT("wrote %d bytes to http client", (int)numRead);
            }
        }
    }

    DBG_PRINT("File %zu bytes sent in: %ld sec", fileSize, (millis() - tStart) / 1000);
}


void ESPWebDAVCore::handlePut(ResourceType resource)
{
    DBG_PRINT("Processing Put");

    // does URI refer to a directory
    if (resource == RESOURCE_DIR)
        return handleIssue(404, "Not found");

    int code ;
    if ((code = allowed(uri)) != 200)
        return handleIssue(code, "Lock error");

    // Expect: 100-continue
    if ( continueHeader )
        client->write( "HTTP/1.1 100 Continue\r\n\r\n" );

    File file;
    stripName(uri);
    DBG_PRINT("create file '%s'", uri.c_str());
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
    if (!(file = gfs->open(uri, "w")))
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        String s = uri;
    if (uri[0] != '/')s = "/" + uri;
    DBG_PRINT("Create file %s", s.c_str());
    if (!(file = gfs->open(s, "w")))
#endif //ARDUINO_ARCH_ESP32
    {
        return handleWriteError("Unable to create a new file", file);
    }

    // file is created/open for writing at this point
    // did server send any data in put
    DBG_PRINT("%s - ready for data (%i bytes)", uri.c_str(), (int)contentLengthHeader);

    if (contentLengthHeader != 0)
    {
        uint8_t buf[128];
#if DBG_WEBDAV
        long tStart = millis();
#endif
        size_t numRemaining = contentLengthHeader;

        if (transferStatusFn)
            transferStatusFn(file.name(), 0, true);
        int percent = 0;

        // read data from stream and write to the file
        while (numRemaining > 0)
        {
            size_t numToRead = numRemaining;
            if (numToRead > sizeof(buf))
                numToRead = sizeof(buf);
            auto numRead = readBytesWithTimeout(buf, numToRead);
            if (numRead == 0)
                break;

            size_t written = 0;
            while (written < numRead)
            {
                auto numWrite = file.write(buf + written, numRead - written);
                if (numWrite == 0 || (int)numWrite == -1)
                {
                    DBG_PRINT("error: numread=%d write=%d written=%d", (int)numRead, (int)numWrite, (int)written);
                    return handleWriteError("Write data failed", file);
                }
                written += numWrite;
            }

            // reduce the number outstanding
            numRemaining -= numRead;
            if (transferStatusFn)
            {
                int p = (100 * (contentLengthHeader - numRemaining)) / contentLengthHeader;
                if (p != percent)
                {
                    transferStatusFn(file.name(), percent = p, true);
                }
            }
        }

        // detect timeout condition
        if (numRemaining)
            return handleWriteError("Timed out waiting for data", file);

        DBG_PRINT("File %zu bytes stored in: %ld sec", (contentLengthHeader - numRemaining), ((millis() - tStart) / 1000));
    }

    DBG_PRINT("file written ('%s': %d = %d bytes)", String(file.name()).c_str(), (int)contentLengthHeader, (int)file.size());

    if (resource == RESOURCE_NONE)
        send("201 Created", NULL, "");
    else
        send("200 OK", NULL, "");
}


void ESPWebDAVCore::handleWriteError(const String& message, File& file)
{
    // close this file
    file.close();
    // delete the wrile being written
    gfs->remove(uri);
    // send error
    send("500 Internal Server Error", "text/plain", message);
    DBG_PRINT("%s", message.c_str());
}


void ESPWebDAVCore::handleDirectoryCreate(ResourceType resource)
{
    DBG_PRINT("Processing MKCOL (r=%d uri='%s' cl=%d)", (int)resource, uri.c_str(), (int)contentLengthHeader);

    if (contentLengthHeader)
        return handleIssue(415, "Unsupported Media Type");

    // does URI refer to anything
    if (resource != RESOURCE_NONE)
        return handleIssue(405, "Not allowed");

    int parentLastIndex = uri.lastIndexOf('/');
    if (parentLastIndex > 0)
    {
        File testParent = gfs->open(uri.substring(0, parentLastIndex), "r");
        if (!testParent.isDirectory())
            return handleIssue(409, "Conflict");
    }

    if (!gfs->mkdir(uri))
    {
        // send error
        send("500 Internal Server Error", "text/plain", "Unable to create directory");
        DBG_PRINT("Unable to create directory");
        return;
    }

    DBG_PRINT("%s directory created", uri.c_str());
    send("201 Created", NULL, "");
}


String ESPWebDAVCore::urlToUri(const String& url)
{
    int index;
    if (url.startsWith("http") && (index = url.indexOf("://")) <= 5)
    {
        int uriStart = url.indexOf('/', index + 3);
        return url.substring(uriStart);
    }
    return url;
}

void ESPWebDAVCore::replaceFront (String& str, const String& from, const String& to)
{
    if (from.length() && to.length() && str.indexOf(from) == 0)
    {
        DBG_PRINT("replaceFront(%s, %s): %s -> ", from.c_str(), to.c_str(), str.c_str());
        String repl;
        repl.reserve(str.length() + to.length() - from.length() + 1);
        repl = to;
        size_t skip = from.length() == 1? 0: from.length();
        repl += str.c_str() + skip;
        str = repl;
        stripSlashes(str);
        DBG_PRINT("%s\n", str.c_str());
    }
}

void ESPWebDAVCore::handleMove(ResourceType resource, File& src)
{
    const char* successCode = "201 Created";

    DBG_PRINT("Processing MOVE");

    // does URI refer to anything
    if (resource == RESOURCE_NONE
            || destinationHeader.length() == 0)
    {
        return handleIssue(404, "Not found");
    }

    String dest = enc2c(urlToUri(destinationHeader));
    stripHost(dest);
    stripSlashes(dest);
    stripName(dest);
    replaceFront(dest, _davRoot, _fsRoot);

    DBG_PRINT("Move destination: %s", dest.c_str());

    int code;
    if ((code = allowed(uri)) != 200 || (code = allowed(dest)) != 200)
        return handleIssue(code, "Locked");

    File destFile;
    if (gfs->exists(dest) || (dest == "/")) destFile = gfs->open(dest, "r");
    if (destFile && !ISFILE(destFile))
    {
        dest += '/';
        dest += src.name();
        stripSlashes(dest);
        stripName(dest);
        destFile.close();
        destFile = gfs->open(dest, "r");
        successCode = "204 No Content"; // MOVE to existing collection resource didn't give 204
    }

    if (destFile)
    {
        if (overwrite.equalsIgnoreCase("F"))
            return handleIssue(412, "Precondition Failed");
        if (destFile.isDirectory())
        {
            destFile.close();
            deleteDir(dest);
        }
        else
        {
            destFile.close();
            gfs->remove(dest);
        }
    }

    src.close();

    DBG_PRINT("finally rename '%s' -> '%s'", uri.c_str(), dest.c_str());

    if (!gfs->rename(uri, dest))
    {
        // send error
        send("500 Internal Server Error", "text/plain", "Unable to move");
        DBG_PRINT("Unable to move file/directory");
        return;
    }

    DBG_PRINT("Move successful");
    send(successCode, NULL, "");
}


bool ESPWebDAVCore::mkFullDir(String fullDir)
{
    bool ret = true;
    stripSlashes(fullDir);

    int idx = 0;
    while (idx != -1)
    {
        idx = fullDir.indexOf('/', idx + 1);
        String part = idx == -1? /*last part*/fullDir: fullDir.substring(0, idx);
        ret = gfs->mkdir(part); // might already exist, keeping on
    }
    return ret; // return last action success
}


bool ESPWebDAVCore::deleteDir(const String& dir)
{
    // delete content of directory
    dirAction(dir, true, [this](int depth, const String & parent, Dir & entry)->bool
    {
        (void)depth;
        String toRemove;
        toRemove.reserve(parent.length() + strlen(FILENAME(entry)) + 2);
        toRemove += parent;
        toRemove += '/';
        toRemove += FILENAME(entry);
        bool ok = !!(entry.isDirectory() ? gfs->rmdir(toRemove) : gfs->remove(toRemove));
        DBG_PRINT("DELETE %s %s: %s", entry.isDirectory() ? "[ dir]" : "[file]", toRemove.c_str(), ok ? "ok" : "bad");
        return ok;
    });

    DBG_PRINT("Delete dir '%s'", dir.c_str());
    gfs->rmdir(dir);

    return true;
}

void ESPWebDAVCore::handleDelete(ResourceType resource)
{
    DBG_PRINT("Processing DELETE '%s'", uri.c_str());

    // does URI refer to anything
    if (resource == RESOURCE_NONE)
        return handleIssue(404, "Not found");

    int code;
    if ((code = allowed(uri)) != 200)
        return handleIssue(code, "Locked");

    bool retVal;
    if (resource == RESOURCE_FILE)
        retVal = gfs->remove(uri);
    else
        retVal = deleteDir(uri);

    DBG_PRINT("handleDelete: uri='%s' ress=%s ret=%d\n", uri.c_str(), resource == RESOURCE_FILE?"file":"dir", retVal);
    // for some reason, parent dir can be removed if empty
    // need to leave it there (also to pass compliance tests).
    int parentIdx = uri.lastIndexOf('/');
    if (parentIdx >= 0)
    {
        uri.remove(parentIdx);
        if (uri.length())
        {
            DBG_PRINT("Recreating directory '%s'\n", uri.c_str());
            if (!mkFullDir(uri))
            {
                DBG_PRINT("Error recreating directory '%s'\n", uri.c_str());
            }
        }
    }

    if (!retVal)
    {
        // send error
        send("500 Internal Server Error", "text/plain", "Unable to delete");
        DBG_PRINT("Unable to delete file/directory");
        return;
    }

    DBG_PRINT("Delete successful");
    send("200 OK", NULL, "");
}


bool ESPWebDAVCore::copyFile(File srcFile, const String& destName)
{
    File dest;
    if (overwrite.equalsIgnoreCase("F"))
    {
        dest = gfs->open(destName, "r");
        if (dest)
        {
            DBG_PRINT("copy dest '%s' already exists and overwrite is false", destName.c_str());
            handleIssue(412, "Precondition Failed");
            return false;
        }
    }
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
    dest = gfs->open(destName, "w");
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    String s = destName;
    if (destName[0] != '/')s = "/" + destName;
    dest = gfs->open(s, "w");
    DBG_PRINT("Create file %s", s.c_str());
#endif //ARDUINO_ARCH_ESP32
    if (!dest)
    {
        handleIssue(413, "Request Entity Too Large");
        return false;
    }
    while (srcFile.available())
    {
        ///XXX USE STREAMTO
        yield();
        char cp[128];
        int nb = srcFile.read((uint8_t*)cp, sizeof(cp));
        if (!nb)
        {
            DBG_PRINT("copy: short read");
            handleIssue(500, "Internal Server Error");
            return false;
        }
        int wr = dest.write((const uint8_t*)cp, nb);
        if (wr != nb)
        {
            DBG_PRINT("copy: short write wr=%d != rd=%d", (int)wr, (int)nb);
            handleIssue(500, "Internal Server Error");
            return false;
        }
    }
    dest.close();
    return true;
}


void ESPWebDAVCore::handleCopy(ResourceType resource, File& src)
{
    const char* successCode = "201 Created";

    DBG_PRINT("Processing COPY");

    if (resource == RESOURCE_NONE)
        return handleIssue(404, "Not found");

    if (!src) // || resource != RESOURCE_FILE)
        return handleIssue(413, "Request Entity Too Large");

    String destParentPath = destinationHeader;
    {
        int j = -1;
        for (int i = 0; i < 3; i++)
            j = destParentPath.indexOf('/', j + 1);
        destParentPath.remove(0, j);
    }

    String destPath = destParentPath;
    if (destPath.length())
    {
        if (destPath[destPath.length() - 1] == '/')
        {
            // add file name
            destPath += src.name();
            successCode = "204 No Content"; // COPY to existing resource should give 204 (RFC2518:S8.8.5)
        }
        else
        {
            // remove last part
            int lastSlash = destParentPath.lastIndexOf('/');
            if (lastSlash > 0)
                destParentPath.remove(lastSlash);
        }
    }
    replaceFront(destPath, _davRoot, _fsRoot);
    replaceFront(destParentPath, _davRoot, _fsRoot);

    DBG_PRINT("copy: src='%s'=>'%s' dest='%s'=>'%s' parent:'%s'",
              uri.c_str(), FILEFULLNAME(src),
              destinationHeader.c_str(), destPath.c_str(),
              destParentPath.c_str());
    File destParent = gfs->open(destParentPath, "r");

    stripName(destPath);
    int code;
    if (/*(code = allowed(uri)) != 200 ||*/ (code = allowed(destParentPath)) != 200 || (code = allowed(destPath)) != 200)
        return handleIssue(code, "Locked");

    // copy directory
    if (src.isDirectory())
    {
        DBG_PRINT("Source is directory");
        if (ISFILE(destParent))
        {
            DBG_PRINT("'%s' is not a directory", destParentPath.c_str());
            return handleIssue(409, "Conflict");
        }

        if (!dirAction(FILEFULLNAME(src), depth == DEPTH_ALL, [this, destParentPath](int depth, const String & parent, Dir & source)->bool
    {
        (void)depth;
            (void)parent;
            String destNameX = destParentPath + '/';
            destNameX += FILENAME(source);
            stripName(destNameX);
            DBG_PRINT("COPY: '%s' -> '%s", FILENAME(source), destNameX.c_str());
            return copyFile(gfs->open(FILENAME(source), "r"), destNameX);
        }))
        {
            return; // handleIssue already called by failed copyFile() handleIssue(409, "Conflict");
        }
    }
    else
    {
        DBG_PRINT("Source is file");

        // (COPY into non-existant collection '/litmus/nonesuch' succeeded)
        if (!destParent || !destParent.isDirectory())
        {
            DBG_PRINT("dest dir '%s' not existing", destParentPath.c_str());
            return handleIssue(409, "Conflict");
        }

        // copy file

        if (!copyFile(src, destPath))
            return;
    }

    DBG_PRINT("COPY successful");
    send(successCode, NULL, "");
}


void ESPWebDAVCore::_prepareHeader(String& response, const String& code, const char* content_type, size_t contentLength)
{
    response = "HTTP/1.1 " + code + "\r\n";

    if (content_type)
        sendHeader("Content-Type", content_type, true);

    if ((size_t)_contentLengthAnswer == CONTENT_LENGTH_NOT_SET)
        sendHeader("Content-Length", String(contentLength));
    else if ((size_t)_contentLengthAnswer != CONTENT_LENGTH_UNKNOWN)
        sendHeader("Content-Length", String(_contentLengthAnswer));
    else //if ((size_t)_contentLengthAnswer == CONTENT_LENGTH_UNKNOWN)
    {
        _chunked = true;
        //sendHeader("Accept-Ranges", "none");
        sendHeader("Transfer-Encoding", "chunked");
    }
    if (m_persistent)
        sendHeader("Connection", "keep-alive");
    else
        sendHeader("Connection", "close");

    response += _responseHeaders;
    response += "\r\n";
}


bool ESPWebDAVCore::parseRequest(const String& givenMethod,
                                 const String& givenUri,
                                 WiFiClient* givenClient,
                                 ContentTypeFunction givenContentTypeFn)
{
    method = givenMethod;
    uri = enc2c(givenUri);
    stripSlashes(uri);
    client = givenClient;
    contentTypeFn = givenContentTypeFn;

    DBG_PRINT("############################################");
    DBG_PRINT(">>>>>>>>>> RECV");

    DBG_PRINT("method: %s", method.c_str());
    DBG_PRINT(" url: %s", uri.c_str());

    // parse and finish all headers
    String headerName;
    String headerValue;
    _rangeStart = 0;
    _rangeEnd = -1;

    DBG_PRINT("INPUT");
    // no new client is waiting, allow more time to current client
    m_persistent_timer_ms = millis();

    m_persistent = ((millis() - m_persistent_timer_ms) < m_persistent_timer_init_ms);

    // reset all variables
    _chunked = false;
    _responseHeaders.clear();
    _contentLengthAnswer = (int)CONTENT_LENGTH_NOT_SET;
    contentLengthHeader = 0;
    depthHeader.clear();
    hostHeader.clear();
    destinationHeader.clear();
    overwrite.clear();
    ifHeader.clear();
    lockTokenHeader.clear();

    while (1)
    {
        String req = client->readStringUntil('\r');
        client->readStringUntil('\n');
        if (req == "")
            // no more headers
            break;

        int headerDiv = req.indexOf(':');
        if (headerDiv == -1)
            break;

        headerName = req.substring(0, headerDiv);
        headerValue = req.substring(headerDiv + 2);
        DBG_PRINT("\t%s: %s", headerName.c_str(), headerValue.c_str());

        if (headerName.equalsIgnoreCase("Host"))
            hostHeader = headerValue;
        else if (headerName.equalsIgnoreCase("Depth"))
            depthHeader = headerValue;
        else if (headerName.equalsIgnoreCase("Content-Length"))
            contentLengthHeader = headerValue.toInt();
        else if (headerName.equalsIgnoreCase("Destination"))
            destinationHeader = headerValue;
        else if (headerName.equalsIgnoreCase("Expect"))
            continueHeader = true;
        else if (headerName.equalsIgnoreCase("Range"))
            processRange(headerValue);
        else if (headerName.equalsIgnoreCase("Overwrite"))
            overwrite = headerValue;
        else if (headerName.equalsIgnoreCase("If"))
            ifHeader = headerValue;
        else if (headerName.equalsIgnoreCase("Lock-Token"))
            lockTokenHeader = headerValue;
    }
    DBG_PRINT("<<<<<<<<<< RECV");

    bool ret = true;
    /*ret =*/ handleRequest();

    // finalize the response
    if (_chunked)
        sendContent("");

    return ret;
}


size_t ESPWebDAVCore::readBytesWithTimeout(uint8_t *buf, size_t size)
{
    size_t where = 0;

    while (where < size)
    {
        int timeout_ms = HTTP_MAX_POST_WAIT;
        while (!client->available() && client->connected() && timeout_ms--)
            delay(1);

        if (!client->available())
            break;

        where += client->read(buf + where, size - where);
    }

    return where;
}


void ESPWebDAVCore::sendHeader(const String& name, const String& value, bool first)
{
    String headerLine = name + ": " + value + "\r\n";

    if (first)
        _responseHeaders = headerLine + _responseHeaders;
    else
        _responseHeaders += headerLine;
}


void ESPWebDAVCore::send(const String& code, const char* content_type, const String& content)
{
    String header;
    _prepareHeader(header, code, content_type, content.length());

    client->write(header.c_str(), header.length());

    //DBG_PRINT(">>>>>>>>>> SENT");
    //DBG_PRINT("---- header: \n%s", header.c_str());

    if (content.length())
    {
        sendContent(content);
#if DBG_WEBDAV
        DBG_PRINT("send content (%d bytes):", (int)content.length());
        for (size_t i = 0; i < DEBUG_LEN && i < content.length(); i++)
            DBG_PRINTSHORT("%c", content[i] < 32 || content[i] > 127 ? '.' : content[i]);
        if (content.length() > DEBUG_LEN) DBG_PRINTSHORT("...");
        DBG_PRINTSHORT("\n");
#endif
    }
    //DBG_PRINT("<<<<<<<<<< SENT");
}


bool ESPWebDAVCore::sendContent(const String& content)
{
    return sendContent(content.c_str(), content.length());
}


bool ESPWebDAVCore::sendContent(const char* data, size_t size)
{
    if (_chunked)
    {
        char chunkSize[32];
        snprintf(chunkSize, sizeof(chunkSize), "%x\r\n", (int)size);
        size_t l = strlen(chunkSize);
        if (client->write(chunkSize, l) != l)
            return false;
        DBG_PRINT("---- chunk %s", chunkSize);
    }

#if DBG_WEBDAV
    DBG_PRINT("---- %scontent (%d bytes):", _chunked ? "chunked " : "", (int)size);
    for (size_t i = 0; i < DEBUG_LEN && i < size; i++)
        DBG_PRINTSHORT("%c", data[i] < 32 || data[i] > 127 ? '.' : data[i]);
    if (size > DEBUG_LEN) DBG_PRINTSHORT("...");
    DBG_PRINTSHORT("\n");
#endif

    if (client->write(data, size) != size)
    {
        DBG_PRINT("SHORT WRITE");
        return false;
    }

    if (_chunked)
    {
        if (client->write("\r\n", 2) != 2)
        {
            DBG_PRINT("SHORT WRITE 2");
            return false;
        }
        if (size == 0)
        {
            DBG_PRINT("END OF CHUNKS");
            _chunked = false;
        }
    }

    DBG_PRINT("OK with sendContent");
    return true;
}


bool  ESPWebDAVCore::sendContent_P(PGM_P content)
{
    const char * footer = "\r\n";
    size_t size = strlen_P(content);

    if (_chunked)
    {
        char chunkSize[32];
        snprintf(chunkSize, sizeof(chunkSize), "%x%s", (int)size, footer);
        size_t l = strlen(chunkSize);
        if (client->write(chunkSize, l) != l)
            return false;
    }

    if (client->write_P(content, size) != size)
    {
        DBG_PRINT("SHORT WRITE");
        return false;
    }

    if (_chunked)
    {
        if (client->write(footer, 2) != 2)
        {
            DBG_PRINT("SHORT WRITE 2");
            return false;
        }
        if (size == 0)
        {
            DBG_PRINT("END OF CHUNKS");
            _chunked = false;
        }
    }

    DBG_PRINT("OK with sendContent_P");
    return true;
}


void ESPWebDAVCore::setContentLength(size_t len)
{
    _contentLengthAnswer = len;
}


void ESPWebDAVCore::processRange(const String& range)
{
    // "Range": "bytes=0-5"
    // "Range": "bytes=0-"

    size_t i = 0;
    while (i < range.length() && (range[i] < '0' || range[i] > '9'))
        i++;
    size_t j = i;
    while (j < range.length() && range[j] >= '0' && range[j] <= '9')
        j++;
    if (j > i)
    {
        _rangeStart = atoi(&range.c_str()[i]);
        if (range.c_str()[j + 1])
            _rangeEnd = atoi(&range.c_str()[j + 1]);
        else
            _rangeEnd = -1;
    }
    DBG_PRINT("Range: %d -> %d", _rangeStart, _rangeEnd);
}


int ESPWebDAVCore::htoi(char c)
{
    c = tolower(c);
    return c >= '0' && c <= '9' ? c - '0' :
           c >= 'a' && c <= 'f' ? c - 'a' + 10 :
           -1;
}


char ESPWebDAVCore::itoH(int c)
{
    return c <= 9 ? c + '0' : c - 10 + 'A';
}


int ESPWebDAVCore::hhtoi(const char* c)
{
    int h = htoi(*c);
    int l = htoi(*(c + 1));
    return h < 0 || l < 0 ? -1 : (h << 4) + l;
}


String ESPWebDAVCore::enc2c(const String& encoded)
{
    String ret = encoded;
    for (size_t i = 0; ret.length() >= 2 && i < ret.length() - 2; i++)
        if (ret[i] == '%')
        {
            int v = hhtoi(ret.c_str() + i + 1);
            if (v > 0)
            {
                ret[i] = v < 128 ? (char)v : '=';
                ret.remove(i + 1, 2);
            }
        }
    return ret;
}


String ESPWebDAVCore::c2enc(const String& decoded)
{
    size_t l = decoded.length();
    for (size_t i = 0; i < decoded.length(); i++)
        if (!isalnum(decoded[i]))
            l += 2;
    String ret;
    ret.reserve(l);
    for (size_t i = 0; i < decoded.length(); i++)
    {
        char c = decoded[i];
        if (isalnum(c) || c == '.' || c == '-' || c == '_')
            ret += c;
        else
        {
            ret += '%';
            ret += itoH(decoded[i] >> 4);
            ret += itoH(decoded[i] & 0xf);
        }
    }
    return ret;
}
