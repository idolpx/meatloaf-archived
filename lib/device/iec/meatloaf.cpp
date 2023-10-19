#ifdef BUILD_IEC

#include "meatloaf.h"

#include <cstring>

#include <unordered_map>

#include "../../include/debug.h"
#include "../../include/cbm_defines.h"

#include "fnFsSD.h"
#include "fnWiFi.h"
#include "fnConfig.h"
#include "fujiCmd.h"

#include "led.h"
#include "led_strip.h"
#include "utils.h"

#include "cbm_media.h"


iecMeatloaf::iecMeatloaf()
{
    // device_active = false;
    device_active = true; // temporary during bring-up

    _base.reset( MFSOwner::File("/") );
    _last_file = "";
}

// Destructor
iecMeatloaf::~iecMeatloaf()
{

}


device_state_t iecMeatloaf::process()
{
    virtualDevice::process();

    if (commanddata.channel != 15)
    {
        Debug_printf("Meatloaf device only accepts on channel 15. Sending NOTFOUND.\r\n");
        device_state = DEVICE_ERROR;
        IEC.senderTimeout();
    }
    else if (commanddata.primary != IEC_UNLISTEN)
        return device_state;

    if (payload[0] > 0x7F)
        process_raw_commands();
    else
        process_basic_commands();

    return device_state;
}

// COMMODORE SPECIFIC CONVENIENCE COMMANDS /////////////////////

void iecMeatloaf::local_ip()
{
    char msg[17];

    fnSystem.Net.get_ip4_info(cfg.localIP, cfg.netmask, cfg.gateway);

    sprintf(msg, "%u.%u.%u.%u", cfg.localIP[0], cfg.localIP[1], cfg.localIP[2], cfg.localIP[3]);

    iecStatus.channel = 15;
    iecStatus.error = 0;
    iecStatus.msg = std::string(msg);
    iecStatus.connected = 0;
}


void iecMeatloaf::process_basic_commands()
{
    mstr::toASCII(payload);
    pt = util_tokenize(payload, ',');

    if (payload.find("adapterconfig") != std::string::npos)
        get_adapter_config();
    else if (payload.find("setssid") != std::string::npos)
        net_set_ssid();
    else if (payload.find("getssid") != std::string::npos)
        net_get_ssid();
    else if (payload.find("reset") != std::string::npos)
        reset_device();
    else if (payload.find("scanresult") != std::string::npos)
        net_scan_result();
    else if (payload.find("scan") != std::string::npos)
        net_scan_networks();
    else if (payload.find("wifistatus") != std::string::npos)
        net_get_wifi_status();
    // else if (payload.find("mounthost") != std::string::npos)
    //     mount_host();
    // else if (payload.find("mountdrive") != std::string::npos)
    //     disk_image_mount();
    // else if (payload.find("opendir") != std::string::npos)
    //     open_directory();
    // else if (payload.find("readdir") != std::string::npos)
    //     read_directory_entry();
    // else if (payload.find("closedir") != std::string::npos)
    //     close_directory();
    // else if (payload.find("gethost") != std::string::npos ||
    //          payload.find("flh") != std::string::npos)
    //     read_host_slots();
    // else if (payload.find("puthost") != std::string::npos ||
    //          payload.find("fhost") != std::string::npos)
    //     write_host_slots();
    // else if (payload.find("getdrive") != std::string::npos)
    //     read_device_slots();
    // else if (payload.find("unmounthost") != std::string::npos)
    //     unmount_host();
    // else if (payload.find("getdirpos") != std::string::npos)
    //     get_directory_position();
    // else if (payload.find("setdirpos") != std::string::npos)
    //     set_directory_position();
    // else if (payload.find("setdrivefilename") != std::string::npos)
    //     set_device_filename();
    else if (payload.find("writeappkey") != std::string::npos)
        write_app_key();
    else if (payload.find("readappkey") != std::string::npos)
        read_app_key();
    else if (payload.find("openappkey") != std::string::npos)
        open_app_key();
    else if (payload.find("closeappkey") != std::string::npos)
        close_app_key();
    // else if (payload.find("drivefilename") != std::string::npos)
    //     get_device_filename();
    else if (payload.find("bootconfig") != std::string::npos)
        set_boot_config();
    else if (payload.find("bootmode") != std::string::npos)
        set_boot_mode();
    // else if (payload.find("mountall") != std::string::npos)
    //     mount_all();
    else if (payload.find("localip") != std::string::npos)
        local_ip();
}

void iecMeatloaf::process_raw_commands()
{
    switch (payload[0])
    {
    case FUJICMD_RESET:
        reset_device();
        break;
    case FUJICMD_GET_SSID:
        net_get_ssid();
        break;
    case FUJICMD_SCAN_NETWORKS:
        net_scan_networks();
        break;
    case FUJICMD_GET_SCAN_RESULT:
        net_scan_result();
        break;
    case FUJICMD_SET_SSID:
        net_set_ssid();
        break;
    case FUJICMD_GET_WIFISTATUS:
        net_get_wifi_status();
        break;
    // case FUJICMD_MOUNT_HOST:
    //     mount_host();
    //     break;
    // case FUJICMD_MOUNT_IMAGE:
    //     disk_image_mount();
    //     break;
    // case FUJICMD_OPEN_DIRECTORY:
    //     open_directory();
    //     break;
    // case FUJICMD_READ_DIR_ENTRY:
    //     read_directory_entry();
    //     break;
    // case FUJICMD_CLOSE_DIRECTORY:
    //     close_directory();
    //     break;
    // case FUJICMD_READ_HOST_SLOTS:
    //     read_host_slots();
    //     break;
    // case FUJICMD_WRITE_HOST_SLOTS:
    //     write_host_slots();
    //     break;
    // case FUJICMD_READ_DEVICE_SLOTS:
    //     read_device_slots();
    //     break;
    // case FUJICMD_WRITE_DEVICE_SLOTS:
    //     write_device_slots();
    //     break;
    // case FUJICMD_ENABLE_UDPSTREAM:
    //     // Not implemented.
    //     break;
    // case FUJICMD_UNMOUNT_IMAGE:
    //     disk_image_umount();
    //     break;
    // case FUJICMD_UNMOUNT_HOST:
    //     unmount_host();
    //     break;
    case FUJICMD_GET_ADAPTERCONFIG:
        get_adapter_config();
        break;
    // case FUJICMD_GET_DIRECTORY_POSITION:
    //     get_directory_position();
    //     break;
    // case FUJICMD_SET_DIRECTORY_POSITION:
    //     set_directory_position();
    //     break;
    // case FUJICMD_SET_DEVICE_FULLPATH:
    //     set_device_filename();
    //     break;
    case FUJICMD_WRITE_APPKEY:
        write_app_key();
        break;
    case FUJICMD_READ_APPKEY:
        read_app_key();
        break;
    case FUJICMD_OPEN_APPKEY:
        open_app_key();
        break;
    case FUJICMD_CLOSE_APPKEY:
        close_app_key();
        break;
    // case FUJICMD_GET_DEVICE_FULLPATH:
    //     get_device_filename();
    //     break;
    case 0xD9:
        set_boot_config();
        break;
    case FUJICMD_SET_BOOT_MODE:
        set_boot_mode();
        break;
    // case FUJICMD_MOUNT_ALL:
    //     mount_all();
    //     break;
    }
}



// Reset Device
void iecMeatloaf::reset_device()
{
    // TODO IMPLEMENT
    fnSystem.reboot();
}

// Scan for networks
void iecMeatloaf::net_scan_networks()
{
    std::string r;
    char c[8];

    _countScannedSSIDs = fnWiFi.scan_networks();

    if (payload[0] == FUJICMD_SCAN_NETWORKS)
    {
        c[0] = _countScannedSSIDs;
        c[1] = 0;
        status_override = std::string(c);
    }
    else
    {
        iecStatus.error = _countScannedSSIDs;
        iecStatus.msg = "networks found";
        iecStatus.connected = 0;
        iecStatus.channel = 15;
    }
}

// Return scanned network entry
void iecMeatloaf::net_scan_result()
{
    std::vector<std::string> t = util_tokenize(payload, ',');

    // t[0] = SCANRESULT
    // t[1] = scan result # (0-numresults)
    struct
    {
        char ssid[33];
        uint8_t rssi;
    } detail;

    if (t.size() > 1)
    {
        int i = atoi(t[1].c_str());
        fnWiFi.get_scan_result(i, detail.ssid, &detail.rssi);
    }
    else
    {
        strcpy(detail.ssid, "INVALID SSID");
        detail.rssi = 0;
    }

    if (payload[0] == FUJICMD_GET_SCAN_RESULT) // raw
    {
        std::string r = std::string((const char *)&detail, sizeof(detail));
        status_override = r;
    }
    else // SCANRESULT,n
    {
        char c[40];
        std::string s = std::string(detail.ssid);
        mstr::toPETSCII(s);

        memset(c, 0, sizeof(c));

        iecStatus.error = detail.rssi;
        iecStatus.channel = 15;
        iecStatus.connected = false;
        iecStatus.msg = std::string(detail.ssid);
    }
}

//  Get SSID
void iecMeatloaf::net_get_ssid()
{
    struct
    {
        char ssid[MAX_SSID_LEN + 1];
        char password[64];
    } cfg;

    memset(&cfg, 0, sizeof(cfg));

    std::string s = Config.get_wifi_ssid();
    memcpy(cfg.ssid, s.c_str(),
           s.length() > sizeof(cfg.ssid) ? sizeof(cfg.ssid) : s.length());

    s = Config.get_wifi_passphrase();
    memcpy(cfg.password, s.c_str(),
           s.length() > sizeof(cfg.password) ? sizeof(cfg.password) : s.length());

    if (payload[0] == FUJICMD_GET_SSID)
    {
        std::string r = std::string((const char *)&cfg, sizeof(cfg));
        response_queue.push(r);
    }
    else // BASIC mode.
    {
        std::string r = std::string(cfg.ssid);
        mstr::toPETSCII(r);
        status_override = r;
    }
}

// Set SSID
void iecMeatloaf::net_set_ssid( bool store )
{
    Debug_println("Fuji cmd: SET SSID");

    // Data for  FUJICMD_SET_SSID
    // struct
    // {
    //     char ssid[MAX_SSID_LEN + 1];
    //     char password[64];
    // } cfg;

    if (payload[0] == FUJICMD_SET_SSID)
    {
        strncpy((char *)&cfg, payload.substr(12, std::string::npos).c_str(), sizeof(cfg));
    }
    else // easy BASIC form
    {
        std::string s = payload.substr(8, std::string::npos);
        std::vector<std::string> t = util_tokenize(s, ',');

        if (t.size() == 2)
        {
            if ( mstr::isNumeric( t[0] ) ) {
                // Find SSID by CRC8 Number
                t[0] = fnWiFi.get_network_name_by_crc8( std::stoi(t[0]) );
            }

            strncpy(cfg.ssid, t[0].c_str(), 33);
            strncpy(cfg.password, t[1].c_str(), 64);
        }
    }

    Debug_printf("Storing WiFi SSID and Password.\r\n");
    Config.store_wifi_ssid(cfg.ssid, sizeof(cfg.ssid));
    Config.store_wifi_passphrase(cfg.password, sizeof(cfg.password));
    Config.save();

    Debug_printf("Connecting to net %s\r\n", cfg.ssid);
    fnWiFi.connect(cfg.ssid, cfg.password);
    
    // Only save these if we're asked to, otherwise assume it was a test for connectivity
    if ( store && fnWiFi.connected() )
        net_store_ssid();

    iecStatus.channel = 15;
    iecStatus.error = 0;
    iecStatus.msg = "ssid set";
    iecStatus.connected = fnWiFi.connected();
}

void iecMeatloaf::net_store_ssid()
{
    // Only save these if we're asked to, otherwise assume it was a test for connectivity

    // 1. if this is a new SSID and not in the old stored, we should push the current one to the top of the stored configs, and everything else down.
    // 2. If this was already in the stored configs, push the stored one to the top, remove the new one from stored so it becomes current only.
    // 3. if this is same as current, then just save it again. User reconnected to current, nothing to change in stored. This is default if above don't happen

    int ssid_in_stored = -1;
    for (int i = 0; i < MAX_WIFI_STORED; i++)
    {
        if (Config.get_wifi_stored_ssid(i) == cfg.ssid)
        {
            ssid_in_stored = i;
            break;
        }
    }

    // case 1
    if (ssid_in_stored == -1 && Config.have_wifi_info() && Config.get_wifi_ssid() != cfg.ssid) {
        Debug_println("Case 1: Didn't find new ssid in stored, and it's new. Pushing everything down 1 and old current to 0");
        // Move enabled stored down one, last one will drop off
        for (int j = MAX_WIFI_STORED - 1; j > 0; j--)
        {
            bool enabled = Config.get_wifi_stored_enabled(j - 1);
            if (!enabled) continue;

            Config.store_wifi_stored_ssid(j, Config.get_wifi_stored_ssid(j - 1));
            Config.store_wifi_stored_passphrase(j, Config.get_wifi_stored_passphrase(j - 1));
            Config.store_wifi_stored_enabled(j, true); // already confirmed this is enabled
        }
        // push the current to the top of stored
        Config.store_wifi_stored_ssid(0, Config.get_wifi_ssid());
        Config.store_wifi_stored_passphrase(0, Config.get_wifi_passphrase());
        Config.store_wifi_stored_enabled(0, true);
    }

    // case 2
    if (ssid_in_stored != -1 && Config.have_wifi_info() && Config.get_wifi_ssid() != cfg.ssid) {
        Debug_printf("Case 2: Found new ssid in stored at %d, and it's not current (should never happen). Pushing everything down 1 and old current to 0\r\n", ssid_in_stored);
        // found the new SSID at ssid_in_stored, so move everything above it down one slot, and store the current at 0
        for (int j = ssid_in_stored; j > 0; j--)
        {
            Config.store_wifi_stored_ssid(j, Config.get_wifi_stored_ssid(j - 1));
            Config.store_wifi_stored_passphrase(j, Config.get_wifi_stored_passphrase(j - 1));
            Config.store_wifi_stored_enabled(j, true);
        }

        // push the current to the top of stored
        Config.store_wifi_stored_ssid(0, Config.get_wifi_ssid());
        Config.store_wifi_stored_passphrase(0, Config.get_wifi_passphrase());
        Config.store_wifi_stored_enabled(0, true);
    }

    // save the new SSID as current
    Config.store_wifi_ssid(cfg.ssid, sizeof(cfg.ssid));
    // Clear text here, it will be encrypted internally if enabled for encryption
    Config.store_wifi_passphrase(cfg.password, sizeof(cfg.password));

    Config.save();
}

// Get WiFi Status
void iecMeatloaf::net_get_wifi_status()
{
    uint8_t wifiStatus = fnWiFi.connected() ? 3 : 6;
    char r[4];

    Debug_printv("payload[0]==%02x\r\n", payload[0]);

    if (payload[0] == FUJICMD_GET_WIFISTATUS)
    {
        r[0] = wifiStatus;
        r[1] = 0;
        status_override = std::string(r);
        return;
    }
    else
    {
        iecStatus.error = wifiStatus;

        if (wifiStatus)
        {
            iecStatus.msg = "CONNECTED";
            iecStatus.connected = true;
        }
        else
        {
            iecStatus.msg = "DISCONNECTED";
            iecStatus.connected = false;
        }

        iecStatus.channel = 15;
    }
}

// Check if Wifi is enabled
void iecMeatloaf::net_get_wifi_enabled()
{
    // Not needed, will remove.
}

// Toggle boot config on/off, aux1=0 is disabled, aux1=1 is enabled
void iecMeatloaf::set_boot_config()
{
    if (payload[0] == FUJICMD_CONFIG_BOOT)
    {
        boot_config = payload[1];
    }
    else
    {
        std::vector<std::string> t = util_tokenize(payload, ':');

        if (t.size() < 2)
        {
            Debug_printf("Invalid # of parameters.\r\n");
            response_queue.push("error: invalid # of parameters\r");
            return;
        }

        boot_config = atoi(t[1].c_str());
    }
    response_queue.push("ok\r");
}

// Do SIO copy
void iecMeatloaf::copy_file()
{
    // TODO IMPLEMENT
}

// Set boot mode
void iecMeatloaf::set_boot_mode()
{
    if (payload[0] == FUJICMD_CONFIG_BOOT)
    {
        boot_config = payload[1];
    }
    else
    {
        std::vector<std::string> t = util_tokenize(payload, ':');

        if (t.size() < 2)
        {
            Debug_printf("Invalid # of parameters.\r\n");
            // send error
            response_queue.push("error: invalid # of parameters\r");
            return;
        }

        boot_config = true;
        insert_boot_device(atoi(t[1].c_str()));
    }
    response_queue.push("ok\r");
}

char *_generate_appkey_filename(appkey *info)
{
    static char filenamebuf[30];

    snprintf(filenamebuf, sizeof(filenamebuf), "/FujiNet/%04hx%02hhx%02hhx.key", info->creator, info->app, info->key);
    return filenamebuf;
}

/*
 Opens an "app key".  This just sets the needed app key parameters (creator, app, key, mode)
 for the subsequent expected read/write command. We could've added this information as part
 of the payload in a WRITE_APPKEY command, but there was no way to do this for READ_APPKEY.
 Requiring a separate OPEN command makes both the read and write commands behave similarly
 and leaves the possibity for a more robust/general file read/write function later.
*/
void iecMeatloaf::open_app_key()
{
    Debug_print("Fuji cmd: OPEN APPKEY\r\n");

    // The data expected for this command
    if (payload[0] == FUJICMD_OPEN_APPKEY)
        memcpy(&_current_appkey, &payload.c_str()[1], sizeof(_current_appkey));
    else
    {
        std::vector<std::string> t = util_tokenize(payload, ':');
        unsigned int val;

        if (t.size() < 5)
        {
            Debug_printf("Incorrect number of parameters.\r\n");
            response_queue.push("error: invalid # of parameters\r");
            // send error.
        }

        sscanf(t[1].c_str(), "%x", &val);
        _current_appkey.creator = (uint16_t)val;
        sscanf(t[2].c_str(), "%x", &val);
        _current_appkey.app = (uint8_t)val;
        sscanf(t[3].c_str(), "%x", &val);
        _current_appkey.key = (uint8_t)val;
        sscanf(t[4].c_str(), "%x", &val);
        _current_appkey.mode = (appkey_mode)val;
        _current_appkey.reserved = 0;
    }

    // We're only supporting writing to SD, so return an error if there's no SD mounted
    if (fnSDFAT.running() == false)
    {
        Debug_println("No SD mounted - returning error");
        // Send error
        response_queue.push("error: no sd card mounted\r");
        return;
    }

    // Basic check for valid data
    if (_current_appkey.creator == 0 || _current_appkey.mode == APPKEYMODE_INVALID)
    {
        Debug_println("Invalid app key data");
        // Send error.
        response_queue.push("error: invalid app key data\r");
        return;
    }

    Debug_printf("App key creator = 0x%04hx, app = 0x%02hhx, key = 0x%02hhx, mode = %hhu, filename = \"%s\"\r\n",
                 _current_appkey.creator, _current_appkey.app, _current_appkey.key, _current_appkey.mode,
                 _generate_appkey_filename(&_current_appkey));

    // Send complete
    response_queue.push("ok\r");
}

/*
  The app key close operation is a placeholder in case we want to provide more robust file
  read/write operations. Currently, the file is closed immediately after the read or write operation.
*/
void iecMeatloaf::close_app_key()
{
    Debug_print("Fuji cmd: CLOSE APPKEY\r\n");
    _current_appkey.creator = 0;
    _current_appkey.mode = APPKEYMODE_INVALID;
    response_queue.push("ok\r");
}

/*
 Write an "app key" to SD (ONLY!) storage.
*/
void iecMeatloaf::write_app_key()
{
    uint16_t keylen = -1;
    char value[MAX_APPKEY_LEN];

    if (payload[0] == FUJICMD_WRITE_APPKEY)
    {
        keylen = payload[1] & 0xFF;
        keylen |= payload[2] << 8;
        strncpy(value, &payload.c_str()[3], MAX_APPKEY_LEN);
    }
    else
    {
        std::vector<std::string> t = util_tokenize(payload, ':');
        if (t.size() > 3)
        {
            keylen = atoi(t[1].c_str());
            strncpy(value, t[2].c_str(), MAX_APPKEY_LEN);
        }
        else
        {
            // send error
            response_queue.push("error: invalid # of parameters\r");
            return;
        }
    }

    Debug_printf("Fuji cmd: WRITE APPKEY (keylen = %hu)\r\n", keylen);

    // Make sure we have valid app key information
    if (_current_appkey.creator == 0 || _current_appkey.mode != APPKEYMODE_WRITE)
    {
        Debug_println("Invalid app key metadata - aborting");
        // Send error
        response_queue.push("error: malformed appkey data\r");
        return;
    }

    // Make sure we have an SD card mounted
    if (fnSDFAT.running() == false)
    {
        Debug_println("No SD mounted - can't write app key");
        // Send error
        response_queue.push("error: no sd card mounted\r");
        return;
    }

    char *filename = _generate_appkey_filename(&_current_appkey);

    // Reset the app key data so we require calling APPKEY OPEN before another attempt
    _current_appkey.creator = 0;
    _current_appkey.mode = APPKEYMODE_INVALID;

    Debug_printf("Writing appkey to \"%s\"\r\n", filename);

    // Make sure we have a "/.app" directory, since that's where we're putting these files
    fnSDFAT.create_path("/.app");

    FILE *fOut = fnSDFAT.file_open(filename, "w");
    if (fOut == nullptr)
    {
        Debug_printf("Failed to open/create output file: errno=%d\r\n", errno);
        // Send error
        char e[8];
        itoa(errno, e, 10);
        response_queue.push("error: failed to create appkey file " + std::string(e) + "\r");
        return;
    }
    size_t count = fwrite(value, 1, keylen, fOut);

    fclose(fOut);

    if (count != keylen)
    {
        char e[128];
        sprintf(e, "error: only wrote %u bytes of expected %hu, errno=%d\r\n", count, keylen, errno);
        response_queue.push(std::string(e));
        // Send error
    }
    // Send ok
    response_queue.push("ok\r");
}

/*
 Read an "app key" from SD (ONLY!) storage
*/
void iecMeatloaf::read_app_key()
{
    Debug_println("Fuji cmd: READ APPKEY");

    // Make sure we have an SD card mounted
    if (fnSDFAT.running() == false)
    {
        Debug_println("No SD mounted - can't read app key");
        response_queue.push("error: no sd mounted\r\n");
        // Send error
        return;
    }

    // Make sure we have valid app key information
    if (_current_appkey.creator == 0 || _current_appkey.mode != APPKEYMODE_READ)
    {
        Debug_println("Invalid app key metadata - aborting");
        response_queue.push("error: invalid app key metadata\r");
        // Send error
        return;
    }

    char *filename = _generate_appkey_filename(&_current_appkey);

    Debug_printf("Reading appkey from \"%s\"\r\n", filename);

    FILE *fIn = fnSDFAT.file_open(filename, "r");
    if (fIn == nullptr)
    {
        char e[128];
        sprintf(e, "Failed to open input file: errno=%d\r\n", errno);
        // Send error
        response_queue.push(std::string(e));
        return;
    }

    struct
    {
        uint16_t size;
        uint8_t value[MAX_APPKEY_LEN];
    } __attribute__((packed)) response;
    memset(&response, 0, sizeof(response));

    size_t count = fread(response.value, 1, sizeof(response.value), fIn);

    fclose(fIn);
    Debug_printf("Read %d bytes from input file\r\n", count);

    response.size = count;

    if (payload[0] == FUJICMD_READ_APPKEY)
        response_queue.push(std::string((char *)&response, MAX_APPKEY_LEN));
    else
    {
        char reply[128];
        memset(reply, 0, sizeof(reply));
        snprintf(reply, sizeof(reply), "\"%04x\",\"%s\"", response.size, response.value);
        response_queue.push(std::string(reply));
    }
    response_queue.push("ok\r");
}

// Disk Image Rotate
/*
  We rotate disks my changing their disk device ID's. That prevents
  us from having to unmount and re-mount devices.
*/
void iecMeatloaf::image_rotate()
{
    // TODO IMPLEMENT
}

// This gets called when we're about to shutdown/reboot
void iecMeatloaf::shutdown()
{
    // TODO IMPLEMENT
}

// Get network adapter configuration
void iecMeatloaf::get_adapter_config()
{
    Debug_printf("get_adapter_config()\r\n");

    memset(&cfg, 0, sizeof(cfg));

    strlcpy(cfg.fn_version, fnSystem.get_fujinet_version(true), sizeof(cfg.fn_version));

    if (!fnWiFi.connected())
    {
        strlcpy(cfg.ssid, "NOT CONNECTED", sizeof(cfg.ssid));
        strlcpy(cfg.hostname, "NOT CONNECTED", sizeof(cfg.hostname));
    }
    else
    {
        strlcpy(cfg.hostname, fnSystem.Net.get_hostname().c_str(), sizeof(cfg.hostname));
        strlcpy(cfg.ssid, fnWiFi.get_current_ssid().c_str(), sizeof(cfg.ssid));
        fnWiFi.get_current_bssid(cfg.bssid);
        fnSystem.Net.get_ip4_info(cfg.localIP, cfg.netmask, cfg.gateway);
        fnSystem.Net.get_ip4_dns_info(cfg.dnsIP);
    }

    fnWiFi.get_mac(cfg.macAddress);

    if (payload[0] == FUJICMD_GET_ADAPTERCONFIG)
    {
        std::string reply = std::string((const char *)&cfg, sizeof(AdapterConfig));
        status_override = reply;
    }
    else if (payload == "ADAPTERCONFIG")
    {
        iecStatus.channel = 15;
        iecStatus.connected = fnWiFi.connected();
        iecStatus.error = 0;
        iecStatus.msg = "use localip, netmask, gateway, dns, mac, bssid, or version";
    }
}

// // Store host path prefix
// void iecMeatloaf::set_host_prefix()
// {
//     // TODO IMPLEMENT
// }

// // Retrieve host path prefix
// void iecMeatloaf::get_host_prefix()
// {
//     // TODO IMPLEMENT
// }

// Mounts the desired boot disk number
void iecMeatloaf::insert_boot_device(uint8_t d)
{
    // TODO IMPLEMENT
}


#endif /* BUILD_IEC */