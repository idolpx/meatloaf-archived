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

//#include "../../include/global_defines.h"
//#include "debug.h"

#include "iec_device.h"
#include "iec.h"
#include "wrappers/iec_buffer.h"

using namespace CBM;

namespace
{

	// Buffer for incoming and outgoing serial bytes and other stuff.
	char serCmdIOBuf[MAX_BYTES_PER_REQUEST];

} // unnamed namespace
Interface::Interface(IEC &iec)
	: m_iec(iec),
	m_atn_cmd(*reinterpret_cast<IEC::ATNCmd *>(&serCmdIOBuf[sizeof(serCmdIOBuf) / 2])),
	m_device(0),
	m_mfile(MFSOwner::File(""))
{
	reset();
} // ctor

bool Interface::begin()
{
	return true;
}

void Interface::reset(void)
{
	m_openState = O_NOTHING;
	setDeviceStatus(73);
	//m_device.reset();
} // reset


void Interface::sendFileNotFound(void)
{
	setDeviceStatus(62);
	m_iec.sendFNF();
}

void Interface::sendStatus(void)
{
	std::string status = m_device_status;
	if (status.size() == 0)
		status = "00, OK,00,00";

	Debug_printv("status: %s", status.c_str());
	Debug_print("[");
	
	m_iec.send(status);

	Debug_println("]");

	// Send CR with EOI marker.
	m_iec.sendEOI('\x0D');

	// Clear the status message
	m_device_status.clear();
} // sendStatus

void Interface::setDeviceStatus(int number, int track, int sector)
{
	switch(number)
	{
		// 1 - FILES SCRATCHED - number scratched in track variable
		case 1:
			m_device_status = "01,FILES SCRATCHED,00,00";		
			break;
		// 20 READ ERROR
		case 20:
			m_device_status = "20,FILE NOT OPEN,00,00";		
			break;
		// 26 WRITE PROTECT ON
		case 26:
			m_device_status = "26,WRITE PROTECT ON,00,00";
			break;
		// 30 SYNTAX ERROR - in arguments
		case 30:
			m_device_status = "30,SYNTAX ERROR,00,00";
			break;
		// 31 SYNTAX ERROR - unknown command
		case 31:
			m_device_status = "31,SYNTAX ERROR,00,00";
			break;
		// 33 S.E. - invalid pattern
		case 33:
			m_device_status = "33,SYNTAX ERROR,00,00";
			break;
		// 34 S.E. - no file name given
		case 34:
			m_device_status = "34,SYNTAX ERROR,00,00";
			break;
		// 50 RECORD NOT PRESENT - eof
		case 50:
			m_device_status = "50,RECORD NOT PRESENT,00,00";
			break;
		// 60 WRITE FILE OPEN - trying to open for wrtiting a file that is open for writing
		case 60:
			m_device_status = "60,WRITE FILE OPEN,00,00";
			break;
		// 61 FILE NOT OPEN
		case 61:
			m_device_status = "61,FILE NOT OPEN,00,00";
			break;
		// 62 FILE NOT FOUND
		case 62:
			m_device_status = "62,FILE NOT FOUND,00,00";
			break;
		// 63 FILE EXISTS
		case 63:
			m_device_status = "63,FILE EXISTS,00,00";
			break;
		// 65 NO BLOCK - for B-A
		case 65:
			m_device_status = "65,NO BLOCK,00,00";
			break;
		// 73 boot message: device name, rom version etc.
		case 73:
			m_device_status = "73," PRODUCT_ID " [" FW_VERSION "],00,00";
			break;
		// 74 DRIVE NOT READY - also drive out of memory
		case 74:
			m_device_status = "74,DRIVE NOT READY,00,00";
			break;
		// 77 SELECTED PARTITION ILLEGAL
		case 77:
			m_device_status = "77,SELECTED PARTITION ILLEGAL,00,00";
			break;
		// 78 BUFFER TOO SMALL (sd2iec buffer ops)
		case 78:
			m_device_status = "78,BUFFER TOO SMALL,00,00";
			break;
		// 8x 64HDD errors
		// case 8:
		// 	break;
		// 89 COMMAND NOT SUPPORTED
		case 89:
			m_device_status = "89,COMMAND NOT SUPPORTED,00,00";
			break;
		// 91 64HDD device activated
		case 91:
			m_device_status = "91,DEVICE ACTIVATED,00,00";
			break;
		case 125:
			m_device_status = "125,NETWORK TIMEOUT,00,00";
			break;
		case 126:
			m_device_status = "126,NODE NOT FOUND,00,00";
			break;
	}
}


void Interface::sendDeviceInfo()
{
	Debug_printf("\r\nsendDeviceInfo:\r\n");

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// #if defined(USE_LITTLEFS)
	//FSInfo64 fs_info;
	//m_fileSystem->info64(fs_info);
	// #endif
	char floatBuffer[10]; // buffer
	dtostrf(getFragmentation(), 3, 2, floatBuffer);

	// Send load address
	m_iec.send(C64_BASIC_START bitand 0xff);
	m_iec.send((C64_BASIC_START >> 8) bitand 0xff);
	Debug_println("");

	// Send List HEADER
	sendLine(basicPtr, 0, CBM_DEL_DEL CBM_REVERSE_ON " %s V%s ", PRODUCT_ID, FW_VERSION);

	// CPU
	sendLine(basicPtr, 0, CBM_DEL_DEL "SYSTEM ---");
	String sdk = String(ESP.getSdkVersion());
	sdk.toUpperCase();
	sendLine(basicPtr, 0, CBM_DEL_DEL "SDK VER    : %s", sdk.c_str());
	//sendLine(basicPtr, 0, "BOOT VER   : %08X", ESP.getBootVersion());
	//sendLine(basicPtr, 0, "BOOT MODE  : %08X", ESP.getBootMode());
	//sendLine(basicPtr, 0, "CHIP ID    : %08X", ESP.getChipId());
	sendLine(basicPtr, 0, CBM_DEL_DEL "CPU MHZ    : %d MHZ", ESP.getCpuFreqMHz());
	sendLine(basicPtr, 0, CBM_DEL_DEL "CYCLES     : %u", ESP.getCycleCount());

	// POWER
	sendLine(basicPtr, 0, CBM_DEL_DEL "POWER ---");
	//sendLine(basicPtr, 0, "VOLTAGE    : %d.%d V", ( ESP.getVcc() / 1000 ), ( ESP.getVcc() % 1000 ));

	// RAM
	sendLine(basicPtr, 0, CBM_DEL_DEL "MEMORY ---");
	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM SIZE   : %5d B", getTotalMemory());
	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM FREE   : %5d B", getTotalAvailableMemory());
	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM >BLK   : %5d B", getLargestAvailableBlock());
	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM FRAG   : %s %%", floatBuffer);

	// ROM
	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM SIZE   : %5d B", ESP.getSketchSize() + ESP.getFreeSketchSpace());
	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM USED   : %5d B", ESP.getSketchSize());
	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM FREE   : %5d B", ESP.getFreeSketchSpace());

	// FLASH
	sendLine(basicPtr, 0, CBM_DEL_DEL "STORAGE ---");
	//sendLine(basicPtr, 0, "FLASH SIZE : %5d B", ESP.getFlashChipRealSize());
	sendLine(basicPtr, 0, CBM_DEL_DEL "FLASH SPEED: %d MHZ", (ESP.getFlashChipSpeed() / 1000000));

	// // FILE SYSTEM
	// sendLine(basicPtr, 0, CBM_DEL_DEL "FILE SYSTEM ---");
	// sendLine(basicPtr, 0, CBM_DEL_DEL "TYPE       : %s", FS_TYPE);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "SIZE       : %5d B", fs_info.totalBytes);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "USED       : %5d B", fs_info.usedBytes);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "FREE       : %5d B", fs_info.totalBytes - fs_info.usedBytes);

	// NETWORK
	sendLine(basicPtr, 0, CBM_DEL_DEL "NETWORK ---");
	char ip[16];
	sprintf(ip, "%s", ipToString(WiFi.softAPIP()).c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "AP MAC     : %s", WiFi.softAPmacAddress().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "AP IP      : %s", ip);
	sprintf(ip, "%s", ipToString(WiFi.localIP()).c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "STA MAC    : %s", WiFi.macAddress().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "STA IP     : %s", ip);

	// End program with two zeros after last line. Last zero goes out as EOI.
	m_iec.send(0);
	m_iec.sendEOI(0);

	ledON();
} // sendDeviceInfo

void Interface::sendDeviceStatus()
{
	Debug_printf("\r\nsendDeviceStatus:\r\n");

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	m_iec.send(C64_BASIC_START bitand 0xff);
	m_iec.send((C64_BASIC_START >> 8) bitand 0xff);
	Debug_println("");

	// Send List HEADER
	sendLine(basicPtr, 0, CBM_DEL_DEL CBM_REVERSE_ON " %s V%s ", PRODUCT_ID, FW_VERSION);

	// Current Config
	sendLine(basicPtr, 0, CBM_DEL_DEL "DEVICE ID : %d", m_device.id());
	sendLine(basicPtr, 0, CBM_DEL_DEL "MEDIA     : %d", m_device.media());
	sendLine(basicPtr, 0, CBM_DEL_DEL "PARTITION : %d", m_device.partition());
	sendLine(basicPtr, 0, CBM_DEL_DEL "URL       : %s", m_device.url().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "PATH      : %s", m_device.path().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "ARCHIVE   : %s", m_device.archive().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "IMAGE     : %s", m_device.image().c_str());
	sendLine(basicPtr, 0, CBM_DEL_DEL "FILENAME  : %s", m_mfile->name.c_str());

	// End program with two zeros after last line. Last zero goes out as EOI.
	m_iec.send(0);
	m_iec.sendEOI(0);

	ledON();
} // sendDeviceStatus

uint8_t Interface::service(void)
{
	//#ifdef HAS_RESET_LINE
	//	if(m_iec.checkRESET()) {
	//		// IEC reset line is in reset mode, so we should set all modes in reset.
	//		reset();
	//
	//
	//		return IEC::ATN_RESET;
	//	}
	//#endif
	// Wait for it to get out of reset.
	//while (m_iec.checkRESET())
	//{
	//	Debug_println("ATN_RESET");
	//}

	//	noInterrupts();
	IEC::ATNMode mode = m_iec.service(m_atn_cmd);
	//	interrupts();


	if (mode == IEC::ATN_ERROR)
	{
		reset();
		mode = IEC::ATN_IDLE;
	}

	// Did anything happen from the host side?
	else if (mode not_eq IEC::ATN_IDLE)
	{
		switch (m_atn_cmd.command)
		{
			case IEC::ATN_CODE_OPEN:
				Debug_printv("[OPEN]");
				if (m_atn_cmd.channel == 0)
					Debug_printf("LOAD \"%s\",%d ", m_atn_cmd.str, m_atn_cmd.device);
				else if (m_atn_cmd.channel == 1)
					Debug_printf("SAVE \"%s\",%d ", m_atn_cmd.str, m_atn_cmd.device);
				else {
					Debug_printf("OPEN #,%d,%d,\"%s\"", m_atn_cmd.device, m_atn_cmd.channel, m_atn_cmd.str);
				}

				// Open either file or prg for reading, writing or single line command on the command channel.
				handleATNCmdCodeOpen(m_atn_cmd);
				break;

			case IEC::ATN_CODE_SECOND: // data channel opened
				Debug_printv("[SECOND]");
				if (mode == IEC::ATN_CMD)
				{
					// Process a command
					Debug_printv("[Process a command]");
					handleATNCmdCodeOpen(m_atn_cmd);
				}
				else if (mode == IEC::ATN_CMD_LISTEN)
				{
					// Receive data
					Debug_printv("[Receive data]");
					handleATNCmdCodeDataListen();	
				}
				else if (mode == IEC::ATN_CMD_TALK)
				{
					// Send data
					Debug_printv("[Send data]");
					if (m_atn_cmd.channel == CMD_CHANNEL)
					{
						handleATNCmdCodeOpen(m_atn_cmd);		 // This is typically an empty command,
					}

					handleATNCmdCodeDataTalk(m_atn_cmd.channel);
				}
				break;

			case IEC::ATN_CODE_CLOSE:
				//Debug_printv("[CLOSE] ");
				handleATNCmdClose();
				break;
		} // switch
	}
	//Debug_printv("mode[%d] command[%.2X] channel[%.2X] state[%d]", mode, m_atn_cmd.command, m_atn_cmd.channel, m_openState);

	return mode;
} // service

MFile* Interface::getPointed(MFile* urlFile) {
	Debug_printv("getPointed [%s]", urlFile->url);
	auto istream = Meat::ifstream(urlFile);

	istream.open();

    if(!istream.is_open()) {
        Debug_printf("couldn't open stream of urlfile");
		return nullptr;
    }
	else {
		std::string linkUrl;
		istream >> linkUrl;
		Debug_printv("path read from [%s]=%s", urlFile->url.c_str(), linkUrl.c_str());

		return MFSOwner::File(linkUrl);
	}
};

CommandPathTuple Interface::parseLine(std::string command, size_t channel)
{

	Debug_printv("* PARSE INCOMING LINE *******************************");

	Debug_printv("we are in              [%s]", m_mfile->url.c_str());
	Debug_printv("unprocessed user input [%s]", command.c_str());

	std::string guessedPath = command;
	CommandPathTuple tuple;

	mstr::toASCII(guessedPath);

	// check to see if it starts with a known command token
	if ( mstr::startsWith(command, "cd:", false) ) // would be case sensitive, but I don't know the proper case
	{
		guessedPath = mstr::drop(guessedPath, 3);
		tuple.command = "cd";
		// if ( mstr::startsWith(guessedPath, ":" ) ) // drop ":" if it was specified
		// 	guessedPath = mstr::drop(guessedPath, 1);
		// else if ( channel != 15 )
		// 	guessedPath = command;
	}
	else if(mstr::startsWith(command, "@info", false))
	{
		guessedPath = mstr::drop(guessedPath, 5);
		tuple.command = "@info";
	}
	else if(mstr::startsWith(command, "@stat", false))
	{
		guessedPath = mstr::drop(guessedPath, 5);
		tuple.command = "@stat";
	}
	else if(mstr::startsWith(command, ":")) {
		// JiffyDOS eats commands it knows, it might be T: which means ASCII dump requested
		guessedPath = mstr::drop(guessedPath, 1);
		tuple.command = "t";
	}
	else if(mstr::startsWith(command, "S:")) {
		// capital S = heart, that's a FAV!
		guessedPath = mstr::drop(guessedPath, 2);
		tuple.command = "mfav";
	}
	else if(mstr::startsWith(command, "MFAV:")) {
		// capital S = heart, that's a FAV!
		guessedPath = mstr::drop(guessedPath, 5);
		tuple.command = "mfav";
	}
	else
	{
		tuple.command = command;
	}

	// TODO more of them?

	// NOW, since user could have requested ANY kind of our supported magic paths like:
	// LOAD ~/something
	// LOAD ../something
	// LOAD //something
	// we HAVE TO PARSE IT OUR WAY!


	// and to get a REAL FULL PATH that the user wanted to refer to, we CD into it, using supplied stripped path:
	mstr::trim(guessedPath);
	tuple.rawPath = guessedPath;

	Debug_printv("found command     [%s]", tuple.command.c_str());

	if(guessedPath == "$") {
		Debug_printv("get directory of [%s]", m_mfile->url.c_str());
	}
	else if(!guessedPath.empty()) {
		auto fullPath = Meat::Wrap(m_mfile->cd(guessedPath));

		tuple.fullPath = fullPath->url;

		Debug_printv("full referenced path [%s]", tuple.fullPath.c_str());
	}

	Debug_printv("* END OF PARSE LINE *******************************");

	return tuple;
}

void Interface::handleATNCmdCodeOpen(IEC::ATNCmd &atn_cmd)
{
	if (m_device.select(atn_cmd.device))
	{
		m_mfile.reset(MFSOwner::File(m_device.url()));
		Debug_printv("!!!! device changed: unit:%d current url: [%s]", m_device.id(), m_device.url().c_str());
		Debug_printv("m_mfile[%s]", m_mfile->url.c_str());
	}

	size_t channel = atn_cmd.channel;
	m_openState = O_NOTHING;

	if ( strlen(atn_cmd.str) == 0 )
	{
		Debug_printv("No command to process");

		if ( atn_cmd.channel == CMD_CHANNEL )
			m_openState = O_STATUS;
		return;
	}

	// 1. obtain command and fullPath
	auto commandAndPath = parseLine(atn_cmd.str, channel);
	auto referencedPath = Meat::New<MFile>(commandAndPath.fullPath);

	Debug_printv("command[%s]", commandAndPath.command.c_str());
	if (mstr::endsWith(commandAndPath.command, "$"))
	{
		m_openState = O_DIR;
		Debug_printv("LOAD $");
	}	
	else if (mstr::endsWith(commandAndPath.command, "*"))
	{
		// Find first program in listing
		if (m_device.path().empty())
		{
			// If in LittleFS root then set it to FB64
			// TODO: Load configured autoload program
			m_filename = "/.sys/fb64";
		}
		else if (m_filename.size() == 0)
		{	
			// Find first PRG file in current directory
			std::unique_ptr<MFile> entry(m_mfile->getNextFileInDir());

			while (entry != nullptr && entry->isDirectory())
			{
				Debug_printv("extension: [%s]", entry->extension.c_str());
				entry.reset(m_mfile->getNextFileInDir());
			}
			m_filename = entry->name;
		}
		m_openState = O_FILE;
		Debug_printv("LOAD * [%s]", m_filename.c_str());
	}
	else if (mstr::equals(commandAndPath.command, "@info", false))
	{
		m_openState = O_ML_INFO;
	}
	else if (mstr::equals(commandAndPath.command, "@stat", false))
	{
		m_openState = O_ML_STATUS;
	}
	else if (commandAndPath.command == "mfav") {
		// create a urlfile named like the argument, containing current m_mfile path
		// here we don't want the full path provided by commandAndPath, though
		// the full syntax should be: heart:urlfilename,[filename] - optional name of the file that should be pointed to

		auto favStream = Meat::ofstream(commandAndPath.rawPath+".url"); // put the name from argument here!
		favStream.open();
		if(favStream.is_open()) {
			favStream << m_mfile->url;
		}
		favStream.close();
	}
	else if(!commandAndPath.rawPath.empty())
	{
		// 2. fullPath.extension == "URL" - change dir or load file
		if (mstr::equals(referencedPath->extension, "url", false)) 
		{
			// CD to the path inside the .url file
			referencedPath.reset(getPointed(referencedPath.get()));

			if ( referencedPath->isDirectory() )
			{
				Debug_printv("change dir called for urlfile");
				changeDir(referencedPath->url);
			}
			else if ( referencedPath->exists() )
			{
				prepareFileStream(referencedPath->url);
			}
		}
		// 2. OR if command == "CD" OR fullPath.isDirectory - change directory
		if (mstr::equals(commandAndPath.command, "cd", false) || referencedPath->isDirectory())
		{
			Debug_printv("change dir called by CD command or because of isDirectory");
			changeDir(referencedPath->url);
		}
		// 3. else - stream file
		else if ( referencedPath->exists() )
		{
			// Set File
			prepareFileStream(referencedPath->url);
		}
	}

	//dumpState();

	// Clear command string
	m_atn_cmd.str[0] = '\0';
} // handleATNCmdCodeOpen

void Interface::dumpState() {
	Debug_println("");
	Debug_printv("-------------------------------");
	Debug_printv("URL: [%s]", m_mfile->url.c_str());
    Debug_printv("streamPath: [%s]", m_mfile->streamPath.c_str());
    Debug_printv("pathInStream: [%s]", m_mfile->pathInStream.c_str());
	Debug_printv("Scheme: [%s]", m_mfile->scheme.c_str());
	Debug_printv("Username: [%s]", m_mfile->user.c_str());
	Debug_printv("Password: [%s]", m_mfile->pass.c_str());
	Debug_printv("Host: [%s]", m_mfile->host.c_str());
	Debug_printv("Port: [%s]", m_mfile->port.c_str());
	Debug_printv("Path: [%s]", m_mfile->path.c_str());
	Debug_printv("File: [%s]", m_mfile->name.c_str());
	Debug_printv("Extension: [%s]", m_mfile->extension.c_str());
	Debug_printv("");
	Debug_printv("m_filename: [%s]", m_filename.c_str());
    Debug_printv("-------------------------------");

}

void Interface::changeDir(std::string url)
{
	m_device.url(url);
	m_mfile.reset(MFSOwner::File(url));
	m_openState = O_DIR;
	Debug_printv("!!!! CD into [%s]", url.c_str());
	Debug_printv("new current url: [%s]", m_mfile->url.c_str());
	Debug_printv("LOAD $");		
}

void Interface::prepareFileStream(std::string url)
{
	m_filename = url;
	m_openState = O_FILE;
	Debug_printv("LOAD [%s]", url.c_str());
}

void Interface::handleATNCmdCodeDataTalk(byte chan)
{
	Debug_printf("(%d CHANNEL) (%d openState)\r\n", chan, m_openState);

	switch (m_openState)
	{
		case O_NOTHING:
			break;

		case O_STATUS:
			// Send status
			sendStatus();
			break;

		case O_FILE:
			// Send file
			sendFile();
			break;

		case O_DIR:
			// Send listing
			sendListing();
			break;

		case O_ML_INFO:
			// Send device info
			sendDeviceInfo();
			break;

		case O_ML_STATUS:
			// Send device info
			sendDeviceStatus();
			break;
	}

	m_openState = O_NOTHING;
} // handleATNCmdCodeDataTalk

void Interface::handleATNCmdCodeDataListen()
{
	Debug_printv("[%s]", m_device.url().c_str());

	saveFile();
} // handleATNCmdCodeDataListen

void Interface::handleATNCmdClose()
{

} // handleATNCmdClose

// send single basic line, including heading basic pointer and terminating zero.
uint16_t Interface::sendLine(uint16_t &basicPtr, uint16_t blocks, const char *format, ...)
{
	// Format our string
	va_list args;
	va_start(args, format);
	char text[vsnprintf(NULL, 0, format, args) + 1];
	vsnprintf(text, sizeof text, format, args);
	va_end(args);

	return sendLine(basicPtr, blocks, text);
}

uint16_t Interface::sendLine(uint16_t &basicPtr, uint16_t blocks, char *text)
{
	byte i;
	uint16_t b_cnt = 0;

	Debug_printf("%d %s ", blocks, text);

	// Get text length
	uint8_t len = strlen(text);

	// Increment next line pointer
	basicPtr += len + 5;

	// Send that pointer
	m_iec.send(basicPtr bitand 0xFF);
	m_iec.send(basicPtr >> 8);

	// Send blocks
	m_iec.send(blocks bitand 0xFF);
	m_iec.send(blocks >> 8);

	// Send line contents
	for (i = 0; i < len; i++)
		m_iec.send(text[i]);

	// Finish line
	m_iec.send(0);

	Debug_println("");

	b_cnt += (len + 5);

	return b_cnt;
} // sendLine


uint16_t Interface::sendHeader(uint16_t &basicPtr, std::string header)
{
	uint16_t byte_count = 0;
	bool sent_info = false;

	PeoplesUrlParser p;
	std::string url = m_device.url();

	mstr::toPETSCII(url);
	p.parseUrl(url);

	url = p.root();
	std::string path = p.pathToFile();
	std::string image = p.name;

	// Send List HEADER
	//byte_count += sendLine(basicPtr, 0, "\x12\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", m_device.device());
	byte_count += sendLine(basicPtr, 0, CBM_REVERSE_ON "%s", header.c_str());

	// Send Extra INFO
	if (url.size())
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[URL]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, url.c_str());
		sent_info = true;
	}
	if (path.size() > 1)
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[PATH]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, path.c_str());
		sent_info = true;
	}
	// if (m_device.archive().length() > 1)
	// {
	// 	byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[ARCHIVE]");
	// 	byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, m_device.archive().c_str());
	// }
	if (image.size())
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[IMAGE]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, image.c_str());
		sent_info = true;
	}
	if (sent_info)
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"-------------------\" NFO", 0, "");
	}

	return byte_count;
}

void Interface::sendListing()
{
	Debug_printf("sendListing: [%s]\r\n=================================\r\n", m_mfile->url.c_str());

	uint16_t byte_count = 0;
	std::string extension = "dir";

	std::unique_ptr<MFile> entry(m_mfile->getNextFileInDir());

	if(entry == nullptr) {
		ledOFF();
		sendFileNotFound();
		return;
	}

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	m_iec.send(C64_BASIC_START bitand 0xff);
	m_iec.send((C64_BASIC_START >> 8) bitand 0xff);
	byte_count += 2;
	Debug_println("");

	// Send Listing Header
	char buffer[100];
	byte space_cnt = 0;
	// if (m_mfile->media_header.size() == 0)
	{
		// Set device default Listing Header
		space_cnt = (16 - strlen(PRODUCT_ID)) / 2;
		sprintf(buffer, "\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", m_device.id());
	}
	// else
	// {
	// 	space_cnt = (16 - m_mfile->media_header.size()) / 2;
	// 	sprintf(buffer, "\"%*s%s%*s\" %s\x00", space_cnt, "", m_mfile->media_header.c_str(), space_cnt, "", m_mfile->media_id.c_str());
	// }
	byte_count += sendHeader(basicPtr, buffer);
	
	// Send Directory Items
	while(entry != nullptr)
	{
		uint16_t block_cnt = entry->size() / m_mfile->media_block_size;
		byte block_spc = 3;
		if (block_cnt > 9)
			block_spc--;
		if (block_cnt > 99)
			block_spc--;
		if (block_cnt > 999)
			block_spc--;

		byte space_cnt = 21 - (entry->name.length() + 5);
		if (space_cnt > 21)
			space_cnt = 0;

		if (!entry->isDirectory())
		{
			if ( block_cnt < 1)
				block_cnt = 1;

			// Get extension
			if (entry->extension.length())
			{
				extension = entry->extension;
			}
			else
			{
				extension = "prg";
			}
		}
		else
		{
			extension = "dir";
		}

		// Don't show hidden folders or files
		//Debug_printv("size[%d] name[%s]", entry->size(), entry->name.c_str());

		std::string name = entry->petsciiName();
		mstr::toPETSCII(extension);

		if (entry->name[0]!='.' || m_show_hidden)
		{
			byte_count += sendLine(basicPtr, block_cnt, "%*s\"%s\"%*s %s", block_spc, "", name.c_str(), space_cnt, "", extension.c_str());
		}
		
		entry.reset(m_mfile->getNextFileInDir());

		ledToggle(true);
	}

	// Send Listing Footer
	byte_count += sendFooter(basicPtr, m_mfile->media_blocks_free, m_mfile->media_block_size);

	// End program with two zeros after last line. Last zero goes out as EOI.
	m_iec.send(0);
	m_iec.sendEOI(0);

	Debug_printf("=================================\r\n%d bytes sent\r\n", byte_count);

	ledON();
} // sendListing


uint16_t Interface::sendFooter(uint16_t &basicPtr, uint16_t blocks_free, uint16_t block_size)
{
	// Send List FOOTER
	// #if defined(USE_LITTLEFS)
	uint64_t byte_count = 0;
	if (block_size > 256)
	{
		byte_count = sendLine(basicPtr, blocks_free, "BLOCKS FREE. (*%d bytes)", block_size);
	}
	else
	{
		byte_count = sendLine(basicPtr, blocks_free, "BLOCKS FREE.");
	}
	
	// if (m_device.url().length() == 0)
	// {
	// 	FSInfo64 fs_info;
	// 	m_fileSystem->info64(fs_info);
	// 	blocks_free = fs_info.totalBytes - fs_info.usedBytes;
	// }
	
	return byte_count;
	// #elif defined(USE_SPIFFS)
	// 	return sendLine(basicPtr, 00, "UNKNOWN BLOCKS FREE.");
	// #endif
	//Debug_println("");
}


void Interface::sendFile()
{
	size_t i = 0;
	bool success = true;

	uint16_t bi = 0;
	uint16_t load_address = 0;
	uint16_t sys_address = 0;
	size_t b_len = 1;
	uint8_t b[b_len];

#ifdef DATA_STREAM
	char ba[9];
	ba[8] = '\0';
#endif

	// Update device database
	m_device.save();

	std::unique_ptr<MFile> file(MFSOwner::File(m_filename));

	if(!file->exists())
	{
		sendFileNotFound();
		return;
	}


	// TODO!!!! you should check istream for nullptr here and return error immediately if null

	
	if(
		mstr::equals(file->extension, "txt", false) ||
		mstr::equals(file->extension, "htm", false) ||
		mstr::equals(file->extension, "html", false)
		) 
	{
		// convert UTF8 files on the fly

		Debug_printv("Sending a text file to C64 [%s]", file->url.c_str());
        //std::unique_ptr<LinedReader> reader(new LinedReader(istream.get()));
		auto istream = Meat::ifstream(file.get());
		auto ostream = oiecstream();

		istream.open();
		ostream.open(&m_iec);

		if(!istream.is_open()) {
			sendFileNotFound();
			return;
		}

		//we can skip the BOM here, EF BB BF for UTF8
		auto b = (char)istream.get();
		if(b != 0xef)
			ostream.put(b);
		else {
			b = istream.get();
			if(b != 0xbb)
				ostream.put(b);
			else {
				b = istream.get();
				if(b != 0xbf)
					ostream.put(b); // not BOM
			}
		}

		while(!istream.eof()) {
			ledToggle(true);
			auto cp = istream.getUtf8();

			ostream.putUtf8(&cp);

			if(ostream.bad() || istream.bad()) {
				Debug_printv("Error sending");
                setDeviceStatus(60); // write error
				break;
            }
		}
		ostream.close();
		istream.close();
	}
	else
	{
		// Get file load address
		std::unique_ptr<MIStream> istream(file->inputStream());
		size_t len = istream->size() - 1;
		size_t avail = istream->available();

		i = 2;
		istream->read(b, b_len);
		success = m_iec.send(b[0]);
		load_address = *b & 0x00FF; // low byte
		sys_address = *b;
		istream->read(b, b_len);
		success = m_iec.send(b[0]);
		load_address = load_address | *b << 8;  // high byte
		sys_address += *b * 256;

		Debug_printf("sendFile: [%s] [$%.4X] (%d bytes)\r\n=================================\r\n", file->url.c_str(), load_address, len);
		while( len && success )
		{
			success = istream->read(b, b_len);
			if (success)
			{
	#ifdef DATA_STREAM
				if (bi == 0)
				{
					Debug_printf(":%.4X ", load_address);
					load_address += 8;
				}
	#endif
				if ( avail == 1 )
				{
					success = m_iec.sendEOI(b[0]); // indicate end of file.
				}
				else
				{
					success = m_iec.send(b[0]);
				}

	#ifdef DATA_STREAM
				// Show ASCII Data
				if (b[0] < 32 || b[0] >= 127) 
				b[0] = 46;

				ba[bi++] = b[0];

				if(bi == 8)
				{
					size_t t = (i * 100) / len;
					Debug_printf(" %s (%d %d%%) [%d]\r\n", ba, i, t, avail - 1);
					bi = 0;
				}
	#endif
			}

			// Exit if ATN is pulled while sending
			if ( m_iec.status(IEC_PIN_ATN) == IEC::IECline::pulled )
			{
				// TODO: If sending from a named channel save file pointer position
				setDeviceStatus(74);
				success = true;
				break;
			}

			// Toggle LED
			if (i % 50 == 0)
			{
				ledToggle(true);
			}

			avail = istream->available();
			i++;
		}
		istream->close();
		Debug_printf("=================================\r\n%d of %d bytes sent [SYS%d]\r\n", i, len, sys_address);
	}

	ledON();

	if (!success)
	{
		Debug_println("sendFile: Transfer aborted!");
		// TODO: Send something to signal that there was an error to the C64
	}
} // sendFile


void Interface::saveFile()
{
	uint16_t i = 0;
	bool done = false;

	uint16_t bi = 0;
	uint16_t load_address = 0;
	size_t b_len = 1;
	uint8_t b[b_len];
	uint8_t ll[b_len];
	uint8_t lh[b_len];

#ifdef DATA_STREAM
	char ba[9];
	ba[8] = '\0';
#endif

	mstr::toASCII(m_filename);
	std::unique_ptr<MFile> file(MFSOwner::File(m_filename));
	Debug_printv("[%s]", file->url.c_str());

	std::unique_ptr<MOStream> ostream(file->outputStream());
	

    if(!ostream->isOpen()) {
        Debug_printv("couldn't open a stream for writing");
		// TODO: Set status and sendFNF
		sendFileNotFound();
        return;
    }
    else 
	{
	 	// Stream is open!  Let's save this!

		// Get file load address
		ll[0] = m_iec.receive();
		load_address = *ll & 0x00FF; // low byte
		lh[0] = m_iec.receive();
		load_address = load_address | *lh << 8;  // high byte

		Debug_printf("saveFile: [%s] [$%.4X]\r\n=================================\r\n", file->url.c_str(), load_address);

		// Recieve bytes until a EOI is detected
		do
		{
			// Save Load Address
			if (i == 0)
			{
				Debug_print("[");
				ostream->write(ll, b_len);
				ostream->write(lh, b_len);
				i += 2;
				Debug_println("]");
			}

#ifdef DATA_STREAM
			if (bi == 0)
			{
				Debug_printf(":%.4X ", load_address);
				load_address += 8;
			}
#endif

			b[0] = m_iec.receive();
			ostream->write(b, b_len);
			i++;

			done = (m_iec.state() bitand IEC::eoiFlag) or (m_iec.state() bitand IEC::errorFlag);

#ifdef DATA_STREAM
			// Show ASCII Data
			if (b[0] < 32 || b[0] >= 127) 
			b[0] = 46;

			ba[bi++] = b[0];

			if(bi == 8)
			{
				Debug_printf(" %s (%d)\r\n", ba, i);
				bi = 0;
			}
#endif
			// Toggle LED
			if (i % 50 == 0)
			{
				ledToggle(true);
			}
		} while (not done);
    }
    ostream->close(); // nor required, closes automagically

	Debug_printf("=================================\r\n%d bytes saved\r\n", i);
	ledON();

	// TODO: Handle errorFlag
} // saveFile