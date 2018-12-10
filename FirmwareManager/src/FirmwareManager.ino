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
#include "task/FlashESPTask.h"
#include "task/FlashESPIndexTask.h"
#include "task/FPGATask.h"
#include "task/DebugTask.h"
#include "task/TimeoutTask.h"
#include "task/FlashCheckTask.h"
#include "task/FlashEraseTask.h"
#include "util.h"
#include "data.h"
#include "web.h"
#include "Menu.h"

//////////////////////////////////////////////////////////////////////////////////

char ssid[64] = DEFAULT_SSID;
char password[64] = DEFAULT_PASSWORD;
char otaPassword[64] = DEFAULT_OTA_PASSWORD; 
char firmwareServer[1024] = DEFAULT_FW_SERVER;
char firmwareVersion[64] = DEFAULT_FW_VERSION;
char httpAuthUser[64] = DEFAULT_HTTP_USER;
char httpAuthPass[64] = DEFAULT_HTTP_PASS;
char confIPAddr[24] = DEFAULT_CONF_IP_ADDR;
char confIPGateway[24] = DEFAULT_CONF_IP_GATEWAY;
char confIPMask[24] = DEFAULT_CONF_IP_MASK;
char confIPDNS[24] = DEFAULT_CONF_IP_DNS;
char host[64] = DEFAULT_HOST;
char videoMode[16] = "";
char configuredResolution[16] = "";
char resetMode[16] = "";
const char* WiFiAPPSK = "geheim1234";
IPAddress ipAddress( 192, 168, 4, 1 );
bool inInitialSetupMode = false;
AsyncWebServer server(80);
SPIFlash flash(CS);
int last_error = NO_ERROR; 
int totalLength;
int readLength;

File flashFile;

uint8_t CurrentResolution = RESOLUTION_1080p;
uint8_t PrevCurrentResolution;
uint8_t ForceVGA = VGA_ON;
uint8_t CurrentResetMode = RESET_MODE_LED;

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
FlashEraseTask flashEraseTask(1);

extern Menu mainMenu;
Menu *currentMenu;
Menu *previousMenu;

//////////////////////////////////////////////////////////////////////////////////

void setupSPIFFS() {
    DBG_OUTPUT_PORT.printf(">> Setting up SPIFFS...\n");
    if (!SPIFFS.begin()) {
        DBG_OUTPUT_PORT.printf(">> SPIFFS begin failed, trying to format...");
        if (SPIFFS.format()) {
            DBG_OUTPUT_PORT.printf("done.\n");
        } else {
            DBG_OUTPUT_PORT.printf("error.\n");
        }
    }
}

void setupResetMode() {
    readCurrentResetMode();
    DBG_OUTPUT_PORT.printf(">> Setting up reset mode: %x\n", CurrentResetMode);
    reflashNeccessary3 = !forceI2CWrite(
        I2C_RESET_CONF, CurrentResetMode, 
        I2C_PING, 0
    );
}

void setupOutputResolution() {
    readVideoMode();
    readCurrentResolution();

    DBG_OUTPUT_PORT.printf(">> Setting up output resolution: %x\n", ForceVGA | CurrentResolution);
    reflashNeccessary = !forceI2CWrite(
        I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, 
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

    DBG_OUTPUT_PORT.printf(">> Setting up scanlines:\n");
    reflashNeccessary2 = !forceI2CWrite(
        I2C_SCANLINE_UPPER, upper, 
        I2C_SCANLINE_LOWER, lower
    );
}

void setupTaskManager() {
    DBG_OUTPUT_PORT.printf(">> Setting up task manager...\n");
    taskManager.Setup();
    taskManager.StartTask(&fpgaTask);
}

void setupCredentials(void) {
    DBG_OUTPUT_PORT.printf(">> Reading stored values...\n");

    _readFile("/etc/ssid", ssid, 64, DEFAULT_SSID);
    _readFile("/etc/password", password, 64, DEFAULT_PASSWORD);
    _readFile("/etc/ota_pass", otaPassword, 64, DEFAULT_OTA_PASSWORD);
    _readFile("/etc/firmware_server", firmwareServer, 1024, DEFAULT_FW_SERVER);
    _readFile("/etc/firmware_version", firmwareVersion, 64, DEFAULT_FW_VERSION);
    _readFile("/etc/http_auth_user", httpAuthUser, 64, DEFAULT_HTTP_USER);
    _readFile("/etc/http_auth_pass", httpAuthPass, 64, DEFAULT_HTTP_PASS);
    _readFile("/etc/conf_ip_addr", confIPAddr, 24, DEFAULT_CONF_IP_ADDR);
    _readFile("/etc/conf_ip_gateway", confIPGateway, 24, DEFAULT_CONF_IP_GATEWAY);
    _readFile("/etc/conf_ip_mask", confIPMask, 24, DEFAULT_CONF_IP_MASK);
    _readFile("/etc/conf_ip_dns", confIPDNS, 24, DEFAULT_CONF_IP_DNS);
    _readFile("/etc/hostname", host, 64, DEFAULT_HOST);
}

void setupAPMode(void) {
    WiFi.mode(WIFI_AP);
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = String(host) + String("-") + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);
    
    for (uint i=0; i<AP_NameString.length(); i++) {
        AP_NameChar[i] = AP_NameString.charAt(i);
    }

    WiFi.softAP(AP_NameChar, WiFiAPPSK);
    DBG_OUTPUT_PORT.printf(">> SSID:   %s\n", AP_NameChar);
    DBG_OUTPUT_PORT.printf(">> AP-PSK: %s\n", WiFiAPPSK);
    inInitialSetupMode = true;
}

void setupWiFi() {
    if (strlen(ssid) == 0) {
        DBG_OUTPUT_PORT.printf(">> No ssid, starting AP mode...\n");
        setupAPMode();
    } else {
        DBG_OUTPUT_PORT.printf(">> Trying to connect in client mode first...\n");
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
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.hostname(host);

    DBG_OUTPUT_PORT.printf(">> Do static ip configuration: %i\n", doStaticIpConfig);
    if (doStaticIpConfig) {
        WiFi.config(ipAddr, ipGateway, ipMask, ipDNS);
    }

    WiFi.mode(WIFI_STA);
    
    DBG_OUTPUT_PORT.printf(">> WiFi.getAutoConnect: %i\n", WiFi.getAutoConnect());
    DBG_OUTPUT_PORT.printf(">> Connecting to %s\n", ssid);

    if (String(WiFi.SSID()) != String(ssid)) {
        WiFi.begin(ssid, password);
        DBG_OUTPUT_PORT.printf(">> WiFi.begin: %s@%s\n", password, ssid);
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

    DBG_OUTPUT_PORT.printf(">> success: %i\n", success);

    if (!success) {
        // setup AP mode to configure ssid and password
        setupAPMode();
    } else {
        ipAddress = WiFi.localIP();
        IPAddress gateway = WiFi.gatewayIP();
        IPAddress subnet = WiFi.subnetMask();

        DBG_OUTPUT_PORT.printf(
            ">> Connected!\n   IP address:      %d.%d.%d.%d\n",
            ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]
        );
        DBG_OUTPUT_PORT.printf(
            "   Gateway address: %d.%d.%d.%d\n",
            gateway[0], gateway[1], gateway[2], gateway[3]
        );
        DBG_OUTPUT_PORT.printf(
            "   Subnet mask:     %d.%d.%d.%d\n",
            subnet[0], subnet[1], subnet[2], subnet[3]
        );
        DBG_OUTPUT_PORT.printf(
            "   Hostname:        %s\n",
            WiFi.hostname().c_str()
        );
    }
    
    if (MDNS.begin(host, ipAddress)) {
        DBG_OUTPUT_PORT.println(">> mDNS started");
        MDNS.addService("http", "tcp", 80);
        DBG_OUTPUT_PORT.printf(
            ">> http://%s.local/\n", 
            host
        );
    }
}

void setupHTTPServer() {
    DBG_OUTPUT_PORT.printf(">> Setting up HTTP server...\n");

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

        AsyncResponseStream *response = request->beginResponseStream("text/json");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();

        FSInfo fs_info;
        SPIFFS.info(fs_info);

        root["totalBytes"] = fs_info.totalBytes;
        root["usedBytes"] = fs_info.usedBytes;
        root["blockSize"] = fs_info.blockSize;
        root["pageSize"] = fs_info.pageSize;
        root["maxOpenFiles"] = fs_info.maxOpenFiles;
        root["maxPathLength"] = fs_info.maxPathLength;
        root["freeSketchSpace"] = ESP.getFreeSketchSpace();
        root["maxSketchSpace"] = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        root["flashChipSize"] = ESP.getFlashChipSize();
        root["freeHeapSize"] = ESP.getFreeHeap();

        JsonArray &datas = root.createNestedArray("files");

        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
        JsonObject &data = datas.createNestedObject();
            data["name"] = dir.fileName();
            data["size"] = dir.fileSize();
        }

        root.printTo(*response);
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
        // remove legacy config data
        SPIFFS.remove("/etc/firmware_fpga");
        SPIFFS.remove("/etc/firmware_format");
        SPIFFS.remove("/etc/force_vga");
        SPIFFS.remove("/etc/resolution");
        request->send(200);
    });

    server.on("/cleanupconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        SPIFFS.remove("/etc/video/resolution");
        SPIFFS.remove("/etc/video/mode");
        SPIFFS.remove("/etc/reset/mode");
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
        DBG_OUTPUT_PORT.printf("progress requested...\n");
        char msg[64];
        if (last_error) {
            sprintf(msg, "ERROR %i\n", last_error);
            request->send(200, "text/plain", msg);
            DBG_OUTPUT_PORT.printf("...delivered: %s (%i).\n", msg, last_error);
            // clear last_error
            last_error = NO_ERROR;
        } else {
            sprintf(msg, "%i\n", totalLength <= 0 ? 0 : (int)(readLength * 100 / totalLength));
            request->send(200, "text/plain", msg);
            DBG_OUTPUT_PORT.printf("...delivered: %s.\n", msg);
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
        DBG_OUTPUT_PORT.printf("FPGA reset requested...\n");
        enableFPGA();
        resetFPGAConfiguration();
        request->send(200, "text/plain", "OK\n");
        DBG_OUTPUT_PORT.printf("...delivered.\n");
    });

    server.on("/reset/all", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        resetall();
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
        writeSetupParameter(request, "firmware_server", firmwareServer, 1024, DEFAULT_FW_SERVER);
        writeSetupParameter(request, "firmware_version", firmwareVersion, 64, DEFAULT_FW_VERSION);
        writeSetupParameter(request, "http_auth_user", httpAuthUser, 64, DEFAULT_HTTP_USER);
        writeSetupParameter(request, "http_auth_pass", httpAuthPass, 64, DEFAULT_HTTP_PASS);
        writeSetupParameter(request, "conf_ip_addr", confIPAddr, 24, DEFAULT_CONF_IP_ADDR);
        writeSetupParameter(request, "conf_ip_gateway", confIPGateway, 24, DEFAULT_CONF_IP_GATEWAY);
        writeSetupParameter(request, "conf_ip_mask", confIPMask, 24, DEFAULT_CONF_IP_MASK);
        writeSetupParameter(request, "conf_ip_dns", confIPDNS, 24, DEFAULT_CONF_IP_DNS);
        writeSetupParameter(request, "hostname", host, 64, DEFAULT_HOST);
        writeSetupParameter(request, "video_resolution", configuredResolution, "/etc/video/resolution", 16, DEFAULT_VIDEO_RESOLUTION);
        writeSetupParameter(request, "video_mode", videoMode, "/etc/video/mode", 16, DEFAULT_VIDEO_MODE);
        writeSetupParameter(request, "reset_mode", resetMode, "/etc/reset/mode", 16, DEFAULT_RESET_MODE);

        request->send(200, "text/plain", "OK\n");
    });
    
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        
        AsyncResponseStream *response = request->beginResponseStream("text/json");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();

        root["ssid"] = ssid;
        root["password"] = password;
        root["ota_pass"] = otaPassword;
        root["firmware_server"] = firmwareServer;
        root["firmware_version"] = firmwareVersion;
        root["http_auth_user"] = httpAuthUser;
        root["http_auth_pass"] = httpAuthPass;
        root["flash_chip_size"] = ESP.getFlashChipSize();
        root["conf_ip_addr"] = confIPAddr;
        root["conf_ip_gateway"] = confIPGateway;
        root["conf_ip_mask"] = confIPMask;
        root["conf_ip_dns"] = confIPDNS;
        root["hostname"] = host;
        root["fw_version"] = FW_VERSION;
        root["video_resolution"] = configuredResolution;
        root["video_mode"] = videoMode;
        root["reset_mode"] = resetMode;

        root.printTo(*response);
        request->send(response);
    });

    server.on("/reset/esp", HTTP_ANY, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        ESP.eraseConfig();
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

    server.on("/reset/pll", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution | PLL_RESET_ON, NULL);
        request->send(200);
    });

    server.on("/generate/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution | GENERATE_TIMING_AND_VIDEO, NULL);
        request->send(200);
    });

    server.on("/generate/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, NULL);
        request->send(200);
    });

    server.on("/spi/flash/erase", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        taskManager.StartTask(&flashEraseTask);
        request->send(200);
    });

    server.on("/pinok", HTTP_GET, [](AsyncWebServerRequest *request) {
        if(!_isAuthenticated(request)) {
            return request->requestAuthentication();
        }
        fpgaTask.Read(I2C_PINOK_BASE, I2C_PINOK_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
            char msg[64];
            if (len == I2C_PINOK_LENGTH) {
                sprintf(msg, "GOT: %02x %02x %02x\n", buffer[0], buffer[1], buffer[2]);
                request->send(200, "text/plain", msg);
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
        if (request->url().endsWith(".md5")) {
           request->send(200, "text/plain", DEFAULT_MD5_SUM"\n");
           return;
        }
        request->send(404);
    });

    server.begin();
}

void setupArduinoOTA() {
    DBG_OUTPUT_PORT.printf(">> Setting up ArduinoOTA...\n");
    
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(host);
    ArduinoOTA.setPassword(otaPassword);
    
    ArduinoOTA.onStart([]() {
        DBG_OUTPUT_PORT.println("ArduinoOTA >> Start");
    });
    ArduinoOTA.onEnd([]() {
        DBG_OUTPUT_PORT.println("\nArduinoOTA >> End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DBG_OUTPUT_PORT.printf("ArduinoOTA >> Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DBG_OUTPUT_PORT.printf("ArduinoOTA >> Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            DBG_OUTPUT_PORT.println("ArduinoOTA >> Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            DBG_OUTPUT_PORT.println("ArduinoOTA >> Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            DBG_OUTPUT_PORT.println("ArduinoOTA >> Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            DBG_OUTPUT_PORT.println("ArduinoOTA >> Receive Failed");
        } else if (error == OTA_END_ERROR) {
            DBG_OUTPUT_PORT.println("ArduinoOTA >> End Failed");
        }
    });
    ArduinoOTA.begin();
}

void setup(void) {
    DBG_OUTPUT_PORT.begin(115200);
    DBG_OUTPUT_PORT.printf("\n>> FirmwareManager starting...\n");
    DBG_OUTPUT_PORT.setDebugOutput(DEBUG);

    pinMode(NCE, INPUT);
    pinMode(NCONFIG, INPUT);

    setupI2C();
    setupSPIFFS();
    setupResetMode();
    setupOutputResolution();
    setupScanlines();
    setupTaskManager();
    setupCredentials();
    setupWiFi();
    setupHTTPServer();
    
    if (strlen(otaPassword)) 
    {
        setupArduinoOTA();
    }

    setOSD(false, NULL); fpgaTask.ForceLoop();
    if (reflashNeccessary && reflashNeccessary2 && reflashNeccessary3) {
        DBG_OUTPUT_PORT.printf("FPGA firmware missing or broken, reflash needed.\n");
        disableFPGA();
        if (flashTask.doStart()) {
            // [](int read, int total, bool done, int error) {
            // }
            DBG_OUTPUT_PORT.printf("   firmware flashing started.\n");
            while (!flashTask.doUpdate()) {
                yield();
            }
            flashTask.doStop();
            DBG_OUTPUT_PORT.printf("   firmware flashing done.\n");
            resetall();
        }
    }
    DBG_OUTPUT_PORT.println(">> Ready.");
}

void loop(void){
    ArduinoOTA.handle();
    taskManager.Loop();
}
