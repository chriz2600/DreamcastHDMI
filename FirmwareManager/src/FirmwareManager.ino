/* 
    Dreamcast Firmware Manager
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
#include "FlashTask.h"
#include "FlashESPTask.h"
#include "FlashESPIndexTask.h"
#include "FPGATask.h"
#include "DebugTask.h"
#include "TimeoutTask.h"
#include "FlashCheckTask.h"
#include "Menu.h"

#define DEFAULT_SSID ""
#define DEFAULT_PASSWORD ""
#define DEFAULT_OTA_PASSWORD ""
#define DEFAULT_FW_SERVER "dc.i74.de"
#define DEFAULT_FW_VERSION "master"
#define DEFAULT_HTTP_USER "Test"
#define DEFAULT_HTTP_PASS "testtest"
#define DEFAULT_CONF_IP_ADDR ""
#define DEFAULT_CONF_IP_GATEWAY ""
#define DEFAULT_CONF_IP_MASK ""
#define DEFAULT_CONF_IP_DNS ""
#define DEFAULT_HOST "dc-firmware-manager"
#define DEFAULT_VIDEO_MODE VIDEO_MODE_STR_FORCE_VGA
#define DEFAULT_VIDEO_RESOLUTION RESOLUTION_STR_1080p
#define DEFAULT_SCANLINES_ACTIVE SCANLINES_DISABLED
#define DEFAULT_SCANLINES_INTENSITY "175"
#define DEFAULT_SCANLINES_ODDEVEN SCANLINES_EVEN
#define DEFAULT_SCANLINES_THICKNESS SCANLINES_THIN
#define DEFAULT_RESET_MODE RESET_MODE_STR_LED

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
bool fpgaDisabled = false;
String fname;
AsyncWebServer server(80);
SPIFlash flash(CS);
int last_error = NO_ERROR; 
int totalLength;
int readLength;

static AsyncClient *aClient = NULL;
File flashFile;
bool headerFound = false;
String header = "";
std::string responseData("");

bool OSDOpen = false;
uint8_t CurrentResolution = RESOLUTION_1080p;
uint8_t PrevCurrentResolution;
uint8_t ForceVGA = VGA_ON;
uint8_t CurrentResetMode = RESET_MODE_LED;
bool DelayVGA = false;

char md5FPGA[48];
char md5ESP[48];
char md5IndexHtml[48];

bool scanlinesActive;
int scanlinesIntensity;
bool scanlinesOddeven;
bool scanlinesThickness;

bool reflashNeccessary;
bool reflashNeccessary2;

MD5Builder md5;
TaskManager taskManager;
FlashTask flashTask(1);
FlashESPTask flashESPTask(1);
FlashESPIndexTask flashESPIndexTask(1);
DebugTask debugTask(16);
TimeoutTask timeoutTask(MsToTaskTime(100));
FlashCheckTask flashCheckTask(1, NULL);

extern Menu mainMenu;
Menu *currentMenu;
Menu *previousMenu;

// functions
void openOSD() {
    currentMenu = &mainMenu;
    setOSD(true, [](uint8_t Address, uint8_t Value) {
        currentMenu->Display();
    });
}

void closeOSD() {
    setOSD(false, NULL);
}

void setScanlines(uint8_t upper, uint8_t lower, WriteCallbackHandlerFunction handler) {
    fpgaTask.Write(I2C_SCANLINE_UPPER, upper, [ lower, handler ](uint8_t Address, uint8_t Value) {
        fpgaTask.Write(I2C_SCANLINE_LOWER, lower, handler);
    });
}

FPGATask fpgaTask(1, [](uint16_t controller_data, bool isRepeat) {
    if (!OSDOpen && !isRepeat && CHECK_BIT(controller_data, CTRLR_TRIGGER_OSD)) {
        openOSD();
        return;
    }
    if (OSDOpen) {
        //DBG_OUTPUT_PORT.printf("Menu: %s %x\n", currentMenu->Name(), controller_data);
        currentMenu->HandleClick(controller_data, isRepeat);
    }
});

// poll I2C slave and wait for a no error condition with a maximum number of tries
void waitForI2CRecover(bool waitForError) {
    int retryCount = I2C_RECOVER_TRIES;
    int prev_last_error = NO_ERROR;
    DBG_OUTPUT_PORT.printf("... PRE: prev_last_error/last_error %i (%u/%u)\n", retryCount, prev_last_error, last_error);
    while (retryCount >= 0) {
        fpgaTask.Read(I2C_PING, 1, NULL); 
        fpgaTask.ForceLoop();
        if (waitForError) {
            if (prev_last_error != NO_ERROR && last_error == NO_ERROR) break;
        } else {
            if (last_error == NO_ERROR) break;
        }
        prev_last_error = last_error;
        retryCount--;
        delayMicroseconds(I2C_RECOVER_RETRY_INTERVAL_US);
        yield();
    }
    DBG_OUTPUT_PORT.printf("... POST: prev_last_error/last_error %i (%u/%u)\n", retryCount, prev_last_error, last_error);
}

void switchResolution(uint8_t newValue) {
    CurrentResolution = newValue;
    fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, NULL);
}

void setOSD(bool value, WriteCallbackHandlerFunction handler) {
    OSDOpen = value;
    fpgaTask.Write(I2C_OSD_ENABLE, value, handler);
}

void _writeFile(const char *filename, const char *towrite, unsigned int len) {
    File f = SPIFFS.open(filename, "w");
    if (f) {
        f.write((const uint8_t*) towrite, len);
        f.close();
        DBG_OUTPUT_PORT.printf(">> _writeFile: %s:[%s]\n", filename, towrite);
    }
}

void _readFile(const char *filename, char *target, unsigned int len, const char* defaultValue) {
    bool exists = SPIFFS.exists(filename);
    bool readFromFile = false;
    if (exists) {
        File f = SPIFFS.open(filename, "r");
        if (f) {
            f.readBytes(target, len);
            f.close();
            DBG_OUTPUT_PORT.printf(">> _readFile: %s:[%s]\n", filename, target);
            readFromFile = true;
        }
    }
    if (!readFromFile) {
        snprintf(target, len, "%s", defaultValue);
        DBG_OUTPUT_PORT.printf(">> _readFile: %s:[%s] (default)\n", filename, target);
    }
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

void disableFPGA() {
    pinMode(NCE, OUTPUT);
    digitalWrite(NCE, HIGH);
    fpgaDisabled = true;
}

void enableFPGA() {
    if (fpgaDisabled) {
        digitalWrite(NCE, LOW);
        pinMode(NCE, INPUT);
        fpgaDisabled = false;
    }
}

void startFPGAConfiguration() {
    pinMode(NCONFIG, OUTPUT);
    digitalWrite(NCONFIG, LOW);
}

void endFPGAConfiguration() {
    digitalWrite(NCONFIG, HIGH);
    pinMode(NCONFIG, INPUT);    
}

void resetFPGAConfiguration() {
    startFPGAConfiguration();
    delay(1);
    endFPGAConfiguration();
}

bool _isAuthenticated(AsyncWebServerRequest *request) {
    return request->authenticate(httpAuthUser, httpAuthPass);
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if(!_isAuthenticated(request)) {
        return;
    }
    if (!index) {
        fname = filename.c_str();
        md5.begin();
        request->_tempFile = SPIFFS.open(filename, "w");
        DBG_OUTPUT_PORT.printf(">> Receiving %s\n", filename.c_str());
    }
    if (request->_tempFile) {
        if (len) {
            request->_tempFile.write(data, len);
            md5.add(data, len);
        }
        if (final) {
            DBG_OUTPUT_PORT.printf(">> MD5 calc for %s\n", fname.c_str());
            request->_tempFile.close();
            md5.calculate();
            String md5sum = md5.toString();
            _writeFile((fname + ".md5").c_str(), md5sum.c_str(), md5sum.length());
        }
    }
}

int writeProgress(uint8_t *buffer, size_t maxLen, int progress) {
    char msg[5];
    uint len = 4;
    int alen = (len > maxLen ? maxLen : len);

    sprintf(msg, "% 3i\n", progress);
    //len = strlen(msg);
    memcpy(buffer, msg, alen);
    return alen;
}

void resetall() {
    DBG_OUTPUT_PORT.printf("all reset requested...\n");
    taskManager.StopTask(&fpgaTask);
    enableFPGA();
    resetFPGAConfiguration();
    //ESP.eraseConfig();
    ESP.restart();
}

void handleFlash(AsyncWebServerRequest *request, const char *filename) {
    if (SPIFFS.exists(filename)) {
        taskManager.StartTask(&flashTask);
        request->send(200);
    } else {
        request->send(404);
    }
}

void handleESPFlash(AsyncWebServerRequest *request, const char *filename) {
    if (SPIFFS.exists(filename)) {
        taskManager.StartTask(&flashESPTask);
        request->send(200);
    } else {
        request->send(404);
    }
}

void handleESPIndexFlash(AsyncWebServerRequest *request) {
    if (SPIFFS.exists(ESP_INDEX_STAGING_FILE)) {
        taskManager.StartTask(&flashESPIndexTask);
        request->send(200);
    } else {
        request->send(404);
    }
}

void handleFPGADownload(AsyncWebServerRequest *request) {
    handleFPGADownload(request, NULL);
}

void handleESPDownload(AsyncWebServerRequest *request) {
    handleESPDownload(request, NULL);
}

void handleESPIndexDownload(AsyncWebServerRequest *request) {
    handleESPIndexDownload(request, NULL);
}

void handleFPGADownload(AsyncWebServerRequest *request, ProgressCallback progressCallback) {
    String httpGet = "GET /fw/" 
        + String(firmwareVersion) 
        + "/DCxPlus-default"
        + "." + FIRMWARE_EXTENSION 
        + " HTTP/1.0\r\nHost: dc.i74.de\r\n\r\n";

    _handleDownload(request, FIRMWARE_FILE, httpGet, progressCallback);
}

void handleESPDownload(AsyncWebServerRequest *request, ProgressCallback progressCallback) {
    String httpGet = "GET /" 
        + String(firmwareVersion) 
        + "/" + (ESP.getFlashChipSize() / 1024 / 1024) + "MB"
        + "-" + "firmware"
        + "." + ESP_FIRMWARE_EXTENSION 
        + " HTTP/1.0\r\nHost: esp.i74.de\r\n\r\n";

    _handleDownload(request, ESP_FIRMWARE_FILE, httpGet, progressCallback);
}

void handleESPIndexDownload(AsyncWebServerRequest *request, ProgressCallback progressCallback) {
    String httpGet = "GET /"
        + String(firmwareVersion)
        + "/esp.index.html.gz"
        + " HTTP/1.0\r\nHost: esp.i74.de\r\n\r\n";

    _handleDownload(request, ESP_INDEX_STAGING_FILE, httpGet, progressCallback);
}

void getMD5SumFromServer(String host, String url, ContentCallback contentCallback) {
    String httpGet = "GET " + url + " HTTP/1.0\r\nHost: " + host + "\r\n\r\n";
    headerFound = false;
    last_error = NO_ERROR;
    responseData.clear();
    aClient = new AsyncClient();
    aClient->onError([ contentCallback ](void *arg, AsyncClient *client, int error) {
        contentCallback(responseData, UNKNOWN_ERROR);
        aClient = NULL;
        delete client;
    }, NULL);

    aClient->onConnect([ httpGet, contentCallback ](void *arg, AsyncClient *client) {
        aClient->onError(NULL, NULL);

        client->onDisconnect([ contentCallback ](void *arg, AsyncClient *c) {
            contentCallback(responseData, NO_ERROR);
            aClient = NULL;
            delete c;
        }, NULL);
    
        client->onData([](void *arg, AsyncClient *c, void *data, size_t len) {
            String sData = String((char*) data);
            if (!headerFound) {
                int idx = sData.indexOf("\r\n\r\n");
                if (idx == -1) {
                    return;
                }
                responseData.append(sData.substring(idx + 4, len).c_str());
                headerFound = true;
            } else {
                responseData.append(sData.substring(0, len).c_str());
            }
        }, NULL);
    
        //send the request
        DBG_OUTPUT_PORT.printf("Requesting: %s\n", httpGet.c_str());
        client->write(httpGet.c_str());
    });

    if (!aClient->connect(firmwareServer, 80)) {
        contentCallback(responseData, UNKNOWN_ERROR);
        AsyncClient *client = aClient;
        aClient = NULL;
        delete client;
    }
}

void _handleDownload(AsyncWebServerRequest *request, const char *filename, String httpGet, ProgressCallback progressCallback) {
    headerFound = false;
    header = "";
    totalLength = -1;
    readLength = -1;
    last_error = NO_ERROR;
    md5.begin();
    flashFile = SPIFFS.open(filename, "w");

    if (flashFile) {
        aClient = new AsyncClient();

        aClient->onError([ progressCallback ](void *arg, AsyncClient *client, int error) {
            DBG_OUTPUT_PORT.println("Connect Error");
            PROGRESS_CALLBACK(false, UNKNOWN_ERROR);
            aClient = NULL;
            delete client;
        }, NULL);
    
        aClient->onConnect([ filename, httpGet, progressCallback ](void *arg, AsyncClient *client) {
            DBG_OUTPUT_PORT.println("Connected");
            //aClient->onError(NULL, NULL);

            client->onDisconnect([ filename, progressCallback ](void *arg, AsyncClient *c) {
                DBG_OUTPUT_PORT.println("onDisconnect");
                flashFile.close();
                md5.calculate();
                String md5sum = md5.toString();
                _writeFile((String(filename) + ".md5").c_str(), md5sum.c_str(), md5sum.length());
                DBG_OUTPUT_PORT.println("Disconnected");
                PROGRESS_CALLBACK(true, NO_ERROR);
                aClient = NULL;
                delete c;
            }, NULL);
        
            client->onData([ progressCallback ](void *arg, AsyncClient *c, void *data, size_t len) {
                uint8_t* d = (uint8_t*) data;

                if (!headerFound) {
                    String sData = String((char*) data);
                    int idx = sData.indexOf("\r\n\r\n");
                    if (idx == -1) {
                        DBG_OUTPUT_PORT.printf("header not found. Storing buffer.\n");
                        header += sData;
                        return;
                    } else {
                        header += sData.substring(0, idx + 4);
                        header.toLowerCase();
                        int clstart = header.indexOf("content-length: ");
                        if (clstart != -1) {
                            clstart += 16;
                            int clend = header.indexOf("\r\n", clstart);
                            if (clend != -1) {
                                totalLength = atoi(header.substring(clstart, clend).c_str());
                            }
                        }
                        d = (uint8_t*) sData.substring(idx + 4).c_str();
                        len = (len - (idx + 4));
                        headerFound = true;
                        readLength = 0;
                        DBG_OUTPUT_PORT.printf("header content length found: %i\n", totalLength);
                    }
                }
                readLength += len;
                DBG_OUTPUT_PORT.printf("write: %i, %i/%i\n", len, readLength, totalLength);
                flashFile.write(d, len);
                md5.add(d, len);
                PROGRESS_CALLBACK(false, NO_ERROR);
            }, NULL);

            //send the request
            DBG_OUTPUT_PORT.printf("Requesting: %s\n", httpGet.c_str());
            client->write(httpGet.c_str());
        }, NULL);

        DBG_OUTPUT_PORT.println("Trying to connect");
        if (!aClient->connect(firmwareServer, 80)) {
            DBG_OUTPUT_PORT.println("Connect Fail");
            AsyncClient *client = aClient;
            PROGRESS_CALLBACK(false, UNKNOWN_ERROR);
            aClient = NULL;
            delete client;
        }

        if (request != NULL) { request->send(200); }
    } else {
        if (request != NULL) { request->send(500); }
        PROGRESS_CALLBACK(false, UNKNOWN_ERROR);
    }
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

void setupWiFi() {
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

void writeSetupParameter(AsyncWebServerRequest *request, const char* param, char* target, unsigned int maxlen, const char* resetValue) {
    String _tmp = "/etc/" + String(param);
    writeSetupParameter(request, param, target, _tmp.c_str(), maxlen, resetValue);
}

void writeSetupParameter(AsyncWebServerRequest *request, const char* param, char* target, const char* filename, unsigned int maxlen, const char* resetValue) {
    if(request->hasParam(param, true)) {
        AsyncWebParameter *p = request->getParam(param, true);
        if (p->value() == "") {
            DBG_OUTPUT_PORT.printf("SPIFFS.remove: %s\n", filename);
            snprintf(target, maxlen, "%s", resetValue);
            SPIFFS.remove(filename);
        } else {
            snprintf(target, maxlen, "%s", p->value().c_str());
            _writeFile(filename, target, maxlen);
        }
    } else {
        DBG_OUTPUT_PORT.printf("no such param: %s\n", param);
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

void setupTaskManager() {
    DBG_OUTPUT_PORT.printf(">> Setting up task manager...\n");
    taskManager.Setup();
    taskManager.StartTask(&fpgaTask);
}

void readVideoMode() {
    _readFile("/etc/video/mode", videoMode, 16, DEFAULT_VIDEO_MODE);
    String vidMode = String(videoMode);

    if (vidMode == VIDEO_MODE_STR_FORCE_VGA) {
        ForceVGA = VGA_ON;
        DelayVGA = false;
    } else if (vidMode == VIDEO_MODE_STR_SWITCH_TRICK) {
        ForceVGA = VGA_ON;
        DelayVGA = true;
    } else { // default: VIDEO_MODE_STR_CABLE_DETECT
        ForceVGA = VGA_OFF;
        DelayVGA = false;
    }
}

void writeVideoMode() {
    String vidMode = VIDEO_MODE_STR_CABLE_DETECT;

    if (ForceVGA == VGA_ON && !DelayVGA) {
        vidMode = VIDEO_MODE_STR_FORCE_VGA;
    } else if (ForceVGA == VGA_ON && DelayVGA) {
        vidMode = VIDEO_MODE_STR_SWITCH_TRICK;
    }

    writeVideoMode2(vidMode);
}

void writeVideoMode2(String vidMode) {
    _writeFile("/etc/video/mode", vidMode.c_str(), 16);
    snprintf(videoMode, 16, "%s", vidMode.c_str());
}

void readCurrentResetMode() {
    _readFile("/etc/reset/mode", resetMode, 16, DEFAULT_RESET_MODE);
    CurrentResetMode = cfgRst2Int(resetMode);
}

void readCurrentResolution() {
    _readFile("/etc/video/resolution", configuredResolution, 16, DEFAULT_VIDEO_RESOLUTION);
    CurrentResolution = cfgRes2Int(configuredResolution);
}

uint8_t cfgRst2Int(char* rstMode) {
    String cfgRst = String(rstMode);

    if (cfgRst == RESET_MODE_STR_GDEMU) {
        return RESET_MODE_GDEMU;
    } else if (cfgRst == RESET_MODE_STR_USBGDROM) {
        return RESET_MODE_USBGDROM;
    }
    // default is LED
    return RESET_MODE_LED;
}

void writeCurrentResetMode() {
    String cfgRst = RESET_MODE_STR_LED;

    if (CurrentResetMode == RESET_MODE_GDEMU) {
        cfgRst = RESET_MODE_STR_GDEMU;
    } else if (CurrentResetMode == RESET_MODE_USBGDROM) {
        cfgRst = RESET_MODE_STR_USBGDROM;
    }

    _writeFile("/etc/reset/mode", cfgRst.c_str(), 16);
    snprintf(resetMode, 16, "%s", cfgRst.c_str());
}

uint8_t cfgRes2Int(char* intResolution) {
    String cfgRes = String(intResolution);

    if (cfgRes == RESOLUTION_STR_960p) {
        return RESOLUTION_960p;
    } else if (cfgRes == RESOLUTION_STR_480p) {
        return RESOLUTION_480p;
    } else if (cfgRes == RESOLUTION_STR_VGA) {
        return RESOLUTION_VGA;
    }
    // default is 1080p
    return RESOLUTION_1080p;
}

void writeCurrentResolution() {
    String cfgRes = RESOLUTION_STR_1080p;

    if (CurrentResolution == RESOLUTION_960p) {
        cfgRes = RESOLUTION_STR_960p;
    } else if (CurrentResolution == RESOLUTION_480p) {
        cfgRes = RESOLUTION_STR_480p;
    } else if (CurrentResolution == RESOLUTION_VGA) {
        cfgRes = RESOLUTION_STR_VGA;
    }

    _writeFile("/etc/video/resolution", cfgRes.c_str(), 16);
    snprintf(configuredResolution, 16, "%s", cfgRes.c_str());
}

/////////

void readScanlinesActive() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/active", buffer, 32, DEFAULT_SCANLINES_ACTIVE);
    if (strcmp(buffer, SCANLINES_ENABLED) == 0) {
        scanlinesActive = true;
        return;
    }
    scanlinesActive = false;
}

void writeScanlinesActive() {
    _writeFile("/etc/scanlines/active", scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED, 32);
}

/////////

void readScanlinesIntensity() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/intensity", buffer, 32, DEFAULT_SCANLINES_INTENSITY);
    scanlinesIntensity = atoi(buffer);
    if (scanlinesIntensity < 0) {
        scanlinesIntensity = 0;
    } else if (scanlinesIntensity > 256) {
        scanlinesIntensity = 256;
    }
}

void writeScanlinesIntensity() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", scanlinesIntensity);
    _writeFile("/etc/scanlines/intensity", buffer, 32);
}

/////////

void readScanlinesOddeven() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/oddeven", buffer, 32, DEFAULT_SCANLINES_ODDEVEN);
    if (strcmp(buffer, SCANLINES_ODD) == 0) {
        scanlinesOddeven = true;
        return;
    }
    scanlinesOddeven = false;
}

void writeScanlinesOddeven() {
    _writeFile("/etc/scanlines/oddeven", scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN, 32);
}

/////////

void readScanlinesThickness() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/thickness", buffer, 32, DEFAULT_SCANLINES_THICKNESS);
    if (strcmp(buffer, SCANLINES_THICK) == 0) {
        scanlinesThickness = true;
        return;
    }
    scanlinesThickness = false;
}

void writeScanlinesThickness() {
    _writeFile("/etc/scanlines/thickness", scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN, 32);
}

/////////
/////////

void setupResetMode() {
    readCurrentResetMode();
    DBG_OUTPUT_PORT.printf(">> Setting up reset mode: %x\n", CurrentResetMode);
    forceI2CWrite(
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

uint8_t getScanlinesUpperPart() {
    return (scanlinesIntensity >> 1);
}

uint8_t getScanlinesLowerPart() {
    return (scanlinesIntensity << 7) | (scanlinesThickness << 6) | (scanlinesOddeven << 5) | (scanlinesActive << 4);
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

bool forceI2CWrite(uint8_t addr1, uint8_t val1, uint8_t addr2, uint8_t val2) {
    int retryCount = 5000;
    int retries = 0;
    bool success = false;

    while (retryCount >= 0) {
        retries++;
        fpgaTask.Write(addr1, val1, NULL); fpgaTask.ForceLoop();
        if (last_error == NO_ERROR) { // only try second command, if first was successful
            DBG_OUTPUT_PORT.printf("   success 1st command: %u %u %i\n", addr1, val1, retries);
            fpgaTask.Write(addr2, val2, NULL); fpgaTask.ForceLoop();
        }
        retryCount--;
        if (last_error == NO_ERROR) {
            DBG_OUTPUT_PORT.printf("   success 2nd command: %u %u %i\n", addr2, val2, retries);
            success = true;
            break;
        }
        delayMicroseconds(500);
        yield();
    }
    DBG_OUTPUT_PORT.printf("   retry loops needed: %i\n", retries);
    return success;
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
    DBG_OUTPUT_PORT.printf("reflashNeccessary: %s %s\n", reflashNeccessary ? "true" : "false", reflashNeccessary2 ? "true": "false");
    if (reflashNeccessary && reflashNeccessary2) {

    }
    DBG_OUTPUT_PORT.println(">> Ready.");
}

void loop(void){
    ArduinoOTA.handle();
    taskManager.Loop();
}
