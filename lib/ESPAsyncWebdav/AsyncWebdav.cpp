#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DateTime.h>
#include <Hash.h>
#include "AsyncWebdav.h"

AsyncWebdav::AsyncWebdav(const String& url){
    this->_url = url;
}


bool AsyncWebdav::canHandle(AsyncWebServerRequest *request){
    if(request->url().startsWith(this->_url)){
        if(request->method() == HTTP_PROPFIND 
            || request->method() == HTTP_PROPPATCH){
            request->addInterestingHeader("depth");
            return true;
        }
        if(request->method() == HTTP_MOVE){
            request->addInterestingHeader("destination");
            return true;
        }
        if(request->method() == HTTP_GET 
            || request->method() == HTTP_HEAD 
            || request->method() == HTTP_OPTIONS 
            || request->method() == HTTP_PUT 
            || request->method() == HTTP_LOCK 
            || request->method() == HTTP_UNLOCK 
            || request->method() == HTTP_MKCOL 
            || request->method() == HTTP_DELETE 
            || request->method() == HTTP_COPY){
            return true;
        }
    }
    return false;
}

void AsyncWebdav::handleRequest(AsyncWebServerRequest *request){
    // parse the url to a proper path
    String path = request->url().substring(this->_url.length());
    if(path.isEmpty()){
        path = "/";
    }
    if(!path.equals("/") && path.endsWith("/")){
        path = path.substring(0, path.length() - 1);
    }

    // check resource type on local storage
    DavResourceType resource = DAV_RESOURCE_NONE;
    File baseFile = LittleFS.open(path, "r");
    if(baseFile){
        resource = baseFile.isDirectory() ? DAV_RESOURCE_DIR : DAV_RESOURCE_FILE;
        baseFile.close();
    }

    // route the request
    if(request->method() == HTTP_PROPFIND || request->method() == HTTP_PROPPATCH){
        return handlePropfind(path, resource, request);
    }
    if(request->method() == HTTP_GET){
        return handleGet(path, resource, request);
    }
    if(request->method() == HTTP_HEAD || request->method() == HTTP_OPTIONS){
        return handleHead(resource, request);
    }
    if(request->method() == HTTP_PUT){
        if(LittleFS.exists(path)){
            return request->send(200);
        }else{
            File f = LittleFS.open(path, "a");
            if(f){
                f.close();
                return request->send(201);
            }else{
                return request->send(500);
            }
        }
    }
    if(request->method() == HTTP_LOCK){
        return handleLock(path, resource, request);
    }
    if(request->method() == HTTP_UNLOCK){
        return handleUnlock(path, resource, request);
    }
    if(request->method() == HTTP_MKCOL){
        return handleMkcol(path, resource, request);
    }
    if(request->method() == HTTP_MOVE){
        return handleMove(path, resource, request);
    }
    if(request->method() == HTTP_DELETE){
        return handleDelete(path, resource, request);
    }

    return handleNotFound(request);
}

void AsyncWebdav::handleBody(AsyncWebServerRequest *request, unsigned char *data, size_t len, size_t index, size_t total){
    // parse the url to a proper path
    String path = request->url().substring(this->_url.length());
    if(path.isEmpty()){
        path = "/";
    }
    if(!path.equals("/") && path.endsWith("/")){
        path = path.substring(0, path.length() - 1);
    }

    // check resource type on local storage
    DavResourceType resource = DAV_RESOURCE_NONE;
    File baseFile = LittleFS.open(path, "r");
    if(baseFile){
        resource = baseFile.isDirectory() ? DAV_RESOURCE_DIR : DAV_RESOURCE_FILE;
        baseFile.close();
    }

    // route the request
    if(request->method() == HTTP_PUT){
        return handlePut(path, resource, request, data, len, index, total);
    }
}

void AsyncWebdav::handlePropfind(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    // check whether file or dir exists
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    // check depth header
    DavDepthType depth = DAV_DEPTH_NONE;
    AsyncWebHeader* depthHeader = request->getHeader("Depth");
    if(depthHeader){
        if(depthHeader->value().equals("1")){
            depth = DAV_DEPTH_CHILD;
        } else if(depthHeader->value().equals("infinity")){
            depth = DAV_DEPTH_ALL;
        }
    }

    // prepare response
    File baseFile = LittleFS.open(path, "r");
    AsyncResponseStream *response = request->beginResponseStream("application/xml");
    response->setCode(207);

    response->print("<?xml version=\"1.0\"?>");
    response->print("<d:multistatus xmlns:d=\"DAV:\">");
    sendPropResponse(response, false, &baseFile);
    if(resource == DAV_RESOURCE_DIR && depth == DAV_DEPTH_CHILD){
        Dir dir = LittleFS.openDir(path);
        File childFile;
        while(dir.next()){
            childFile = dir.openFile("r");
            sendPropResponse(response, true, &childFile);
            childFile.close();
        }
    }
    response->print("</d:multistatus>");

    baseFile.close();
    return request->send(response);
}

void AsyncWebdav::handleGet(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    if(resource != DAV_RESOURCE_FILE){
        return handleNotFound(request);
    }

    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path);
    response->addHeader("Allow", "PROPFIND,OPTIONS,DELETE,COPY,MOVE,HEAD,POST,PUT,GET");
    request->send(response);
}

void AsyncWebdav::handlePut(const String& path, DavResourceType resource, AsyncWebServerRequest * request, unsigned char *data, size_t len, size_t index, size_t total){
    if(resource == DAV_RESOURCE_DIR){
        return handleNotFound(request);
    }

    File file;
    if(!index){
        file = LittleFS.open(path, "w");
    }else{
        file = LittleFS.open(path, "a");
    }
    file.write(data, len);
    file.close();
}

void AsyncWebdav::handleLock(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    AsyncResponseStream *response = request->beginResponseStream("application/xml; charset=utf-8");
    response->setCode(200);
    response->addHeader("Allow", "PROPPATCH,PROPFIND,OPTIONS,DELETE,UNLOCK,COPY,LOCK,MOVE,HEAD,POST,PUT,GET");
    response->addHeader("Lock-Token", "urn:uuid:26e57cb3-834d-191a-00de-000042bdecf9");

    response->print("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    response->print("<D:prop xmlns:D=\"DAV:\">");
    response->print("<D:lockdiscovery>");
    response->print("<D:activelock>");
    response->print("<D:locktype><write/></D:locktype>");
    response->print("<D:lockscope><exclusive/></D:lockscope>");
    response->print("<D:locktoken><D:href>urn:uuid:26e57cb3-834d-191a-00de-000042bdecf9</D:href></D:locktoken>");
    response->printf("<D:lockroot><D:href>%s</D:href></D:lockroot>", (String("http://") + request->host() + "/dav" + path).c_str());
    response->print("<D:depth>infinity</D:depth>");
    response->printf("<D:owner><a:href xmlns:a=\"DAV:\">%s</a:href></D:owner>", "todo");
    response->print("<D:timeout>Second-3600</D:timeout>");
    response->print("</D:activelock>");
    response->print("</D:lockdiscovery>");
    response->print("</D:prop>");

    request->send(response);
}

void AsyncWebdav::handleUnlock(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    AsyncWebServerResponse *response = request->beginResponse(200);
    response->addHeader("Allow", "PROPPATCH,PROPFIND,OPTIONS,DELETE,UNLOCK,COPY,LOCK,MOVE,HEAD,POST,PUT,GET");
    response->addHeader("Lock-Token", "urn:uuid:26e57cb3-834d-191a-00de-000042bdecf9");
    request->send(response);
}

void AsyncWebdav::handleMkcol(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    // does the file/dir already exist?
    int status;
    if(resource != DAV_RESOURCE_NONE){
        status = 405;
    }else{
        // create dir and send response
        if(LittleFS.mkdir(path)){
            status = 201;
        }else{
            status = 405;
        }
    }
    request->send(status);
}

void AsyncWebdav::handleMove(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    AsyncWebHeader* destinationHeader = request->getHeader("destination");
    if(!destinationHeader || destinationHeader->value().isEmpty()){
        return handleNotFound(request);
    }

    AsyncWebServerResponse *response;
    if(LittleFS.rename(path, urlToUri(destinationHeader->value()))){
        response = request->beginResponse(201);
        response->addHeader("Allow", "OPTIONS,MKCOL,LOCK,POST,PUT");
    }else{
        response = request->beginResponse(500, "text/plain", "Unable to move");
    }

    request->send(response);
}

void AsyncWebdav::handleDelete(const String& path, DavResourceType resource, AsyncWebServerRequest * request){
    // does the file/dir exist?
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    // delete file or dir
    bool result;
    if(resource == DAV_RESOURCE_FILE){
        result = LittleFS.remove(path);
    }else{
        result = LittleFS.rmdir(path);
    }

    // check for error
    AsyncWebServerResponse *response;
    if(result)	{
        response = request->beginResponse(200);
        response->addHeader("Allow", "OPTIONS,MKCOL,LOCK,POST,PUT");
    }else{
        response = request->beginResponse(201);
    }

    request->send(response);
}

void AsyncWebdav::handleHead(DavResourceType resource, AsyncWebServerRequest * request){
    if(resource == DAV_RESOURCE_NONE){
        return handleNotFound(request);
    }

    AsyncWebServerResponse *response = request->beginResponse(200);
    if(resource == DAV_RESOURCE_FILE){
        response->addHeader("Allow", "PROPFIND,OPTIONS,DELETE,COPY,MOVE,HEAD,POST,PUT,GET");
    }
    if(resource == DAV_RESOURCE_DIR){
        response->addHeader("Allow", "PROPFIND,OPTIONS,DELETE,COPY,MOVE");
    }
    request->send(response);
}

void AsyncWebdav::handleNotFound(AsyncWebServerRequest * request){
    AsyncWebServerResponse *response = request->beginResponse(404);
    response->addHeader("Allow", "OPTIONS,MKCOL,POST,PUT");
    request->send(response);
}

String AsyncWebdav::urlToUri(String url){
    if(url.startsWith("http://")){
        int uriStart = url.indexOf('/', 7);
        return url.substring(uriStart + 4);
    }else if(url.startsWith("https://")){
        int uriStart = url.indexOf('/', 8);
        return url.substring(uriStart + 4);
    }else{
        return url.substring(this->_url.length());
    }
}

void AsyncWebdav::sendPropResponse(AsyncResponseStream *response, boolean recursing, File *curFile){
    String fullPath = curFile->fullName();
    if(fullPath.substring(0, 1) != "/"){
        fullPath = String("/") + fullPath;
    }
    if(curFile->isDirectory() && fullPath.substring(fullPath.length() - 1, fullPath.length()) != "/"){
        fullPath += "/";
    }
    fullPath = this->_url + fullPath;
    fullPath.replace(" ", "%20");

    // get file modified time
    time_t lastWrite = curFile->getLastWrite();
    DateTimeClass dt(lastWrite);
    String fileTimeStamp = dt.format("%a, %d %b %Y %H:%M:%S GMT");

    // load fs info
    FSInfo fsInfo;
    LittleFS.info(fsInfo);

    // send response
    response->print("<d:response>");
    response->printf("<d:href>%s</d:href>", fullPath.c_str());
    response->print("<d:propstat>");
    response->print("<d:prop>");
    
    // last modified
    response->printf("<d:getlastmodified>%s</d:getlastmodified>", fileTimeStamp.c_str());
    
    // quota
    response->printf("<d:quota-used-bytes>%d</d:quota-used-bytes>", fsInfo.usedBytes);
    response->printf("<d:quota-available-bytes>%d</d:quota-available-bytes>", fsInfo.totalBytes - fsInfo.usedBytes);

    if(curFile->isDirectory()) {
        // resource type
        response->print("<d:resourcetype><d:collection/></d:resourcetype>");
    } else	{
        // etag
        response->printf("<d:getetag>%s</d:getetag>", sha1(fullPath + fileTimeStamp).c_str());

        // resource type
        response->print("<d:resourcetype/>");

        // content length
        response->printf("<d:getcontentlength>%d</d:getcontentlength>", curFile->size());

        // content type
        response->print("<d:getcontenttype>text/plain</d:getcontenttype>");
    }
    response->print("</d:prop>");
    response->print("<d:status>HTTP/1.1 200 OK</d:status>");
    response->print("</d:propstat>");

    response->print("</d:response>");
}
