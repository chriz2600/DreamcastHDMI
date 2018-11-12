#ifndef WEB_H
#define WEB_H

#include "global.h"
#include <ESPAsyncTCP.h>

extern char httpAuthUser[64];
extern char httpAuthPass[64];
extern char firmwareServer[1024];

static AsyncClient *aClient = NULL;
String fname;
bool headerFound = false;
std::string responseHeader("");
std::string responseData("");

void _handleDownload(AsyncWebServerRequest *request, const char *filename, String httpGet, ProgressCallback progressCallback);
void writeSetupParameter(AsyncWebServerRequest *request, const char* param, char* target, const char* filename, unsigned int maxlen, const char* resetValue);

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
            std::string sData((char*) data);
            if (!headerFound) {
                int idx = sData.find("\r\n\r\n");
                if (idx == -1) {
                    return;
                }
                responseData.append(sData.substr(idx + 4, len - (idx + 4)));
                headerFound = true;
            } else {
                responseData.append(sData.substr(0, len));
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
    responseHeader.clear();
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
                    std::string sData((char*) data);
                    int idx = sData.find("\r\n\r\n");
                    if (idx == -1) {
                        DBG_OUTPUT_PORT.printf("header not found. Storing buffer.\n");
                        responseHeader.append(sData.substr(0, len));
                        return;
                    } else {
                        responseHeader.append(sData.substr(0, idx + 4));
                        int clstart = responseHeader.find("Content-Length: ");
                        if (clstart != -1) {
                            clstart += 16;
                            int clend = responseHeader.find("\r\n", clstart);
                            if (clend != -1) {
                                totalLength = atoi(responseHeader.substr(clstart, clend - clstart).c_str());
                            }
                        }
                        d = (uint8_t*) sData.substr(idx + 4, len - (idx + 4)).c_str();
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

#endif
