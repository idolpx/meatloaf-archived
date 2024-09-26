#include "meatloaf.h"

#include "../../include/debug.h"

void devMeatloaf::sendMeatloafSystemInformation()
{
	Debug_printf("\r\nsendDeviceInfo:\r\n");

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// #if defined(USE_LITTLEFS)
	// FSInfo64 fs_info;
	// m_fileSystem->info64(fs_info);
	// #endif
//	char floatBuffer[10]; // buffer
//	dtostrf(getFragmentation(), 3, 2, floatBuffer);

	// Send load address
	IEC.send(C64_BASIC_START bitand 0xff);
	IEC.send((C64_BASIC_START >> 8) bitand 0xff);
	Debug_println("");

	// Send List HEADER
	sendLine(basicPtr, 0, CBM_DEL_DEL CBM_REVERSE_ON " %s V%s ", PRODUCT_ID, FW_VERSION);

	// CPU
	sendLine(basicPtr, 0, CBM_DEL_DEL "SYSTEM ---");
//	String sdk = String(ESP.getSdkVersion());
//	sdk.toUpperCase();
//	sendLine(basicPtr, 0, CBM_DEL_DEL "SDK VER    : %s", sdk.c_str());
	//sendLine(basicPtr, 0, "BOOT VER   : %08X", ESP.getBootVersion());
	//sendLine(basicPtr, 0, "BOOT MODE  : %08X", ESP.getBootMode());
	//sendLine(basicPtr, 0, "CHIP ID    : %08X", ESP.getChipId());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "CPU MHZ    : %d MHZ", ESP.getCpuFreqMHz());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "CYCLES     : %u", ESP.getCycleCount());

	// POWER
	sendLine(basicPtr, 0, CBM_DEL_DEL "POWER ---");
	//sendLine(basicPtr, 0, "VOLTAGE    : %d.%d V", ( ESP.getVcc() / 1000 ), ( ESP.getVcc() % 1000 ));

	// RAM
	sendLine(basicPtr, 0, CBM_DEL_DEL "MEMORY ---");
//	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM SIZE   : %5d B", getTotalMemory());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM FREE   : %5d B", getTotalAvailableMemory());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM >BLK   : %5d B", getLargestAvailableBlock());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "RAM FRAG   : %s %%", floatBuffer);

	// ROM
//	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM SIZE   : %5d B", ESP.getSketchSize() + ESP.getFreeSketchSpace());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM USED   : %5d B", ESP.getSketchSize());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "ROM FREE   : %5d B", ESP.getFreeSketchSpace());

	// FLASH
	sendLine(basicPtr, 0, CBM_DEL_DEL "STORAGE ---");
//	sendLine(basicPtr, 0, "FLASH SIZE : %5d B", ESP.getFlashChipRealSize());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "FLASH SPEED: %d MHZ", (ESP.getFlashChipSpeed() / 1000000));

	// // FILE SYSTEM
	// sendLine(basicPtr, 0, CBM_DEL_DEL "FILE SYSTEM ---");
	// sendLine(basicPtr, 0, CBM_DEL_DEL "TYPE       : %s", FS_TYPE);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "SIZE       : %5d B", fs_info.totalBytes);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "USED       : %5d B", fs_info.usedBytes);
	// sendLine(basicPtr, 0, CBM_DEL_DEL "FREE       : %5d B", fs_info.totalBytes - fs_info.usedBytes);

	// NETWORK
	sendLine(basicPtr, 0, CBM_DEL_DEL "NETWORK ---");
//	char ip[16];
//	sprintf(ip, "%s", ipToString(WiFi.softAPIP()).c_str());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "AP MAC     : %s", WiFi.softAPmacAddress().c_str());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "AP IP      : %s", ip);
//	sprintf(ip, "%s", ipToString(WiFi.localIP()).c_str());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "STA MAC    : %s", WiFi.macAddress().c_str());
//	sendLine(basicPtr, 0, CBM_DEL_DEL "STA IP     : %s", ip);

	// End program with two zeros after last line. Last zero goes out as EOI.
	IEC.send(0);
	IEC.sendEOI(0);

	fnLedManager.set(eLed::LED_BUS, true);
} // sendMeatloafSystemInformation

void devMeatloaf::sendMeatloafVirtualDeviceStatus()
{
	Debug_printf("\r\nsendDeviceStatus:\r\n");

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	IEC.send(C64_BASIC_START bitand 0xff);
	IEC.send((C64_BASIC_START >> 8) bitand 0xff);
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
	IEC.send(0);
	IEC.sendEOI(0);

	fnLedManager.set(eLed::LED_BUS, true);
} // sendMeatloafVirtualDeviceStatus