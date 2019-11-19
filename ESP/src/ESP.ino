//////////////////////////////////////////////////////////////////////////////////
/* 
    Dreamcast Companion App
*/
#include "global.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SPIFlash.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <Task.h>
#include "task/FlashTask.h"
#include "task/FlashVerifyTask.h"
#include "task/FlashESPTask.h"
#include "task/FlashESPIndexTask.h"
#include "task/FPGATask.h"
#include "task/DebugTask.h"
#include "task/TimeoutTask.h"
#include "task/FlashCheckTask.h"
#include "task/FlashEraseTask.h"
#include "task/InfoTask.h"
#include "util.h"
#include "data.h"
#include "web.h"
#include "Menu.h"
#include "pwgen.h"

//////////////////////////////////////////////////////////////////////////////////

bool isHhttpAuthPassGenerated = false;
bool isRelaxedFirmware = false;

char ssid[64] = DEFAULT_SSID;
char password[64] = DEFAULT_PASSWORD;
char otaPassword[64] = DEFAULT_OTA_PASSWORD; 
char firmwareServer[256] = DEFAULT_FW_SERVER;
char firmwareServerPath[256] = DEFAULT_FW_SERVER_PATH;
char firmwareVersion[64] = DEFAULT_FW_VERSION;
char firmwareVariant[64] = DEFAULT_FW_VARIANT;
char httpAuthUser[64] = DEFAULT_HTTP_USER;
char httpAuthPass[64] = "";
char confIPAddr[24] = DEFAULT_CONF_IP_ADDR;
char confIPGateway[24] = DEFAULT_CONF_IP_GATEWAY;
char confIPMask[24] = DEFAULT_CONF_IP_MASK;
char confIPDNS[24] = DEFAULT_CONF_IP_DNS;
char host[64] = DEFAULT_HOST;
char videoMode[16] = "";
char configuredResolution[16] = "";
char resetMode[16] = "";
char deinterlaceMode480i[16] = "";
char deinterlaceMode576i[16] = "";
char protectedMode[8] = "";
char AP_NameChar[64];
char WiFiAPPSK[12] = "";
char keyboardLayout[8] = DEFAULT_KEYBOARD_LAYOUT; 
IPAddress ipAddress( 192, 168, 4, 1 );
bool inInitialSetupMode = false;
AsyncWebServer server(80);
SPIFlash flash(CS);
int last_error = NO_ERROR; 
int totalLength;
int readLength;
bool currentJobDone = true;

File flashFile;

uint8_t CurrentResolution = RESOLUTION_1080p;
uint8_t PrevCurrentResolution;
uint8_t CurrentResolutionData = 0;
uint8_t ForceVGA = VGA_ON;
uint8_t CurrentResetMode = RESET_MODE_LED;
uint8_t CurrentDeinterlaceMode480i = DEINTERLACE_MODE_BOB;
uint8_t CurrentDeinterlaceMode576i = DEINTERLACE_MODE_BOB;
uint8_t CurrentProtectedMode = PROTECTED_MODE_OFF;
int8_t Offset240p;
int8_t OffsetVGA;
int8_t AutoOffsetVGA = 0;
uint8_t UpscalingMode;
uint8_t ColorExpansionMode;
uint8_t GammaMode;
uint8_t ColorSpace;

char md5FPGA[48];
char md5ESP[48];
char md5IndexHtml[48];

bool scanlinesActive;
int scanlinesIntensity;
bool scanlinesOddeven;
bool scanlinesThickness;

bool reflashNeccessary;
bool reflashNeccessary2;
bool reflashNeccessary3;

MD5Builder md5;
TaskManager taskManager;
FlashTask flashTask(1);
FlashESPTask flashESPTask(1);
FlashESPIndexTask flashESPIndexTask(1);
DebugTask debugTask(16);
TimeoutTask timeoutTask(MsToTaskTime(100));
FlashCheckTask flashCheckTask(1, NULL);
FlashVerifyTask flashVerifyTask(1);
FlashEraseTask flashEraseTask(1);
InfoTask infoTask(16);

extern Menu mainMenu;
Menu *currentMenu;
Menu *previousMenu;

//////////////////////////////////////////////////////////////////////////////////

void setupSPIFFS() {
    DEBUG2(">> Setting up SPIFFS...\n");
    if (!SPIFFS.begin()) {
        DEBUG(">> SPIFFS begin failed, trying to format...");
        if (SPIFFS.format()) {
            DEBUG("done.\n");
        } else {
            DEBUG("error.\n");
        }
    }
}

void setupResetMode() {
    readCurrentResetMode();
    DEBUG2(">> Setting up reset mode: %x\n", CurrentResetMode);
    reflashNeccessary3 = !forceI2CWrite(
        I2C_RESET_CONF, CurrentResetMode, 
        I2C_PING, 0
    );
}

void setupOutputResolution() {
    readVideoMode();
    readCurrentResolution();
    readCurrentDeinterlaceMode480i();
    readCurrentDeinterlaceMode576i();

    DEBUG2(">> Setting up output resolution: %x\n", ForceVGA | CurrentResolution);
    reflashNeccessary = !forceI2CWrite(
        I2C_OUTPUT_RESOLUTION, ForceVGA | mapResolution(CurrentResolution),
        //ForceVGA ? I2C_DC_RESET : I2C_PING, 0
        I2C_DC_RESET, 0
    );
}

void setupScanlines() {
    readScanlinesActive();
    readScanlinesIntensity();
    readScanlinesOddeven();
    readScanlinesThickness();

    uint8_t upper = getScanlinesUpperPart();
    uint8_t lower = getScanlinesLowerPart();

    DEBUG2(">> Setting up scanlines:\n");
    reflashNeccessary2 = !forceI2CWrite(
        I2C_SCANLINE_UPPER, upper, 
        I2C_SCANLINE_LOWER, lower
    );
}

void setupOffsets() {
    read240pOffset();
    readVGAOffset();
    forceI2CWrite(I2C_240P_OFFSET, Offset240p, I2C_240P_OFFSET, Offset240p);
    forceI2CWrite(I2C_VGA_OFFSET, getEffectiveOffsetVGA(), I2C_VGA_OFFSET, getEffectiveOffsetVGA());
}

void setupUpscalingMode() {
    readUpscalingMode();
    forceI2CWrite(
        I2C_UPSCALING_MODE, UpscalingMode,
        I2C_UPSCALING_MODE, UpscalingMode
    );
}

void setupColorExpansionAndGamma() {
    readColorExpansionMode();
    readGammaMode();
    forceI2CWrite(
        I2C_COLOR_EXPANSION_AND_GAMMA_MODE, ColorExpansionMode | GammaMode << 3,
        I2C_COLOR_EXPANSION_AND_GAMMA_MODE, ColorExpansionMode | GammaMode << 3
    );
}

void setupColorSpace() {
    readColorSpace();
    forceI2CWrite(
        I2C_COLOR_SPACE, ColorSpace,
        I2C_COLOR_SPACE, ColorSpace
    );
}

void setupTaskManager() {
    DEBUG2(">> Setting up task manager...\n");
    taskManager.Setup();
    taskManager.StartTask(&fpgaTask);
}

void setupCredentials(void) {
    DEBUG2(">> Reading stored values...\n");

    _readFile("/etc/ssid", ssid, 64, DEFAULT_SSID);
    _readFile("/etc/password", password, 64, DEFAULT_PASSWORD);
    _readFile("/etc/ota_pass", otaPassword, 64, DEFAULT_OTA_PASSWORD);
    _readFile("/etc/firmware_server", firmwareServer, 256, DEFAULT_FW_SERVER);
    _readFile("/etc/firmware_server_path", firmwareServerPath, 256, DEFAULT_FW_SERVER_PATH);
    _readFile("/etc/firmware_variant", firmwareVariant, 64, DEFAULT_FW_VARIANT);
    _readFile("/etc/firmware_version", firmwareVersion, 64, DEFAULT_FW_VERSION);
    _readFile("/etc/http_auth_user", httpAuthUser, 64, DEFAULT_HTTP_USER);
    _readFile("/etc/http_auth_pass", httpAuthPass, 64, DEFAULT_HTTP_PASS);
    _readFile("/etc/conf_ip_addr", confIPAddr, 24, DEFAULT_CONF_IP_ADDR);
    _readFile("/etc/conf_ip_gateway", confIPGateway, 24, DEFAULT_CONF_IP_GATEWAY);
    _readFile("/etc/conf_ip_mask", confIPMask, 24, DEFAULT_CONF_IP_MASK);
    _readFile("/etc/conf_ip_dns", confIPDNS, 24, DEFAULT_CONF_IP_DNS);
    _readFile("/etc/hostname", host, 64, DEFAULT_HOST);
    _readFile("/etc/keyblayout", keyboardLayout, 8, DEFAULT_KEYBOARD_LAYOUT);
    readCurrentProtectedMode();

    if (strlen(httpAuthPass) == 0) {
        generate_password(httpAuthPass);
        isHhttpAuthPassGenerated = true;
    }
}

void generateWiFiPassword() {
    generate_password(WiFiAPPSK);
    DEBUG2("AP password: %s\n", WiFiAPPSK);
}

void setupAPMode(void) {
    WiFi.mode(WIFI_AP);
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = String("DCHDMI") + String("-") + macID;

    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (uint i=0; i<AP_NameString.length(); i++) {
        if (i < 63) {
            AP_NameChar[i] = AP_NameString.charAt(i);
        }
    }

    DEBUG2("AP_NameChar: %s\n", AP_NameChar);
    generateWiFiPassword();
    WiFi.softAP(AP_NameChar, WiFiAPPSK);
    DEBUG2(">> SSID:   %s\n", AP_NameChar);
    DEBUG2(">> AP-PSK: %s\n", WiFiAPPSK);
    inInitialSetupMode = true;
}

void setupWiFi() {
    WiFi.persistent(false);
    if (strlen(ssid) == 0) {
        DEBUG2(">> No ssid, starting AP mode...\n");
        setupAPMode();
    } else {
        DEBUG2(">> Trying to connect in client mode first...\n");
        setupWiFiStation();
    }
}

void setupWiFiStation() {
    bool doStaticIpConfig = false;
    IPAddress ipAddr;
    doStaticIpConfig = ipAddr.fromString(confIPAddr);
    IPAddress ipGateway;
    doStaticIpConfig = doStaticIpConfig && ipGateway.fromString(confIPGateway);
    IPAddress ipMask;
    doStaticIpConfig = doStaticIpConfig && ipMask.fromString(confIPMask);
    IPAddress ipDNS;
    doStaticIpConfig = doStaticIpConfig && ipDNS.fromString(confIPDNS);

    //WIFI INIT
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    // WiFi.setAutoConnect(true);
    // WiFi.setAutoReconnect(true);
    //WiFi.setPhyMode(WIFI_PHY_MODE_11B);
    //WiFi.setPhyMode(WIFI_PHY_MODE_11G);
    //WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    WiFi.setSleepMode(WIFI_NONE_SLEEP, 0);

    DEBUG(">> Do static ip configuration: %i\n", doStaticIpConfig);
    if (doStaticIpConfig) {
        WiFi.config(ipAddr, ipGateway, ipMask, ipDNS);
    }

    DEBUG(">> WiFi.getAutoConnect: %i\n", WiFi.getAutoConnect());
    DEBUG(">> Connecting to %s\n", ssid);

    if (String(WiFi.SSID()) != String(ssid)) {
        WiFi.begin(ssid, password);
        DEBUG(">> WiFi.begin: %s@%s\n", password, ssid);
    }
    
    bool success = true;
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        DBG_OUTPUT_PORT.print(".");
        if (tries == 60) {
            WiFi.disconnect();
            success = false;
            break;
        }
        tries++;
    }

    DEBUG2(">> success: %i\n", success);

    if (!success) {
        // setup AP mode to configure ssid and password
        setupAPMode();
    } else {
        WiFi.hostname(host);
        WiFi.setAutoReconnect(true);

        ipAddress = WiFi.localIP();
        IPAddress gateway = WiFi.gatewayIP();
        IPAddress subnet = WiFi.subnetMask();

        DEBUG2(
            ">> Connected!\n   IP address:      %d.%d.%d.%d\n",
            ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]
        );
        DEBUG2(
            "   Gateway address: %d.%d.%d.%d\n",
            gateway[0], gateway[1], gateway[2], gateway[3]
        );
        DEBUG2(
            "   Subnet mask:     %d.%d.%d.%d\n",
            subnet[0], subnet[1], subnet[2], subnet[3]
        );
        DEBUG2(
            "   Hostname:        %s\n",
            WiFi.hostname().c_str()
        );
        DEBUG2(
            "   MAC addr:        %s\n",
            WiFi.macAddress().c_str()
        );
    }
}

void setupMDNS() {
    DEBUG2(">> Setting up mDNS...\n");
    if (MDNS.begin(host, ipAddress)) {
        DEBUG2(">> mDNS started.\n");
        MDNS.addService("http", "tcp", 80);
        DEBUG(
            ">> http://%s.local/\n", 
            host
        );
    } else {
        DEBUG2(">> mDNS setup failed.\n");
    }
}

void setupHTTPServer() {
    DEBUG2(">> Setting up HTTP server...\n");

    server.on("/upload/fpga", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        request->send(200);
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        handleUpload(request, FIRMWARE_FILE, index, data, len, final);
    });

    server.on("/upload/esp", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        request->send(200);
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        handleUpload(request, ESP_FIRMWARE_FILE, index, data, len, final);
    });

    server.on("/upload/index", HTTP_POST, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        request->send(200);
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        handleUpload(request, ESP_INDEX_STAGING_FILE, index, data, len, final);
    });

    server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        char msg[64];
        _readFile("/debug", msg, 64, "---");
        request->send(200, "text/plain", msg);
    });

    server.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }

        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","DCHDMI");
        JsonObject& root = response->getRoot();
        FSInfo fs_info;
        SPIFFS.info(fs_info);

        root["totalBytes"] = fs_info.totalBytes;
        root["usedBytes"] = fs_info.usedBytes;
        // root["blockSize"] = fs_info.blockSize;
        // root["pageSize"] = fs_info.pageSize;
        // root["maxOpenFiles"] = fs_info.maxOpenFiles;
        // root["maxPathLength"] = fs_info.maxPathLength;
        // root["freeSketchSpace"] = ESP.getFreeSketchSpace();
        // root["maxSketchSpace"] = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        // root["flashChipSize"] = ESP.getFlashChipSize();
        // root["freeHeapSize"] = ESP.getFreeHeap();

        JsonArray &datas = root.createNestedArray("files");

        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            JsonObject &data = datas.createNestedObject();
            data["name"] = dir.fileName();
            data["size"] = dir.fileSize();
        }

        response->setLength();
        request->send(response);
    });

    server.on("/download/fpga", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleFPGADownload(request);
    });

    server.on("/download/esp", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleESPDownload(request);
    });

    server.on("/download/index", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleESPIndexDownload(request);
    });

    server.on("/flash/fpga", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleFlash(request, FIRMWARE_FILE);
    });

    server.on("/flash/esp", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleESPFlash(request, ESP_FIRMWARE_FILE);
    });

    server.on("/flash/index", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        handleESPIndexFlash(request);
    });

    server.on("/cleanupindex", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        SPIFFS.remove(ESP_INDEX_FILE);
        SPIFFS.remove(String(ESP_INDEX_FILE) + ".md5");
        request->send(200);
    });

    server.on("/cleanup", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        SPIFFS.remove(FIRMWARE_FILE);
        SPIFFS.remove(ESP_FIRMWARE_FILE);
        SPIFFS.remove(ESP_INDEX_STAGING_FILE);
        SPIFFS.remove(String(FIRMWARE_FILE) + ".md5");
        SPIFFS.remove(String(ESP_FIRMWARE_FILE) + ".md5");
        SPIFFS.remove(String(ESP_INDEX_STAGING_FILE) + ".md5");

        SPIFFS.remove(LOCAL_FPGA_MD5);
        SPIFFS.remove(LOCAL_ESP_MD5);
        SPIFFS.remove(LOCAL_ESP_INDEX_MD5);

        SPIFFS.remove(SERVER_FPGA_MD5);
        SPIFFS.remove(SERVER_ESP_MD5);
        SPIFFS.remove(SERVER_ESP_INDEX_MD5);

        // remove legacy config data
        SPIFFS.remove("/etc/firmware_fpga");
        SPIFFS.remove("/etc/firmware_format");
        SPIFFS.remove("/etc/force_vga");
        SPIFFS.remove("/etc/resolution");
        request->send(200);
    });

    server.on("/resetconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        SPIFFS.remove("/etc/ssid");
        SPIFFS.remove("/etc/password");
        SPIFFS.remove("/etc/ota_pass");
        SPIFFS.remove("/etc/http_auth_user");
        SPIFFS.remove("/etc/http_auth_pass");
        SPIFFS.remove("/etc/conf_ip_addr");
        SPIFFS.remove("/etc/conf_ip_gateway");
        SPIFFS.remove("/etc/conf_ip_mask");
        SPIFFS.remove("/etc/conf_ip_dns");
        SPIFFS.remove("/etc/hostname");
        SPIFFS.remove("/etc/keyblayout");

        ssid[0] = '\0';
        password[0] = '\0';
        otaPassword[0] = '\0';
        keyboardLayout[0] = '\0';
        // keep passwords alive until power off
        //httpAuthUser[0] = '\0';
        //httpAuthPass[0] = '\0';
        confIPAddr[0] = '\0';
        confIPGateway[0] = '\0';
        confIPMask[0] = '\0';
        confIPDNS[0] = '\0';
        host[0] = '\0';

        request->send(200);
    });

    server.on("/factoryresetall", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        SPIFFS.format();
        request->send(200);
    });

   server.on("/flash/secure/fpga", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        disableFPGA();
        handleFlash(request, FIRMWARE_FILE);
    });

    server.on("/progress", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        DEBUG("progress requested...\n");
        char msg[64];
        if (last_error) {
            sprintf(msg, "ERROR %i\n", last_error);
            request->send(200, "text/plain", msg);
            DEBUG("...delivered: %s (%i).\n", msg, last_error);
            // clear last_error
            last_error = NO_ERROR;
        } else {
            int progress = totalLength <= 0 ? 0 : (int)(readLength * 100 / totalLength);
            if (progress == 100 && !currentJobDone) {
                progress = 99;
            }
            sprintf(msg, "%i\n", progress);
            request->send(200, "text/plain", msg);
            DEBUG2("...delivered: %s.\n", msg);
        }
    });

    server.on("/flash_size", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        char msg[64];
        sprintf(msg, "%lu\n", (long unsigned int) ESP.getFlashChipSize());
        request->send(200, "text/plain", msg);
    });

    server.on("/reset/fpga", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        DEBUG("FPGA reset requested...\n");
        fpgaTask.DoResetFPGA();
        request->send(200, "text/plain", "OK\n");
        DEBUG("...delivered.\n");
    });

    server.on("/reset/dc", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        DEBUG("DC reset requested...\n");
        fpgaTask.Write(I2C_DC_RESET, 0, NULL);
        request->send(200, "text/plain", "OK\n");
        DEBUG("...delivered.\n");
    });

    server.on("/reset/all", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        resetall();
    });

    server.on("/reset/pll", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_PLL_RESET, 0, NULL);
        request->send(200, "text/plain", "OK\n");
    });

    server.on("/issetupmode", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        request->send(200, "text/plain", inInitialSetupMode ? "true\n" : "false\n");
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200);
    });

    server.on("/setup", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        writeSetupParameter(request, "ssid", ssid, 64, DEFAULT_SSID);
        writeSetupParameter(request, "password", password, 64, DEFAULT_PASSWORD);
        writeSetupParameter(request, "ota_pass", otaPassword, 64, DEFAULT_OTA_PASSWORD);
        writeSetupParameter(request, "firmware_server", firmwareServer, 256, DEFAULT_FW_SERVER);
        writeSetupParameter(request, "firmware_server_path", firmwareServerPath, 256, DEFAULT_FW_SERVER_PATH);
        writeSetupParameter(request, "firmware_version", firmwareVersion, 64, DEFAULT_FW_VERSION);
        writeSetupParameter(request, "http_auth_user", httpAuthUser, 64, DEFAULT_HTTP_USER, true);
        writeSetupParameter(request, "http_auth_pass", httpAuthPass, 64, DEFAULT_HTTP_PASS, true);
        writeSetupParameter(request, "conf_ip_addr", confIPAddr, 24, DEFAULT_CONF_IP_ADDR);
        writeSetupParameter(request, "conf_ip_gateway", confIPGateway, 24, DEFAULT_CONF_IP_GATEWAY);
        writeSetupParameter(request, "conf_ip_mask", confIPMask, 24, DEFAULT_CONF_IP_MASK);
        writeSetupParameter(request, "conf_ip_dns", confIPDNS, 24, DEFAULT_CONF_IP_DNS);
        writeSetupParameter(request, "hostname", host, 64, DEFAULT_HOST);
        writeSetupParameter(request, "video_resolution", configuredResolution, "/etc/video/resolution", 16, DEFAULT_VIDEO_RESOLUTION);
        writeSetupParameter(request, "video_mode", videoMode, "/etc/video/mode", 16, DEFAULT_VIDEO_MODE);
        writeSetupParameter(request, "reset_mode", resetMode, "/etc/reset/mode", 16, DEFAULT_RESET_MODE);
        writeSetupParameter(request, "deinterlace_mode", deinterlaceMode480i, "/etc/deinterlace/mode/480i", 16, DEFAULT_DEINTERLACE_MODE);
        writeSetupParameter(request, "deinterlace_mode", deinterlaceMode576i, "/etc/deinterlace/mode/576i", 16, DEFAULT_DEINTERLACE_MODE);
        writeSetupParameter(request, "protected_mode", protectedMode, "/etc/protected/mode", 8, DEFAULT_PROTECTED_MODE);
        writeSetupParameter(request, "keyboard_layout", keyboardLayout, "/etc/keyblayout", 8, DEFAULT_KEYBOARD_LAYOUT);
        readCurrentProtectedMode(true);

        request->send(200, "text/plain", "OK\n");
    });
    
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject& root = response->getRoot();

        root["ssid"] = ssid;
        root["password"] = password;
        root["ota_pass"] = otaPassword;
        root["firmware_server"] = firmwareServer;
        root["firmware_server_path"] = firmwareServerPath;
        root["firmware_version"] = firmwareVersion;
        root["http_auth_user"] = httpAuthUser;
        root["http_auth_pass"] = httpAuthPass;
        root["flash_chip_size"] = ESP.getFlashChipSize();
        root["conf_ip_addr"] = confIPAddr;
        root["conf_ip_gateway"] = confIPGateway;
        root["conf_ip_mask"] = confIPMask;
        root["conf_ip_dns"] = confIPDNS;
        root["hostname"] = host;
        root["fw_version"] = isRelaxedFirmware ? FW_VERSION "-rlx" : FW_VERSION "-std";
        root["video_resolution"] = configuredResolution;
        root["video_mode"] = videoMode;
        root["reset_mode"] = resetMode;
        root["deinterlace_mode"] = deinterlaceMode480i;
        root["protected_mode"] = protectedMode;
        root["keyboard_layout"] = keyboardLayout;

        response->setLength();
        request->send(response);
    });

    server.on("/reset/esp", HTTP_ANY, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        //ESP.eraseConfig();
        ESP.restart();
    });

    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
    });

    server.on("/osdwrite", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }

        AsyncWebParameter *column = request->getParam("column", true);
        AsyncWebParameter *row = request->getParam("row", true);
        AsyncWebParameter *text = request->getParam("text", true);

        fpgaTask.DoWriteToOSD(atoi(column->value().c_str()), atoi(row->value().c_str()), (uint8_t*) text->value().c_str());
        request->send(200);
    });

    server.on("/240p_offset", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }

        AsyncWebParameter *offset = request->getParam("offset", true);
        fpgaTask.Write(I2C_240P_OFFSET, atoi(offset->value().c_str()), NULL);
        request->send(200);
    });

    server.on("/scanlines", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }

        AsyncWebParameter *s_intensity = request->getParam("intensity", true);
        AsyncWebParameter *s_thickness = request->getParam("thickness", true);
        AsyncWebParameter *s_oddeven = request->getParam("oddeven", true);
        AsyncWebParameter *s_active = request->getParam("active", true);

        scanlinesIntensity = atoi(s_intensity->value().c_str());
        scanlinesThickness = atoi(s_thickness->value().c_str());
        scanlinesOddeven = atoi(s_oddeven->value().c_str());
        scanlinesActive = atoi(s_active->value().c_str());

        uint8_t upper = getScanlinesUpperPart();
        uint8_t lower = getScanlinesLowerPart();

        setScanlines(upper, lower, NULL);
        request->send(200);
    });

    server.on("/osd/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        openOSD();
        request->send(200);
    });

    server.on("/osd/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        closeOSD();
        request->send(200);
    });

    server.on("/hq2x/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_UPSCALING_MODE, 0x01, NULL);
        request->send(200);
    });

    server.on("/hq2x/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_UPSCALING_MODE, 0x00, NULL);
        request->send(200);
    });

    server.on("/res/VGA", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        switchResolution(RESOLUTION_VGA);
        request->send(200);
    });

    server.on("/res/480p", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        switchResolution(RESOLUTION_480p);
        request->send(200);
    });

    server.on("/res/960p", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        switchResolution(RESOLUTION_960p);
        request->send(200);
    });

    server.on("/res/1080p", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        switchResolution(RESOLUTION_1080p);
        request->send(200);
    });

    server.on("/deinterlace/bob", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        CurrentDeinterlaceMode480i = DEINTERLACE_MODE_BOB;
        CurrentDeinterlaceMode576i = DEINTERLACE_MODE_BOB;
        switchResolution(CurrentResolution);
        request->send(200);
    });

    server.on("/deinterlace/passthru", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        CurrentDeinterlaceMode480i = DEINTERLACE_MODE_PASSTHRU;
        CurrentDeinterlaceMode576i = DEINTERLACE_MODE_PASSTHRU;
        switchResolution(CurrentResolution);
        request->send(200);
    });

    server.on("/generate/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_VIDEO_GEN, GENERATE_TIMING_AND_VIDEO, NULL);
        request->send(200);
    });

    server.on("/generate/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_VIDEO_GEN, 0, NULL);
        request->send(200);
    });

    server.on("/spi/flash/erase", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        taskManager.StartTask(&flashEraseTask);
        request->send(200);
    });

    server.on("/clock/config/get", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Read(0xD0, 1, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
            char msg[16];
            sprintf(msg, "%u\n", buffer[0]);
            request->send(200, "text/plain", msg);
        });
    });

    server.on("/clock/config/set", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }

        AsyncWebParameter *s_value = request->getParam("value", true);
        fpgaTask.Write(0xD0, atoi(s_value->value().c_str()), NULL);
        request->send(200);
    });

    server.on("/mac/get", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        char msg[32];
        sprintf(msg, "%s\n", WiFi.macAddress().c_str());
        request->send(200, "text/plain", msg);
    });

    server.on("/nbp/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_NBP_RESET, 0x01, [](uint8_t Address, uint8_t Value) {
            DEBUG2("nbp reset.\n");
        });
        request->send(200, "text/plain", "OK\n");
    });

    server.on("/testdata3", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Read(0xF5, 3, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
            if (len == 3) {
                char msg[3 * 3 + 1] = "";
                int p = 0;
                for (int i = 0 ; i < 3 ; i++) {
                    sprintf((char*) &msg[p], "%02x ", buffer[i]);
                    p = p + 3;
                }
                sprintf((char*) &msg[p], "\n");
                request->send(200, "text/plain", msg);
            } else {
                request->send(200, "text/plain", "SOMETHING_IS_WRONG\n");
            }
        }); 
        fpgaTask.ForceLoop();
    });

    server.on("/testdata2", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        char msg[16];
        sprintf(
            msg, 
            "%02x %02x %02x %02x %02x\n",
            remapResolution(CurrentResolution),
            mapResolution(CurrentResolution, true),
            CurrentResolutionData,
            CurrentDeinterlaceMode480i,
            CurrentDeinterlaceMode576i
        );
        request->send(200, "text/plain", msg);
    });

    server.on("/testdata", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Read(I2C_TESTDATA_BASE, I2C_TESTDATA_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
            if (len == I2C_TESTDATA_LENGTH) {
                char msg[I2C_TESTDATA_LENGTH * 3 + 1] = "";
                int p = 0;
                for (int i = 0 ; i < I2C_TESTDATA_LENGTH ; i++) {
                    sprintf((char*) &msg[p], "%02x ", buffer[i]);
                    p = p + 3;
                }
                sprintf((char*) &msg[p], "\n");
                request->send(200, "text/plain", msg);

                // request->send("text/plain", I2C_TESTDATA_LENGTH * 3 + 1, [ buffer ](uint8_t *buffer_out, size_t maxLen, size_t index) -> size_t {
                //     return I2C_TESTDATA_LENGTH * 3 + 1;
                // });
            } else {
                request->send(200, "text/plain", "SOMETHING_IS_WRONG\n");
            }
        }); 
        fpgaTask.ForceLoop();
    });

    AsyncStaticWebHandler* handler = &server
        .serveStatic("/", SPIFFS, "/")
        .setDefaultFile("index.html");
    // set authentication by configured user/pass later
    handler->setAuthentication(httpAuthUser, httpAuthPass);

    server.onNotFound([](AsyncWebServerRequest *request){
        if (request->url().endsWith("md5")) {
           request->send(200, "text/plain", DEFAULT_MD5_SUM"\n");
           return;
        }
        request->send(404);
    });

    server.begin();
}

void setupArduinoOTA() {
    DEBUG2(">> Setting up ArduinoOTA...\n");
    
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(host);
    ArduinoOTA.setPassword(otaPassword);
    
    ArduinoOTA.onStart([]() {
        DEBUG("ArduinoOTA >> Start\n");
    });
    ArduinoOTA.onEnd([]() {
        DEBUG("\nArduinoOTA >> End\n");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUG("ArduinoOTA >> Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG("ArduinoOTA >> Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            DEBUG("ArduinoOTA >> Auth Failed\n");
        } else if (error == OTA_BEGIN_ERROR) {
            DEBUG("ArduinoOTA >> Begin Failed\n");
        } else if (error == OTA_CONNECT_ERROR) {
            DEBUG("ArduinoOTA >> Connect Failed\n");
        } else if (error == OTA_RECEIVE_ERROR) {
            DEBUG("ArduinoOTA >> Receive Failed\n");
        } else if (error == OTA_END_ERROR) {
            DEBUG("ArduinoOTA >> End Failed\n");
        }
    });
    ArduinoOTA.begin(true);
    DEBUG2(">> ArduinoOTA started.\n");
}

void waitForController() {
    bool gotValidPacket = false;
    uint8_t _ForceVGA = ForceVGA;
    DEBUG2(">> Checking video mode controller override...\n");

    for (int i = 0 ; i < 3333 ; i++) {
        // stop, if we got a valid packet
        if (gotValidPacket) {
            DEBUG2("   found valid controller packet at %i\n", i);
            if (_ForceVGA != ForceVGA) {
                ForceVGA = _ForceVGA;
                writeVideoMode();
                DEBUG2("   performing a resetall\n");
                resetall();
            } else {
                switchResolution();
                fpgaTask.ForceLoop();
                DEBUG2("   video mode NOT changed: %u\n", ForceVGA);
            }
            return;
        }
        fpgaTask.Read(I2C_CONTROLLER_AND_DATA_BASE, I2C_CONTROLLER_AND_DATA_BASE_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
            if (len == I2C_CONTROLLER_AND_DATA_BASE_LENGTH) {
                uint16_t cdata = (buffer[0] << 8 | buffer[1]);
                uint8_t metadata = buffer[2];
                if (CHECK_BIT(cdata, CTRLR_DATA_VALID)) {
                    isRelaxedFirmware = metadata & HQ2X_MODE_FLAG;
                    storeResolutionData(metadata);
                    DEBUG2("   %i: %04x %02x\n", i, cdata, metadata);
                    if (CHECK_BIT(cdata, CTRLR_PAD_UP)) {
                        _ForceVGA = VGA_ON;
                    } else if (CHECK_BIT(cdata, CTRLR_PAD_DOWN)) {
                        _ForceVGA = VGA_OFF;
                    }
                    gotValidPacket = true;
                }
            }
        });
        fpgaTask.ForceLoop();
        delay(1);
    }
    DEBUG2("no valid controller packet found within timeout\n");
}

void doReflash() {
    disableFPGA();
    if (flashTask.doStart()) {
        // [](int read, int total, bool done, int error) {
        // }
        DEBUG2("   firmware flashing started.\n");
        while (!flashTask.doUpdate()) {
            yield();
        }
        flashTask.doStop();
        DEBUG2("   firmware flashing done.\n");
        resetall();
    }
}

void printSerialMenu() {
    DEBUG2("- Menu -----------------------\n");
    DEBUG2("f: re-flash fpga firmware\n");
    DEBUG2("i: show info\n");
    DEBUG2("r: reset visible area counters\n");
    DEBUG2("p: reset PLL\n");
    DEBUG2("t: show visible area counters\n");
    DEBUG2("c: show color space markers\n");
    DEBUG2("o: activate OTA update\n");
    DEBUG2("w: print wifi rssi information\n");
    DEBUG2("h: print this menu\n");
    DEBUG2("------------------------------\n");
}

void setup(void) {
    DBG_OUTPUT_PORT.begin(115200);
    DEBUG2("\n>> ESP starting... " DCHDMI_VERSION "\n");
    DEBUG2(">> %s\n", ESP.getFullVersion().c_str());
    DBG_OUTPUT_PORT.setDebugOutput(false);

    pinMode(NCE, INPUT);
    pinMode(NCONFIG, INPUT);

    setupI2C();
    setupSPIFFS();
    setupResetMode();
    setupOutputResolution();
    setupScanlines();
    setupOffsets();
    setupUpscalingMode();
    setupColorSpace();
    setupColorExpansionAndGamma();
    setupTaskManager();
    setupCredentials();
    waitForController();
    fpgaTask.Write(I2C_ACTIVATE_HDMI, 1, NULL); fpgaTask.ForceLoop();
    setupWiFi();

    setOSD(false, NULL); fpgaTask.ForceLoop();
    char buff[MENU_WIDTH+1]; osd_get_resolution(buff);
    fpgaTask.DoWriteToOSD(0, MENU_WIDTH, (uint8_t*) buff); fpgaTask.ForceLoop();

    if (!inInitialSetupMode) {
        // skip MDNS setup, because it can cause random crashes on startup
        //setupMDNS();
    }

    if (reflashNeccessary && reflashNeccessary2 && reflashNeccessary3) {
        DEBUG2("FPGA firmware missing or broken, reflash needed.\n");
        doReflash();
    }
    setupHTTPServer();
    DEBUG2(">> httpAuthUser: %s\n", httpAuthUser);
    DEBUG2(">> httpAuthPass: %s\n", httpAuthPass);
    DEBUG2(">> Ready.\n");
    printSerialMenu();
}

void showInfo() {
    DEBUG2("ESP: " DCHDMI_VERSION "\n");
    DEBUG2("Arduino ESP version: %s\n", ESP.getFullVersion().c_str());
    DEBUG2("Free sketch space: %u\n", ESP.getFreeSketchSpace());
    DEBUG2("Max sketch space: %u\n", (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
    DEBUG2("Flash chip size: %u\n", ESP.getFlashChipSize());
    DEBUG2("Heap fragmentation: %u\n", ESP.getHeapFragmentation());
    DEBUG2("Free heap size: %u\n\n", ESP.getFreeHeap());
}

int getWiFiQuality(int dBm) {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void toBinaryString(char* msg, uint8_t* a, int len) {
    uint8_t i, ii = 0;
    for(int j = 0 ; j < len ; j++) {
        if (j > 0) {
            sprintf((char*) &msg[ii++], " ");
        }
        for(i = 0x80 ; i != 0 ; i >>= 1) {
            sprintf((char*) &msg[ii++], "%c", (a[j]&i)?'1':'0');
        }
    }
}

void setColorMode() {
    uint8_t val = fpgaTask.GetColorExpansion() | GammaMode << 3;
    fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, val, [&](uint8_t Address, uint8_t Value) {
        char msg[9] = "";
        toBinaryString(msg, &Value, 1);
        DEBUG2("set color mode to %s (0x%02x/0x%02x).\n", msg, (Value & 0xF8) >> 3, Value & 0x7);
    });
}

void loop(void){
    ArduinoOTA.handle();
    taskManager.Loop();
    if (Serial.available() > 0) {
        int incomingByte = Serial.read();
        if (incomingByte == 'f') {
            doReflash();
        } else if (incomingByte == 'i') {
            showInfo();
        } else if (incomingByte == 'h') {
            printSerialMenu();
        } else if (incomingByte == 'p') {
            fpgaTask.Write(I2C_PLL_RESET, 0, [](uint8_t Address, uint8_t Value) {
                DEBUG2("pll reset.\n");
            });
        } else if (incomingByte == 'r') {
            fpgaTask.Write(I2C_NBP_RESET, 0, [](uint8_t Address, uint8_t Value) {
                DEBUG2("nbp reset.\n");
            });
        } else if (incomingByte == 't') {
            fpgaTask.Read(I2C_TESTDATA_BASE, I2C_TESTDATA_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                int nbp1 = (buffer[35] << 4) | (buffer[36] >> 4);
                int nbp2 = ((buffer[36] & 0xF) << 8) | buffer[37];
                DEBUG2("nbp: %02x %02x %02x %dx%d\n", buffer[35], buffer[36], buffer[37], nbp1, nbp2);
            });
        } else if (incomingByte == 'c') {
            fpgaTask.Read(I2C_CSEDATA_BASE, I2C_CSEDATA_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                char msg[9*I2C_CSEDATA_LENGTH+1] = "";
                toBinaryString(msg, buffer, I2C_CSEDATA_LENGTH);
                DEBUG2("cse: %s %02x %02x %02x\n", msg, buffer[0], buffer[1], buffer[2]);
            });
            fpgaTask.Read(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, 1, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                char msg[9] = "";
                toBinaryString(msg, buffer, 1);
                DEBUG2("Color mode: %s (0x%02x/0x%02x) [0x%02x].\n", msg, (buffer[0] & 0xF8) >> 3, buffer[0] & 0x7, ColorExpansionMode);
            });
        } else if (incomingByte == 'o') {
            if (strlen(otaPassword)) {
                setupArduinoOTA();
            } else {
                DEBUG2("Please set an OTA password via web interface first!\n");
            }
        } else if (incomingByte == 'w') {
            int dBm = WiFi.RSSI();
            DEBUG2("RSSI: %d dBm, quality: %d%%, channel: %d\n", dBm, getWiFiQuality(dBm), WiFi.channel());
        } else if (incomingByte == '0') {
            ColorExpansionMode = 0; setColorMode();
        } else if (incomingByte == '1') {
            ColorExpansionMode = 1; setColorMode();
        } else if (incomingByte == '3') {
            ColorExpansionMode = 3; setColorMode();
        } else if (incomingByte == '+') {
            GammaMode++; setColorMode();
        } else if (incomingByte == '-') {
            GammaMode--; setColorMode();
        } else if (incomingByte == '=') {
            GammaMode = 0x0F; setColorMode();
        } else {
            DEBUG2("DEBUG serial key: %u\n", incomingByte);
        }
    }
}
