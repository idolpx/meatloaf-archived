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

using namespace CBM;

namespace
{

	// Buffer for incoming and outgoing serial bytes and other stuff.
	char serCmdIOBuf[MAX_BYTES_PER_REQUEST];

} // unnamed namespace
Interface::Interface(IEC &iec)
	: m_iec(iec),
	m_atn_cmd(*reinterpret_cast<IEC::ATNCmd *>(&serCmdIOBuf[sizeof(serCmdIOBuf) / 2])),
	m_device(0)
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
	m_queuedError = ErrIntro;
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
	size_t i;

	std::string status = m_device_status;
	if (status.size() == 0)
		status = "00, OK, 00, 00";

	Debug_printv("status: %s", status.c_str());
	Debug_print("[");
	
	for (i = 0; i < status.length()-1; ++i)
		m_iec.send(status[i]);

	Debug_println("]");

	// ...and last byte in string as with EOI marker.
	m_iec.sendEOI(status[i]);

	// Clear the status message
	m_device_status.clear();
} // sendStatus

void Interface::setDeviceStatus(int number, int track, int sector)
{
	switch(number)
	{
		// 1 - FILES SCRATCHED - number scratched in track variable
		case 1:
			m_device_status = "01, FILES SCRATCHED, 00, 00";		
			break;
		// 20 READ ERROR
		case 20:
			m_device_status = "20, FILE NOT OPEN, 00, 00";		
			break;
		// 26 WRITE PROTECT ON
		case 26:
			m_device_status = "26, WRITE PROTECT ON, 00, 00";
			break;
		// 30 SYNTAX ERROR - in arguments
		case 30:
			m_device_status = "30, SYNTAX ERROR, 00, 00";
			break;
		// 31 SYNTAX ERROR - unknown command
		case 31:
			m_device_status = "31, SYNTAX ERROR, 00, 00";
			break;
		// 33 S.E. - invalid pattern
		case 33:
			m_device_status = "33, SYNTAX ERROR, 00, 00";
			break;
		// 34 S.E. - no file name given
		case 34:
			m_device_status = "34, SYNTAX ERROR, 00, 00";
			break;
		// 50 RECORD NOT PRESENT - eof
		case 50:
			m_device_status = "50, RECORD NOT PRESENT, 00, 00";
			break;
		// 60 WRITE FILE OPEN - trying to open for wrtiting a file that is open for writing
		case 60:
			m_device_status = "60, WRITE FILE OPEN, 00, 00";
			break;
		// 61 FILE NOT OPEN
		case 61:
			m_device_status = "61, FILE NOT OPEN, 00, 00";
			break;
		// 62 FILE NOT FOUND
		case 62:
			m_device_status = "62, FILE NOT FOUND, 00, 00";
			break;
		// 63 FILE EXISTS
		case 63:
			m_device_status = "63, FILE EXISTS, 00, 00";
			break;
		// 65 NO BLOCK - for B-A
		case 65:
			m_device_status = "65, NO BLOCK, 00, 00";
			break;
		// 73 boot message: device name, rom version etc.
		case 73:
			m_device_status = "73, " PRODUCT_ID " [" FW_VERSION "], 00, 00";
			break;
		// 74 DRIVE NOT READY - also drive out of memory
		case 74:
			m_device_status = "74, DRIVE NOT READY, 00, 00";
			break;
		// 77 SELECTED PARTITION ILLEGAL
		case 77:
			m_device_status = "77, SELECTED PARTITION ILLEGAL, 00, 00";
			break;
		// 78 BUFFER TOO SMALL (sd2iec buffer ops)
		case 78:
			m_device_status = "78, BUFFER TOO SMALL, 00, 00";
			break;
		// 8x 64HDD errors
		// case 8:
		// 	break;
		// 89 COMMAND NOT SUPPORTED
		case 89:
			m_device_status = "89, COMMAND NOT SUPPORTED, 00, 00";
			break;
		// 91 64HDD device activated
		case 91:
			m_device_status = "91, DEVICE ACTIVATED, 00, 00";
			break;
		case 125:
			m_device_status = "125, NETWORK TIMEOUT, 00, 00";
			break;
		case 126:
			m_device_status = "126, NODE NOT FOUND, 00, 00";
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
	sendLine(basicPtr, 0, CBM_DEL_DEL "DEVICE    : %d", m_device.device());
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

byte Interface::loop(void)
{
	//#ifdef HAS_RESET_LINE
	//	if(m_iec.checkRESET()) {
	//		// IEC reset line is in reset state, so we should set all states in reset.
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
	IEC::ATNCheck retATN = m_iec.checkATN(m_atn_cmd);
	//	interrupts();

	if (retATN == IEC::ATN_ERROR)
	{
		//Debug_printf("\r\n[ERROR]");
		reset();
		retATN = IEC::ATN_IDLE;
	}
	// Did anything happen from the host side?
	else if (retATN not_eq IEC::ATN_IDLE)
	{

		switch (m_atn_cmd.command)
		{
		case IEC::ATN_CODE_OPEN:
			if (m_atn_cmd.channel == 0)
				Debug_printf("\r\n[OPEN] LOAD \"%s\",%d ", m_atn_cmd.str, m_atn_cmd.device);
			if (m_atn_cmd.channel == 1)
				Debug_printf("\r\n[OPEN] SAVE \"%s\",%d ", m_atn_cmd.str, m_atn_cmd.device);

			// Open either file or prg for reading, writing or single line command on the command channel.
			// In any case we just issue an 'OPEN' to the host and let it process.
			// Note: Some of the host response handling is done LATER, since we will get a TALK or LISTEN after this.
			// Also, simply issuing the request to the host and not waiting for any response here makes us more
			// responsive to the CBM here, when the DATA with TALK or LISTEN comes in the next sequence.
			handleATNCmdCodeOpen(m_atn_cmd);
			break;

		case IEC::ATN_CODE_DATA: // data channel opened
			Debug_printf("\r\n[DATA] ");
			if (retATN == IEC::ATN_CMD_TALK)
			{
				// when the CMD channel is read (status), we first need to issue the host request. The data channel is opened directly.
				if (m_atn_cmd.channel == CMD_CHANNEL)
					handleATNCmdCodeOpen(m_atn_cmd);		 // This is typically an empty command,
				handleATNCmdCodeDataTalk(m_atn_cmd.channel); // ...but we do expect a response from PC that we can send back to CBM.
			}
			else if (retATN == IEC::ATN_CMD_LISTEN)
				handleATNCmdCodeDataListen();
			else if (retATN == IEC::ATN_CMD)	 // Here we are sending a command to PC and executing it, but not sending response
				handleATNCmdCodeOpen(m_atn_cmd); // back to CBM, the result code of the command is however buffered on the PC side.
			break;

		case IEC::ATN_CODE_CLOSE:
			Debug_printf("\r\n[CLOSE] ");
			// handle close with host.
			handleATNCmdClose();
			break;

		case IEC::ATN_CODE_LISTEN:
			Debug_printf("\r\n[LISTEN] ");
			break;
		case IEC::ATN_CODE_TALK:
			Debug_printf("\r\n[TALK] ");
			break;
		case IEC::ATN_CODE_UNLISTEN:
			Debug_printf("\r\n[UNLISTEN] ");
			break;
		case IEC::ATN_CODE_UNTALK:
			Debug_printf("\r\n[UNTALK] ");
			break;
		} // switch
	}	  // IEC not idle

	return retATN;
} // handler

MFile* Interface::guessIncomingPath(std::string commandLne)
{
	std::string guessedPath = commandLne;

	Debug_printv("[%s]", guessedPath.c_str());

	// get the current directory
	std::shared_ptr<MFile> currentDir = m_mfile;
	Debug_printv("currentDir[%s]", currentDir->url.c_str());

	// check to see if it starts with a known command token
	if(mstr::startsWith(commandLne, "CD:", false) || mstr::startsWith(commandLne, "CD/", false)) // would be case sensitive, but I don't know the proper case
	{
		guessedPath = mstr::drop(guessedPath, 3);
		// if ( mstr::startsWith(guessedPath, ":" ) ) // drop ":" if it was specified
		// 	guessedPath = mstr::drop(guessedPath, 1);
	}
	// TODO more of them?

	// NOW, since user could have requested ANY kind of our supported magic paths like:
	// LOAD ~/something
	// LOAD ../something
	// LOAD //something
	// we HAVE TO PARSE IT OUR WAY!

	Debug_printv("guessedPath[%s]", guessedPath.c_str());
	
	// and to get a REAL FULL PATH that the user wanted to refer to, we CD into it, using supplied stripped path:
	return currentDir->cd(guessedPath);
}

void Interface::handleATNCmdCodeOpen(IEC::ATNCmd &atn_cmd)
{
	if (m_device.select(atn_cmd.device))
	{
		m_mfile.reset(MFSOwner::File(m_device.url()));
	}

	std::string command = atn_cmd.str;
	m_openState = O_NOTHING;

	//Serial.printf("\r\n$IEC: DEVICE[%d] DRIVE[%d] PARTITION[%d] URL[%s] PATH[%s] IMAGE[%s] FILENAME[%s] FILETYPE[%s] COMMAND[%s]\r\n", m_device.device(), m_device.drive(), m_device.partition(), m_device.url().c_str(), m_device.path().c_str(), m_device.image().c_str(), m_filename.c_str(), m_filetype.c_str(), atn_cmd.str);

	if (mstr::endsWith(command, "*"))
	{
		// Find first program in listing
		if (m_device.path().empty())
		{
			// If in LittleFS root then set it to FB64
			// TODO: Load configured autoload program
			m_mfile.reset(MFSOwner::File("/.sys/fb64"));
		}
		else if (m_filename_last.size() > 0)
		{
			// Load last used file
			m_mfile.reset(MFSOwner::File(m_filename_last));
		}
		else
		{	
			// Find first PRG file in current directory
			std::shared_ptr<MFile> entry(m_mfile->getNextFileInDir());

			while (entry != nullptr && entry->isDirectory())
			{
				Debug_printv("extension: [%s]", entry->extension);
				entry.reset(m_mfile->getNextFileInDir());
			}
			m_mfile = entry;
		}
		m_openState = O_FILE;
		Debug_printv("LOAD * [%s]", m_mfile->url.c_str());
	}
	else if (mstr::startsWith(command, "@INFO", false))
	{
		m_openState = O_DEVICE_INFO;
	}
	else if (mstr::startsWith(command, "@STAT", false))
	{
		m_openState = O_DEVICE_STATUS;
	}
	if (mstr::startsWith(command, "$"))
	{
		m_openState = O_DIR;
		Debug_printv("LOAD $");
	}	
	else
	{
		// we need this because if user came here via LOAD"CD//somepath" then we'll end up with
		// some shit in check variable!
		std::shared_ptr<MFile> new_mfile(guessIncomingPath(command));

		m_device.url(new_mfile->url);
		if (mstr::equals(new_mfile->extension,"URL")) 
		{
			new_mfile.reset(new_mfile->cd("dummy"));
			m_device.url(new_mfile->url);
			m_openState = O_DIR;
			Debug_printv("CD into URL file [%s]", new_mfile->url.c_str());
			Debug_printv("LOAD $");
		}
		if (mstr::startsWith(command, "CD", false) || new_mfile->isDirectory())
		{
			// Enter directory
			m_device.url(new_mfile->url);
			m_mfile = new_mfile;
			m_openState = O_DIR;
			Debug_printv("CD [%s]", new_mfile->url.c_str());
			Debug_printv("LOAD $");
		}
		else
		{
			m_device.path(new_mfile->parent()->url);
			m_filename_last = m_mfile->url;
			m_mfile = new_mfile;
			m_openState = O_FILE;
			Debug_printv("Load File [%s]", m_mfile->url.c_str());
		}
	}


	if (m_openState == O_DIR)
	{
		m_atn_cmd.str[0] = '\0';
		m_atn_cmd.strLen = 0;
	}

	//Debug_printf("\r\nhandleATNCmdCodeOpen: %d (M_OPENSTATE) [%s]", m_openState, m_atn_cmd.str);
	Serial.printf("\r\nDEVICE[%d]\nMEDIA[%d]\nPARTITION[%d]\nURL[%s]\nPATH[%s]\nFILE[%s]\nCOMMAND[%s]\r\n", 
					m_device.device(), 
					m_device.media(), 
					m_device.partition(), 
					m_device.url().c_str(), 
					m_device.path().c_str(),
					m_mfile->url.c_str(),
					atn_cmd.str
	);
	
	// Debug_println("");
	// Debug_printv("-------------------------------");
	// Debug_printv("URL: [%s]", m_mfile->url.c_str());
    // Debug_printv("streamPath: [%s]", m_mfile->streamPath.c_str());
    // Debug_printv("pathInStream: [%s]", m_mfile->pathInStream.c_str());
	// Debug_printv("Scheme: [%s]", m_mfile->scheme.c_str());
	// Debug_printv("Username: [%s]", m_mfile->user.c_str());
	// Debug_printv("Password: [%s]", m_mfile->pass.c_str());
	// Debug_printv("Host: [%s]", m_mfile->host.c_str());
	// Debug_printv("Port: [%s]", m_mfile->port.c_str());
	// Debug_printv("Path: [%s]", m_mfile->path.c_str());
	// Debug_printv("File: [%s]", m_mfile->name.c_str());
	// Debug_printv("Extension: [%s]", m_mfile->extension.c_str());
    // Debug_printv("-------------------------------");

} // handleATNCmdCodeOpen

void Interface::handleATNCmdCodeDataTalk(byte chan)
{
	// process response into m_queuedError.
	// Response: ><code in binary><CR>

	Debug_printf("\r\nhandleATNCmdCodeDataTalk: %d (CHANNEL) %d (M_OPENSTATE)", chan, m_openState);

	if (chan == CMD_CHANNEL)
	{
		// Send status message
		sendStatus();
		// go back to OK state, we have dispatched the error to IEC host now.
		m_queuedError = ErrOK;
	}
	else
	{

		//Debug_printf("\r\nm_openState: %d", m_openState);

		switch (m_openState)
		{
		case O_NOTHING:
			// Say file not found
			sendFileNotFound();
			break;

		case O_INFO:
			// Reset and send SD card info
			reset();
			sendListing();
			break;

		case O_FILE:
			// Send file
			sendFile();
			break;

		case O_DIR:
			// Send listing
			sendListing();
			break;

		case O_FILE_ERR:
			sendFileNotFound();
			break;

		case O_DEVICE_INFO:
			// Send device info
			sendDeviceInfo();
			break;

		case O_DEVICE_STATUS:
			// Send device info
			sendDeviceStatus();
			break;
		}
	}

} // handleATNCmdCodeDataTalk

void Interface::handleATNCmdCodeDataListen()
{
	Debug_printv("[%s]", m_device.url().c_str());

	saveFile();
} // handleATNCmdCodeDataListen

void Interface::handleATNCmdClose()
{
	Debug_printf("\r\nhandleATNCmdClose: Success!");

	//Serial.printf("\r\nIEC: DEVICE[%d] DRIVE[%d] PARTITION[%d] URL[%s] PATH[%s] IMAGE[%s] FILENAME[%s] FILETYPE[%s]\r\n", m_device.device(), m_device.drive(), m_device.partition(), m_device.url().c_str(), m_device.path().c_str(), m_device.image().c_str(), m_filename.c_str(), m_filetype.c_str());
	Debug_printf("\r\n=================================\r\n\r\n");

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

	// Send List HEADER
	//byte_count += sendLine(basicPtr, 0, "\x12\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", m_device.device());
	byte_count += sendLine(basicPtr, 0, CBM_REVERSE_ON "%s", header.c_str());

	// Send Extra INFO
	if (m_device.url().size())
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[URL]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, m_device.url().c_str());
		sent_info = true;
	}
	if (m_device.path().size() > 1)
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[PATH]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, m_device.path().c_str());
		sent_info = true;
	}
	// if (m_device.archive().length() > 1)
	// {
	// 	byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[ARCHIVE]");
	// 	byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, m_device.archive().c_str());
	// }
	if (m_device.image().size())
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[IMAGE]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, m_device.image().c_str());
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
	Debug_printf("\r\nsendListing:\r\n");

	uint16_t byte_count = 0;
	std::string extension = "DIR";

	// Send List ITEMS
	std::shared_ptr<MFile> dir(m_mfile.get());
	if(!dir->isDirectory())
		dir.reset(m_mfile->parent());

	std::unique_ptr<MFile> entry(dir->getNextFileInDir());

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
		sprintf(buffer, "\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", m_device.device());
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
		uint16_t block_cnt = entry->size() / dir->media_block_size;
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
				extension = "PRG";
			}
		}
		else
		{
			extension = "DIR";
		}

		// Don't show hidden folders or files
		//Debug_printv("size[%d] name[%s]", entry->size(), entry->name.c_str());
		if (entry->name[0]!='.' || m_show_hidden)
		{
			byte_count += sendLine(basicPtr, block_cnt, "%*s\"%s\"%*s %3s", block_spc, "", entry->name.c_str(), space_cnt, "", extension.c_str());
		}
		
		entry.reset(dir->getNextFileInDir());

		ledToggle(true);
	}

	// Send Listing Footer
	byte_count += sendFooter(basicPtr, dir->media_blocks_free, dir->media_block_size);

	// End program with two zeros after last line. Last zero goes out as EOI.
	m_iec.send(0);
	m_iec.sendEOI(0);

	Debug_printf("\r\nsendListing: %d Bytes Sent\r\n", byte_count);

	ledON();
} // sendListing


uint16_t Interface::sendFooter(uint16_t &basicPtr, uint16_t blocks_free, uint16_t block_size)
{
	// Send List FOOTER
	// #if defined(USE_LITTLEFS)
	uint64_t byte_count = 0;
	if (block_size > 256)
	{
		//String bytes = String(" BYTES");
		byte_count += sendLine(basicPtr, 0, "%*s\"-------------------\" NFO", 0, "");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "[BLOCK SIZE]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 0, "", 19, "1024 BYTES");
		byte_count += sendLine(basicPtr, 0, "%*s\"===================\" NFO", 0, "");
	}
	
	// if (m_device.url().length() == 0)
	// {
	// 	FSInfo64 fs_info;
	// 	m_fileSystem->info64(fs_info);
	// 	blocks_free = fs_info.totalBytes - fs_info.usedBytes;
	// }
	byte_count = sendLine(basicPtr, blocks_free, "BLOCKS FREE.");
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
	size_t b_len = 1;
	uint8_t b[b_len];

#ifdef DATA_STREAM
	char ba[9];
	ba[8] = '\0';
#endif

	// Update device database
	m_device.save();

	Debug_printv("[%s]", m_mfile->url.c_str());

	if(!m_mfile->exists())
	{
		sendFileNotFound();
		return;
	}

	std::shared_ptr<MIStream> istream(m_mfile->inputStream());
	size_t len = istream->available() - 1;

	// Get file load address
	istream->read(b, b_len);
	success = m_iec.send(b[0]);
	load_address = *b & 0x00FF; // low byte
	istream->read(b, b_len);
	success = m_iec.send(b[0]);
	load_address = load_address | *b << 8;  // high byte

	Debug_printf("\r\nsendFile: [%s] [$%.4X] (%d bytes)\r\n=================================\r\n", m_mfile->url.c_str(), load_address, len);
	for (i = 2; success and i <= len; i++)
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
			if (i == len)
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
				Debug_printf(" %s (%d %d%%)\r\n", ba, i, t);
				bi = 0;
			}
#endif
			// Toggle LED
			if (i % 50 == 0)
			{
				ledToggle(true);
			}
		}

		// Exit if ATN is pulled while sending
		if ( m_iec.status(IEC_PIN_ATN) == IEC::IECline::pulled )
		{
			success = true;
			break;
		}
	}
	istream->close();
	Debug_println("");
	Debug_printf("%d of %d bytes sent\r\n", i, len);

	ledON();

	if (!success || i != len)
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

	std::shared_ptr<MOStream> ostream(m_mfile->outputStream());
	Debug_printf("\r\nsaveFile: [%s]\r\n=================================\r\nLOAD ADDRESS [ ", m_mfile->url.c_str());

    if(!ostream->isOpen()) {
        Debug_printv("couldn't open a stream for writing");
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

		// Recieve bytes until a EOI is detected
		do
		{
			// Save Load Address
			if (i == 0)
			{
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

	Debug_printf("\n%d bytes received\n", i);
	ledON();

	// TODO: Handle errorFlag
} // saveFile