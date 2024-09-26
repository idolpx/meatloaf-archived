// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// Meatloaf is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Meatloaf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.


#include "main.h"

// ------------------------
void setup()
{

    delay ( 1000 );

    // Setup Modem
    modem.setup();


    Serial.println ( "\r\n\r\n==============================" );
    Serial.println ( "   " PRODUCT_ID " " FW_VERSION );
    Serial.println ( "------------------------------" );

    modem.start();

// #if defined(ESP8266)
//     // initialize selected file system
//     fileSystemConfig.setAutoFormat ( AUTO_FORMAT );
// #if defined USE_SDFS
//     fileSystemConfig.setCSPin ( CHIP_SELECT_PIN );
//     fileSystemConfig.setSPI ( SPI_SETTINGS );
// #endif
//     fileSystem->setConfig ( fileSystemConfig );
// #endif

#if defined(ESP8266)
    if ( !fileSystem->begin() )
#elif defined(ESP32) && defined(USE_LITTLEFS)
    if ( !LITTLEFS.begin() ) // not sure why pointer is not working
#else
    if ( !SPIFFS.begin() ) // not sure why pointer is not working
#endif
    {
        // File System failed
        statusMessage = "Failed to initialize file system";
        Serial.print ( "ERROR: " );
        Serial.println ( statusMessage );
        initFailed = true;
    }
    else
    {
        Serial.println ( "Flash File System started" );

        // Start the Web Server with WebDAV
        #if defined(ML_WEB_SERVER)
            setupWWW();
        #endif
        #if defined(ML_WEBDAV)
            // WebDAV Server Setup
            tcp.begin();
            dav.begin ( &tcp, fileSystem );
            dav.setTransferStatusCallback ( [] ( const char *name, int percent, bool receive )
            {
                Serial.printf ( "WebDAV %s: '%s': %d%%\n", receive ? "recv" : "send", name, percent );
            } );
            Serial.println("WebDAV server started");
        #endif


        // mDNS INIT
        #if defined(ML_MDNS)
            if ( MDNS.begin ( HOSTNAME ) )
            {
                MDNS.addService ( "http", "tcp", SERVER_PORT );
                Serial.println ( "mDNS service started" );
                Serial.println ( ">>> http://" HOSTNAME ".local" );
            }
            else
            {
                Serial.println ( "mDNS service failed to start" );
                Serial.print ( ">>> http://" );
                Serial.println ( WiFi.localIP() );
            }
        #endif

        // Setup IEC Bus
        iec.enabledDevices = DEVICE_MASK;
        iec.enableDevice(30);
        iec.init();
        Serial.println("IEC Bus Initialized");

        Serial.print("Virtual Device(s) Started: [ ");
        for (byte i = 0; i < 31; i++)
        {
         if (iec.isDeviceEnabled(i))
         {
             Serial.printf("%.02d ", i);
         }
        }
        Serial.println("]");

        // Setup interrupt for ATN
        attachInterrupt(
            digitalPinToInterrupt(IEC_PIN_ATN),
            onAttention,
            FALLING
        );
    }


    pinMode ( LED_PIN, OUTPUT ); // Configure the onboard LED for output
    ledON();
    Serial.println ( "READY." );

    //runTestsSuite();
}

// ------------------------
void loop()
{
#if defined(ML_MDNS)
    MDNS.update();
#endif

#if defined(ML_WEB_SERVER)
    www.handleClient();
#elif defined(ML_WEBDAV)
    dav.handleClient();
#endif

    modem.service();
    //cli.readSerial();
    if ( bus_state != statemachine::idle )
    {
        //Debug_printv("before[%d]", bus_state);
        if( drive.service() == IEC::BUS_IDLE)
            bus_state = statemachine::idle;
        //Debug_printv("after[%d]", bus_state);
    }


#ifdef DEBUG_TIMING
    iec.debugTiming();
#endif
}

void onAttention()
{
    bus_state = statemachine::select;
    iec.protocol.flags or_eq ATN_PULLED;
}


#if defined(ML_WEB_SERVER)
////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK()
{
    www.send ( 200, FPSTR ( TEXT_PLAIN ), "" );
}

void replyOKWithMsg ( String msg )
{
    www.send ( 200, FPSTR ( TEXT_PLAIN ), msg );
}

void replyNotFound ( String msg )
{
    www.send ( 404, FPSTR ( TEXT_PLAIN ), msg );
}

void replyBadRequest ( String msg )
{
    Debug_println ( msg );
    www.send ( 400, FPSTR ( TEXT_PLAIN ), msg + "\r\n" );
}

void replyServerError ( String msg )
{
    Debug_println ( msg );
    www.send ( 500, FPSTR ( TEXT_PLAIN ), msg + "\r\n" );
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath ( String filename )
{
    String error = String();

    if ( !filename.startsWith ( "/" ) )
    {
        error += F ( "!NO_LEADING_SLASH! " );
    }

    if ( filename.indexOf ( "//" ) != -1 )
    {
        error += F ( "!DOUBLE_SLASH! " );
    }

    if ( filename.endsWith ( "/" ) )
    {
        error += F ( "!TRAILING_SLASH! " );
    }

    return error;
}
#endif


////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
void handleStatus()
{
    Debug_println ( "handleStatus" );
    FSInfo fs_info;
    String json;
    json.reserve ( 128 );

    json = "{\"type\":\"" FS_TYPE "\", \"isOk\":";

    if ( fsOK )
    {
        fileSystem->info ( fs_info );
        json += F ( "\"true\", \"totalBytes\":\"" );
        json += fs_info.totalBytes;
        json += F ( "\", \"usedBytes\":\"" );
        json += fs_info.usedBytes;
        json += "\"";
    }
    else
    {
        json += "\"false\"";
    }

    json += F ( ",\"unsupportedFiles\":\"" );
    json += unsupportedFiles;
    json += "\"}";

    www.send ( 200, "application/json", json );
}


/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chunked responses.
*/
void handleFileList()
{
    if ( !fsOK )
    {
        return replyServerError ( FPSTR ( FS_INIT_ERROR ) );
    }

    if ( !www.hasArg ( "dir" ) )
    {
        return replyBadRequest ( F ( "DIR ARG MISSING" ) );
    }

    String path = www.arg ( "dir" );

    if ( path != "/" && !fileSystem->exists ( path ) )
    {
        return replyBadRequest ( "BAD PATH" );
    }

    Debug_println ( String ( "handleFileList: " ) + path );
    Dir dir = fileSystem->openDir ( path );
    path.clear();

    // use HTTP/1.1 Chunked response to avoid building a huge temporary string
    if ( !www.chunkedResponseModeStart ( 200, "text/json" ) )
    {
        www.send ( 505, F ( "text/html" ), F ( "HTTP1.1 required" ) );
        return;
    }

    // use the same string for every line
    String output;
    output.reserve ( 64 );

    while ( dir.next() )
    {
#ifdef USE_SPIFFS
        String error = checkForUnsupportedPath ( dir.fileName() );

        if ( error.length() > 0 )
        {
            Debug_println ( String ( "Ignoring " ) + error + dir.fileName() );
            continue;
        }

#endif

        if ( output.length() )
        {
            // www.send string from previous iteration
            // as an HTTP chunk
            www.sendContent ( output );
            output = ',';
        }
        else
        {
            output = '[';
        }

        output += "{\"type\":\"";

        if ( dir.isDirectory() )
        {
            output += "dir";
        }
        else
        {
            output += F ( "file\",\"size\":\"" );
            output += dir.fileSize();
        }

        output += F ( "\",\"name\":\"" );

        // Always return names without leading "/"
        if ( dir.fileName() [0] == '/' )
        {
            output += & ( dir.fileName() [1] );
        }
        else
        {
            output += dir.fileName();
        }

        output += "\"}";
    }

    // www.send last string
    output += "]";
    www.sendContent ( output );
    www.chunkedResponseFinalize();
}


/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead ( String path )
{
    Debug_println ( String ( "handleFileRead: " ) + path );

    if ( !fsOK )
    {
        replyServerError ( FPSTR ( FS_INIT_ERROR ) );
        return true;
    }

    if ( path.endsWith ( "/" ) )
    {
        path += "index.htm";
    }

    String contentType;

    if ( www.hasArg ( "download" ) )
    {
        contentType = F ( "application/octet-stream" );
    }
    else
    {
        contentType = mime::getContentType ( path );
    }

    if ( !fileSystem->exists ( path ) )
    {
        // File not found, try gzip version
        path = path + ".gz";
    }

    if ( fileSystem->exists ( path ) )
    {
        File file = fileSystem->open ( path, "r" );

        if ( www.streamFile ( file, contentType ) != file.size() )
        {
            Debug_println ( "Sent less data than expected!" );
        }

        file.close();
        return true;
    }

    return false;
}


/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent ( String path )
{
    while ( !path.isEmpty() && !fileSystem->exists ( path ) )
    {
        if ( path.lastIndexOf ( '/' ) > 0 )
        {
            path = path.substring ( 0, path.lastIndexOf ( '/' ) );
        }
        else
        {
            path = String();  // No slash => the top folder does not exist
        }
    }

    Debug_println ( String ( "Last existing parent: " ) + path );
    return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
void handleFileCreate()
{
    if ( !fsOK )
    {
        return replyServerError ( FPSTR ( FS_INIT_ERROR ) );
    }

    String path = www.arg ( "path" );

    if ( path.isEmpty() )
    {
        return replyBadRequest ( F ( "PATH ARG MISSING" ) );
    }

#ifdef USE_SPIFFS

    if ( checkForUnsupportedPath ( path ).length() > 0 )
    {
        return replyServerError ( F ( "INVALID FILENAME" ) );
    }

#endif

    if ( path == "/" )
    {
        return replyBadRequest ( "BAD PATH" );
    }

    if ( fileSystem->exists ( path ) )
    {
        return replyBadRequest ( F ( "PATH FILE EXISTS" ) );
    }

    String src = www.arg ( "src" );

    if ( src.isEmpty() )
    {
        // No source specified: creation
        Debug_println ( String ( "handleFileCreate: " ) + path );

        if ( path.endsWith ( "/" ) )
        {
            // Create a folder
            path.remove ( path.length() - 1 );

            if ( !fileSystem->mkdir ( path ) )
            {
                return replyServerError ( F ( "MKDIR FAILED" ) );
            }
        }
        else
        {
            // Create a file
            File file = fileSystem->open ( path, "w" );

            if ( file )
            {
                file.write ( ( const char * ) 0 );
                file.close();
            }
            else
            {
                return replyServerError ( F ( "CREATE FAILED" ) );
            }
        }

        if ( path.lastIndexOf ( '/' ) > -1 )
        {
            path = path.substring ( 0, path.lastIndexOf ( '/' ) );
        }

        replyOKWithMsg ( path );
    }
    else
    {
        // Source specified: rename
        if ( src == "/" )
        {
            return replyBadRequest ( "BAD SRC" );
        }

        if ( !fileSystem->exists ( src ) )
        {
            return replyBadRequest ( F ( "SRC FILE NOT FOUND" ) );
        }

        Debug_println ( String ( "handleFileCreate: " ) + path + " from " + src );

        if ( path.endsWith ( "/" ) )
        {
            path.remove ( path.length() - 1 );
        }

        if ( src.endsWith ( "/" ) )
        {
            src.remove ( src.length() - 1 );
        }

        if ( !fileSystem->rename ( src, path ) )
        {
            return replyServerError ( F ( "RENAME FAILED" ) );
        }

        replyOKWithMsg ( lastExistingParent ( src ) );
    }
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive ( String path )
{
    File file = fileSystem->open ( path, "r" );
    bool isDir = file.isDirectory();
    file.close();

    // If it's a plain file, delete it
    if ( !isDir )
    {
        fileSystem->remove ( path );
        return;
    }

    // Otherwise delete its contents first
    Dir dir = fileSystem->openDir ( path );

    while ( dir.next() )
    {
        deleteRecursive ( path + '/' + dir.fileName() );
    }

    // Then delete the folder itself
    fileSystem->rmdir ( path );
}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete()
{
    if ( !fsOK )
    {
        return replyServerError ( FPSTR ( FS_INIT_ERROR ) );
    }

    String path = www.arg ( 0 );

    if ( path.isEmpty() || path == "/" )
    {
        return replyBadRequest ( "BAD PATH" );
    }

    Debug_println ( String ( "handleFileDelete: " ) + path );

    if ( !fileSystem->exists ( path ) )
    {
        return replyNotFound ( FPSTR ( FILE_NOT_FOUND ) );
    }

    deleteRecursive ( path );

    replyOKWithMsg ( lastExistingParent ( path ) );
}

/*
   Handle a file upload request
*/
void handleFileUpload()
{
    if ( !fsOK )
    {
        return replyServerError ( FPSTR ( FS_INIT_ERROR ) );
    }

    if ( www.uri() != "/edit" )
    {
        return;
    }

    HTTPUpload &upload = www.upload();

    if ( upload.status == UPLOAD_FILE_START )
    {
        String filename = upload.filename;

        // Make sure paths always start with "/"
        if ( !filename.startsWith ( "/" ) )
        {
            filename = "/" + filename;
        }

        Debug_println ( String ( "handleFileUpload Name: " ) + filename );
        uploadFile = fileSystem->open ( filename, "w" );

        if ( !uploadFile )
        {
            return replyServerError ( F ( "CREATE FAILED" ) );
        }

        Debug_println ( String ( "Upload: START, filename: " ) + filename );
    }
    else if ( upload.status == UPLOAD_FILE_WRITE )
    {
        if ( uploadFile )
        {
            size_t bytesWritten = uploadFile.write ( upload.buf, upload.currentSize );

            if ( bytesWritten != upload.currentSize )
            {
                return replyServerError ( F ( "WRITE FAILED" ) );
            }
        }

        Debug_println ( String ( "Upload: WRITE, Bytes: " ) + upload.currentSize );
    }
    else if ( upload.status == UPLOAD_FILE_END )
    {
        if ( uploadFile )
        {
            uploadFile.close();
        }

        Debug_println ( String ( "Upload: END, Size: " ) + upload.totalSize );
    }
}


/*
   The "Not Found" handler catches all URI not explicitly declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
// void notFound ()
// {
//     String nf = DAVROOT;
//     nf += ESP8266WebServer::urlDecode(server.uri());
//     Serial.printf("User request for HTTP file '%s' from '" DAVROOT "'\n", nf.c_str() + sizeof(DAVROOT));
//     // open file 'DAVROOT nf' (/dav/userfilename)
//     File f = gfs.open(nf.c_str(), "r");
//     if (!f)
//     {
//         Serial.printf("not found: '%s'\n", nf.c_str());
//         server.send(404, FPSTR(TEXT_PLAIN), "webserver's notfound/404");
//     }
//     else
//     {
//         // This is a reachable piece of code if the file is
//         // explicitally ignored in dav.setIgnored({}),
//         // or if DAVROOT is not '/'.
//         Serial.printf("found, streaming with HTTP (not webdav)\n");
//         server.streamFile(f, F("application/octet-stream"));
//     }
// }
void handleNotFound()
{
    if ( !fsOK )
    {
        return replyServerError ( FPSTR ( FS_INIT_ERROR ) );
    }

    String uri = WebServer::urlDecode ( www.uri() ); // required to read paths with blanks

    Debug_printv("url[%s]", uri);

    if ( handleFileRead ( uri ) )
    {
        return;
    }

    // Dump debug data
    String message;
    message.reserve ( 100 );
    message = F ( "Error: File not found\n\nURI: " );
    message += uri;
    message += F ( "\nMethod: " );
    message += ( www.method() == HTTP_GET ) ? "GET" : "POST";
    message += F ( "\nArguments: " );
    message += www.args();
    message += '\n';

    for ( uint8_t i = 0; i < www.args(); i++ )
    {
        message += F ( " NAME:" );
        message += www.argName ( i );
        message += F ( "\n VALUE:" );
        message += www.arg ( i );
        message += '\n';
    }

    message += "path=";
    message += www.arg ( "path" );
    message += '\n';
    Debug_print ( message );

    return replyNotFound ( message );
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit()
{
    if ( handleFileRead ( F ( "/edit/index.htm" ) ) )
    {
        return;
    }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
    www.sendHeader ( F ( "Content-Encoding" ), "gzip" );
    www.send ( 200, "text/html", index_htm_gz, index_htm_gz_len );
#else
    replyNotFound ( FPSTR ( FILE_NOT_FOUND ) );
#endif

}


void setupWWW ( void )
{

#ifdef USE_SPIFFS
    // Debug: dump on console contents of filesystem with no filter and check filenames validity
    Dir dir = fileSystem->openDir ( "" );
    Debug_println ( F ( "List of files at root of filesystem:" ) );

    while ( dir.next() )
    {
        String error = checkForUnsupportedPath ( dir.fileName() );
        String fileInfo = dir.fileName() + ( dir.isDirectory() ? " [DIR]" : String ( " (" ) + dir.fileSize() + "b)" );
        Debug_println ( error + fileInfo );

        if ( error.length() > 0 )
        {
            unsupportedFiles += error + fileInfo + '\n';
        }
    }

    Debug_println();

    // Keep the "unsupportedFiles" variable to show it, but clean it up
    unsupportedFiles.replace ( "\n", "<br/>" );
    unsupportedFiles = unsupportedFiles.substring ( 0, unsupportedFiles.length() - 5 );
#endif

    // WebDAV Server Setup
    dav.begin ( fileSystem );
    dav.setTransferStatusCallback ( [] ( const char *name, int percent, bool receive )
    {
        Serial.printf ( "WebDAV %s: '%s': %d%%\n", receive ? "recv" : "send", name, percent );
    } );


    ////////////////////////////////
    // WEB SERVER INIT
    // Set WebDAV hook
    www.addHook ( hookWebDAVForWebserver ( "/", dav, "/" ) );

    // Allow some paths within the WebDAV namespace to be served by the regular webwerver
    //
    // 1. provide a callback to verify what to ignore to WebDAV (=> thus served by webserver)
    dav.setIgnored([] (const String& uri) { return uri == F("/index.html") || uri == F("/another.html") || uri == F("/notfound.html"); });
    //
    // 2. setup regular web pages callbacks (only meaningful when paths are ignored by WebDAV, like above)
    www.on(F("/index.html"), [] () { www.send_P(200, PSTR("text/html"), PSTR("<meta http-equiv=\"Refresh\" content=\"0;url=/.www/index.html\">")); });
    //www.on(F("/another.html"), [] () { www.send_P(200, PSTR("text/html"), PSTR("<button style=\"background-color:red;color:orange;\" onclick=\"window.location.href='notfound.html';\">heaven</button>")); });

    // Filesystem status
    www.on ( "/status", HTTP_GET, handleStatus );

    // List directory
    www.on ( "/list", HTTP_GET, handleFileList );

    // Load editor
    www.on ( "/edit", HTTP_GET, handleGetEdit );

    // Create file
    www.on ( "/edit",  HTTP_PUT, handleFileCreate );

    // Delete file
    www.on ( "/edit",  HTTP_DELETE, handleFileDelete );

    // Upload file
    // - first callback is called after the request has ended with all parsed arguments
    // - second callback handles file upload at that location
    www.on ( "/edit",  HTTP_POST, replyOK, handleFileUpload );

    // Default handler for all URIs not defined above
    // Use it to read files from filesystem
    www.onNotFound ( handleNotFound );

    // Start Server
    www.begin();

    Serial.println ( "HTTP Server started" );
    Serial.println ( "WebDAV Server started" );

}

#endif
