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
uint8_t ForceVGA = VGA_ON;
bool DelayVGA = false;

char md5FPGA[48];
char md5ESP[48];
char md5IndexHtml[48];
bool md5CheckResult;
bool newFWDownloaded;
bool newFWFlashed;

bool firmwareCheckStarted;
bool firmwareDownloadStarted;
bool firmwareFlashStarted;

bool scanlinesActive;
int scanlinesIntensity;
bool scanlinesOddeven;
bool scanlinesThickness;

MD5Builder md5;
TaskManager taskManager;
FlashTask flashTask(1);
FlashESPTask flashESPTask(1);
FlashESPIndexTask flashESPIndexTask(1);
DebugTask debugTask(8);

extern Menu mainMenu;
extern Menu outputResMenu;
extern Menu outputResSaveMenu;
extern Menu videoModeMenu;
extern Menu firmwareMenu;
extern Menu firmwareCheckMenu;
extern Menu firmwareDownloadMenu;
extern Menu firmwareFlashMenu;
extern Menu firmwareResetMenu;
extern Menu scanlinesMenu;
extern Menu infoMenu;
Menu *currentMenu;
// functions
void setOSD(bool value, WriteCallbackHandlerFunction handler);

void openOSD() {
    currentMenu = &mainMenu;
    setOSD(true, [](uint8_t Address, uint8_t Value) {
        currentMenu->Display();
    });
}

void closeOSD() {
    setOSD(false, NULL);
}

void waitForI2CRecover();
void readVideoMode();
void writeVideoMode();
void writeVideoMode2(String vidMode);
void readCurrentResolution();
void writeCurrentResolution();
uint8_t cfgRes2Int(char* intResolution);
void readScanlinesActive();
void writeScanlinesActive();
void readScanlinesIntensity();
void writeScanlinesIntensity();
void readScanlinesOddeven();
void writeScanlinesOddeven();
void readScanlinesThickness();
void writeScanlinesThickness();

void setScanlines(uint8_t upper, uint8_t lower, WriteCallbackHandlerFunction handler) {
    fpgaTask.Write(I2C_SCANLINE_UPPER, upper, [ lower, handler ](uint8_t Address, uint8_t Value) {
        fpgaTask.Write(I2C_SCANLINE_LOWER, lower, handler);
    });
}

///////////////////////////////////////////////////////////////////
// Menus start -->
///////////////////////////////////////////////////////////////////

Menu outputResSaveMenu("OutputResSaveMenu", (uint8_t*) OSD_OUTPUT_RES_SAVE_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &outputResMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        writeCurrentResolution();
        currentMenu = &outputResMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL);

///////////////////////////////////////////////////////////////////

Menu outputResMenu("OutputResMenu", (uint8_t*) OSD_OUTPUT_RES_MENU, MENU_OR_FIRST_SELECT_LINE, MENU_OR_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        uint8_t value = RESOLUTION_1080p;

        switch (menu_activeLine) {
            case MENU_OR_LAST_SELECT_LINE-3:
                value = RESOLUTION_VGA;
                break;
            case MENU_OR_LAST_SELECT_LINE-2:
                value = RESOLUTION_480p;
                break;
            case MENU_OR_LAST_SELECT_LINE-1:
                value = RESOLUTION_960p;
                break;
            case MENU_OR_LAST_SELECT_LINE:
                value = RESOLUTION_1080p;
                break;
        }

        bool valueChanged = (value != CurrentResolution);
        CurrentResolution = value;
        DBG_OUTPUT_PORT.printf("setting output resolution: %u\n", (ForceVGA | CurrentResolution));
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, [valueChanged](uint8_t Address, uint8_t Value) {
            DBG_OUTPUT_PORT.printf("switch resolution callback: %u\n", Value);
            if (valueChanged) {
                waitForI2CRecover();
            }
            DBG_OUTPUT_PORT.printf("Turn FOLLOWUP save menu on!\n");
            currentMenu->endTransaction();
            currentMenu = &outputResSaveMenu;
            currentMenu->Display();
        });
        return;
    }
}, [](uint8_t* menu_text) {
    // restore original menu text
    for (int i = (MENU_OR_LAST_SELECT_LINE-3) ; i <= MENU_OR_LAST_SELECT_LINE ; i++) {
        menu_text[i * MENU_WIDTH] = '-';
    }
    menu_text[(MENU_OR_LAST_SELECT_LINE - cfgRes2Int(configuredResolution)) * MENU_WIDTH] = '>';
    return (MENU_OR_LAST_SELECT_LINE - CurrentResolution);
}, NULL);

///////////////////////////////////////////////////////////////////

Menu videoModeMenu("VideoModeMenu", (uint8_t*) OSD_VIDEO_MODE_MENU, MENU_VM_FIRST_SELECT_LINE, MENU_VM_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        String vidMode = VIDEO_MODE_STR_CABLE_DETECT;

        switch (menu_activeLine) {
            case MENU_VM_FORCE_VGA_LINE:
                vidMode = VIDEO_MODE_STR_FORCE_VGA;
                break;
            case MENU_VM_CABLE_DETECT_LINE:
                vidMode = VIDEO_MODE_STR_CABLE_DETECT;
                break;
            case MENU_VM_SWITCH_TRICK_LINE:
                vidMode = VIDEO_MODE_STR_SWITCH_TRICK;
                break;
        }

        writeVideoMode2(vidMode);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, [](uint8_t* menu_text) {
    String vidMode = String(videoMode);
    if (vidMode == VIDEO_MODE_STR_FORCE_VGA) {
        return MENU_VM_FORCE_VGA_LINE;
    } else if (vidMode == VIDEO_MODE_STR_SWITCH_TRICK) {
        return MENU_VM_SWITCH_TRICK_LINE;
    }
    return MENU_VM_CABLE_DETECT_LINE;
}, NULL);

Menu firmwareMenu("FirmwareMenu", (uint8_t*) OSD_FIRMWARE_MENU, MENU_FW_FIRST_SELECT_LINE, MENU_FW_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu->StoreMenuActiveLine(MENU_FW_FIRST_SELECT_LINE);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        switch (menu_activeLine) {
            case MENU_FW_CHECK_LINE:
                currentMenu = &firmwareCheckMenu;
                currentMenu->Display();
                break;
            case MENU_FW_DOWNLOAD_LINE:
                currentMenu = &firmwareDownloadMenu;
                currentMenu->Display();
                break;
            case MENU_FW_FLASH_LINE:
                currentMenu = &firmwareFlashMenu;
                currentMenu->Display();
                break;
            case MENU_FW_RESET_LINE:
                currentMenu = &firmwareResetMenu;
                currentMenu->Display();
                break;
        }
        return;
    }
}, NULL, NULL);

///////////////////////////////////////////////////////////////////

Menu firmwareCheckMenu("FirmwareCheckMenu", (uint8_t*) OSD_FIRMWARE_CHECK_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        if (!firmwareCheckStarted) {
            firmwareCheckStarted = true;
            md5CheckResult = false;
            md5Cascade(0);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareCheckStarted = false;
});

void md5Cascade(int pos) {
    DBG_OUTPUT_PORT.printf("md5Cascade: %i\n", pos);
    switch (pos) {
        case 0:
            currentMenu->startTransaction();
            md5Cascade(pos + 1);
            break;
        case 1:
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE, (uint8_t*) MENU_BACK_LINE, [ pos ]() {
                md5Cascade(pos + 1);
            });
            break;
        case 2:
            readStoredMD5Sum(pos, MENU_FWC_FPGA_LINE, LOCAL_FPGA_MD5, md5FPGA);
            break;
        case 3:
            readStoredMD5Sum(pos, MENU_FWC_ESP_LINE, LOCAL_ESP_MD5, md5ESP);
            break;
        case 4:
            readStoredMD5Sum(pos, MENU_FWC_INDEXHTML_LINE, LOCAL_ESP_INDEX_MD5, md5IndexHtml);
            break;
        case 5:
            getMD5SumFromServer(REMOTE_FPGA_HOST, REMOTE_FPGA_MD5, createMD5Callback(pos, MENU_FWC_FPGA_LINE, md5FPGA));
            break;
        case 6:
            getMD5SumFromServer(REMOTE_ESP_HOST, REMOTE_ESP_MD5, createMD5Callback(pos, MENU_FWC_ESP_LINE, md5ESP));
            break;
        case 7:
            getMD5SumFromServer(REMOTE_ESP_HOST, REMOTE_ESP_INDEX_MD5, createMD5Callback(pos, MENU_FWC_INDEXHTML_LINE, md5IndexHtml));
            break;
        case 8:
            const char* result;
            if (md5CheckResult) {
                result = (
                    "     Firmware update is available!      "
                    "       Please download firmware!"
                );
            } else {
                result = (
                    "       Firmware is up to date!"
                );
            }
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_FWC_RESULT_LINE, (uint8_t*) result, [ pos ]() {
                md5Cascade(pos + 1);
            });
            break;
        default:
            currentMenu->endTransaction();
            break;
    }
}


void readStoredMD5Sum(int pos, int line, const char* fname, char* md5sum) {
    char value[9];
    _readFile(fname, md5sum, 33, DEFAULT_MD5_SUM);
    snprintf(value, 9, "%.8s", md5sum);
    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) value, [ pos ]() {
        md5Cascade(pos + 1);
    });
}

ContentCallback createMD5Callback(int pos, int line, char* storedMD5Sum) {
    return [pos, line, storedMD5Sum](std::string data, int error) {
        char md5Sum[33] = "[error!]";
        char result[32] = "";
        bool isError = (error != NO_ERROR);

        if (!isError) {
            data.copy(md5Sum, 33, 0);
        }
        if (strncmp(storedMD5Sum, md5Sum, 32) != 0) {
            snprintf(result, 19, "%.8s  %s", md5Sum, (!isError ? "Update!" : "OK"));
            md5CheckResult |= (!isError);
        } else {
            snprintf(result, 19, "%.8s", md5Sum);
        }

        fpgaTask.DoWriteToOSD(22, MENU_OFFSET + line, (uint8_t*) result, [ pos ]() {
            md5Cascade(pos + 1);
        });
    };
}

///////////////////////////////////////////////////////////////////

Menu firmwareDownloadMenu("FirmwareDownloadMenu", (uint8_t*) OSD_FIRMWARE_DOWNLOAD_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        if (!firmwareDownloadStarted) {
            firmwareDownloadStarted = true;
            newFWDownloaded = false;
            downloadCascade(0, false);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareDownloadStarted = false;
});

void downloadCascade(int pos, bool forceDownload) {
    DBG_OUTPUT_PORT.printf("downloadCascade: %i\n", pos);
    switch (pos) {
        case 0:
            currentMenu->startTransaction();
            downloadCascade(pos + 1, forceDownload);
            break;
        case 1:
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE, (uint8_t*) MENU_BACK_LINE, [ pos, forceDownload ]() {
                downloadCascade(pos + 1, forceDownload);
            });
            break;
        /*
            FPGA
        */
        case 2: // Check for FPGA firmware version
            if (forceDownload) {
                downloadCascade(pos + 2, forceDownload);
            } else {
                readStoredMD5SumDownload(pos, forceDownload, STAGED_FPGA_MD5, md5FPGA);
            }
            break;
        case 3:
            getMD5SumFromServer(REMOTE_FPGA_HOST, REMOTE_FPGA_MD5, createMD5DownloadCallback(pos, forceDownload, MENU_FWD_FPGA_LINE, md5FPGA));
            break;
        case 4: // Download FPGA firmware
            handleFPGADownload(NULL, createProgressCallback(pos, forceDownload, MENU_FWD_FPGA_LINE));
            break;
        /*
            ESP
        */
        case 5: // Check for ESP firmware version
            if (forceDownload) {
                downloadCascade(pos + 2, forceDownload);
            } else {
                readStoredMD5SumDownload(pos, forceDownload, STAGED_ESP_MD5, md5ESP);
            }
            break;
        case 6:
            getMD5SumFromServer(REMOTE_ESP_HOST, REMOTE_ESP_MD5, createMD5DownloadCallback(pos, forceDownload, MENU_FWD_ESP_LINE, md5ESP));
            break;
        case 7: // Download ESP firmware
            handleESPDownload(NULL, createProgressCallback(pos, forceDownload, MENU_FWD_ESP_LINE));
            break;
        /*
            ESP INDEX
        */
        case 8: // Check for ESP index.html version
            if (forceDownload) {
                downloadCascade(pos + 2, forceDownload);
            } else {
                readStoredMD5SumDownload(pos, forceDownload, STAGED_ESP_INDEX_MD5, md5IndexHtml);
            }
            break;
        case 9:
            getMD5SumFromServer(REMOTE_ESP_HOST, REMOTE_ESP_INDEX_MD5, createMD5DownloadCallback(pos, forceDownload, MENU_FWD_INDEXHTML_LINE, md5IndexHtml));
            break;
        case 10: // Download ESP index.html
            handleESPIndexDownload(NULL, createProgressCallback(pos, forceDownload, MENU_FWD_INDEXHTML_LINE));
            break;
        case 11:
            const char* result;
            if (newFWDownloaded) {
                result = (
                    "   Firmware successfully downloaded!    "
                    "         Please flash firmware!"
                );
            } else {
                result = (
                    "       Firmware is up to date!"
                );
            }
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_FWD_RESULT_LINE, (uint8_t*) result, [ pos, forceDownload ]() {
                downloadCascade(pos + 1, forceDownload);
            });
            break;
        default:
            currentMenu->endTransaction();
            break;
    }
}

void readStoredMD5SumDownload(int pos, bool forceDownload, const char* fname, char* md5sum) {
    char value[9] = "";
    _readFile(fname, md5sum, 33, DEFAULT_MD5_SUM);
    fpgaTask.DoWriteToOSD(0, 0, (uint8_t*) value, [ pos, forceDownload ]() {
        downloadCascade(pos + 1, forceDownload);
    });
}

ProgressCallback createProgressCallback(int pos, bool forceDownload, int line) {
    return [ pos, forceDownload, line ](int read, int total, bool done, int error) {
        if (error != NO_ERROR) {
            // TODO: handle error
            downloadCascade(pos + 1, forceDownload);
            return;
        }

        if (done) {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[********************] done.", [ pos, forceDownload ]() {
                // IMPORTANT: do only advance here, if done is true!!!!!
                newFWDownloaded |= true;
                downloadCascade(pos + 1, forceDownload);
            });
            return;
        }

        displayProgress(read, total, line);
    };
}

ContentCallback createMD5DownloadCallback(int pos, bool forceDownload, int line, char* storedMD5Sum) {
    return [pos, forceDownload, line, storedMD5Sum](std::string data, int error) {
        if (error != NO_ERROR) {
            // TODO: handle error
            downloadCascade(pos + 1, forceDownload);
            return;
        }

        char md5Sum[33];
        data.copy(md5Sum, 33, 0);

        if (strncmp(storedMD5Sum, md5Sum, 32) != 0) {
            // new firmware file available
            downloadCascade(pos + 1, forceDownload);
        } else {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "File already downloaded.    ", [ pos, forceDownload ]() {
                downloadCascade(pos + 2, forceDownload);
            });
        }
    };
}

///////////////////////////////////////////////////////////////////

Menu firmwareFlashMenu("FirmwareFlashMenu", (uint8_t*) OSD_FIRMWARE_FLASH_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        if (!firmwareFlashStarted) {
            firmwareFlashStarted = true;
            newFWFlashed = false;
            flashCascade(0, false);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareFlashStarted = false;
});

void flashCascade(int pos, bool force) {
    DBG_OUTPUT_PORT.printf("flashCascade: %i\n", pos);
    switch (pos) {
        case 0:
            currentMenu->startTransaction();
            flashCascade(pos + 1, force);
            break;
        case 1:
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE, (uint8_t*) MENU_BACK_LINE, [ pos, force ]() {
                flashCascade(pos + 1, force);
            });
            break;
        /*
            FPGA
        */
        case 2: // Check for FPGA firmware version
            if (force) {
                flashCascade(pos + 2, force);
            } else {
                readStoredMD5SumFlash(pos, force, STAGED_FPGA_MD5, md5FPGA);
            }
            break;
        case 3:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_FPGA_LINE, LOCAL_FPGA_MD5, md5FPGA);
            break;
        case 4: // Flash FPGA firmware
            flashTask.SetProgressCallback(createFlashProgressCallback(pos, force, MENU_FWF_FPGA_LINE));
            taskManager.StartTask(&flashTask);
            break;
        /*
            ESP
        */
        case 5: // Check for ESP firmware version
            flashTask.ClearProgressCallback();
            if (force) {
                flashCascade(pos + 2, force);
            } else {
                readStoredMD5SumFlash(pos, force, STAGED_ESP_MD5, md5ESP);
            }
            break;
        case 6:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_ESP_LINE, LOCAL_ESP_MD5, md5ESP);
            break;
        case 7: // Flash ESP firmware
            flashESPTask.SetProgressCallback(createFlashProgressCallback(pos, force, MENU_FWF_ESP_LINE));
            taskManager.StartTask(&flashESPTask);
            break;
        /*
            ESP INDEX
        */
        case 8: // Check for ESP index.html version
            flashESPTask.ClearProgressCallback();
            if (force) {
                flashCascade(pos + 2, force);
            } else {
                readStoredMD5SumFlash(pos, force, STAGED_ESP_INDEX_MD5, md5IndexHtml);
            }
            break;
        case 9:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_INDEXHTML_LINE, LOCAL_ESP_INDEX_MD5, md5IndexHtml);
            break;
        case 10: // Flash ESP index.html
            flashESPIndexTask.SetProgressCallback(createFlashProgressCallback(pos, force, MENU_FWF_INDEXHTML_LINE));
            taskManager.StartTask(&flashESPIndexTask);
            break;
        case 11:
            flashESPIndexTask.ClearProgressCallback();
            const char* result;
            if (newFWFlashed) {
                result = (
                    "     Firmware successfully flashed!     "
                    "          Please reset system!          "
                );
            } else {
                result = (
                    "    Firmware is already up to date!"
                );
            }
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_FWF_RESULT_LINE, (uint8_t*) result, [ pos, force ]() {
                flashCascade(pos + 1, force);
            });
            break;
        default:
            currentMenu->endTransaction();
            break;
    }
}

void readStoredMD5SumFlash(int pos, bool force, const char* fname, char* md5sum) {
    char value[9] = "";
    _readFile(fname, md5sum, 33, DEFAULT_MD5_SUM);
    fpgaTask.DoWriteToOSD(0, 0, (uint8_t*) value, [ pos, force ]() {
        flashCascade(pos + 1, force);
    });
}

void checkStoredMD5SumFlash(int pos, bool force, int line, const char* fname, char* storedMD5Sum) {
    char value[9] = "";
    char md5Sum[48] = "";
    _readFile(fname, md5Sum, 33, DEFAULT_MD5_SUM);

    DBG_OUTPUT_PORT.printf("[%s] [%s] %i\n", storedMD5Sum, md5Sum, strncmp(storedMD5Sum, md5Sum, 32));

    if (strncmp(storedMD5Sum, DEFAULT_MD5_SUM, 32) == 0) {
        fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "No file to flash available. ", [ pos, force ]() {
            flashCascade(pos + 2, force);
        });
    } else if (strncmp(storedMD5Sum, md5Sum, 32) == 0) {
        fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "File already flashed.       ", [ pos, force ]() {
            flashCascade(pos + 2, force);
        });
    } else {
        // new firmware file available
        fpgaTask.DoWriteToOSD(0, 0, (uint8_t*) value, [ pos, force ]() {
            flashCascade(pos + 1, force);
        });
    }
}

ProgressCallback createFlashProgressCallback(int pos, bool force, int line) {
    return [ pos, force, line ](int read, int total, bool done, int error) {
        if (error != NO_ERROR) {
            // TODO: handle error
            flashCascade(pos + 1, force);
            return;
        }

        if (done) {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[********************] done.", [ pos, force ]() {
                // IMPORTANT: do only advance here, if done is true!!!!!
                newFWFlashed |= true;
                flashCascade(pos + 1, force);
            });
            return;
        }

        displayProgress(read, total, line);
    };
}

///////////////////////////////////////////////////////////////////

Menu firmwareResetMenu("FirmwareResetMenu", (uint8_t*) OSD_FIRMWARE_RESET_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        resetall();
        return;
    }
}, NULL, NULL);

///////////////////////////////////////////////////////////////////

void displayProgress(int read, int total, int line) {
    // download size may be yet unknown
    if (total <= 0) {
        return;
    }

    int stars = (int)(read * 20 / total);
    int blanks = 20 - stars;
    int percent = (int)(read * 100 / total);
    char result[32];

    if (blanks > 0) {
        snprintf(result, 32, "[%.*s%*c] %3d%% ", stars, "********************", blanks, ' ', percent);
    } else {
        snprintf(result, 32, "[%.*s] %3d%% ", stars, "********************", percent);
    }
    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) result);
}

///////////////////////////////////////////////////////////////////

Menu scanlinesMenu("ScanlinesMenu", (uint8_t*) OSD_SCANLINES_MENU, MENU_SL_FIRST_SELECT_LINE, MENU_SL_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        // restoreScanlines
        readScanlinesActive();
        readScanlinesIntensity();
        readScanlinesThickness();
        readScanlinesOddeven();
        setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
            currentMenu = &mainMenu;
            currentMenu->Display();
        });
        return;
    }

    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        // restoreScanlines
        writeScanlinesActive();
        writeScanlinesIntensity();
        writeScanlinesThickness();
        writeScanlinesOddeven();
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }

    bool isLeft = CHECK_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_MASK(controller_data, CTRLR_PAD_RIGHT);

    if (isLeft || isRight) {
        switch (menu_activeLine) {
            case MENU_SL_ACTIVE:
                scanlinesActive = !scanlinesActive;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED));
                    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + MENU_SL_ACTIVE, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_THICKNESS:
                scanlinesThickness = !scanlinesThickness;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN));
                    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + MENU_SL_THICKNESS, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_ODDEVEN:
                scanlinesOddeven = !scanlinesOddeven;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN));
                    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + MENU_SL_ODDEVEN, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_INTENSITY:
                if (isLeft) {
                    if (scanlinesIntensity > 0) {
                        scanlinesIntensity -= 1;
                    }
                } else if (isRight) {
                    if (scanlinesIntensity < 256) {
                        scanlinesIntensity += 1;
                    }
                }
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6d", scanlinesIntensity);
                    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + MENU_SL_INTENSITY, (uint8_t*) buffer);
                });
                break;
        }
    }
}, [](uint8_t* menu_text) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";

    snprintf(buffer, 7, "%6s", (scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED));
    memcpy(&menu_text[MENU_SL_ACTIVE * MENU_WIDTH + 12], buffer, 6);
    snprintf(buffer, 7, "%6d", scanlinesIntensity);
    memcpy(&menu_text[MENU_SL_INTENSITY * MENU_WIDTH + 12], buffer, 6);
    snprintf(buffer, 7, "%6s", (scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN));
    memcpy(&menu_text[MENU_SL_THICKNESS * MENU_WIDTH + 12], buffer, 6);
    snprintf(buffer, 7, "%6s", (scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN));
    memcpy(&menu_text[MENU_SL_ODDEVEN * MENU_WIDTH + 12], buffer, 6);

    return MENU_SL_FIRST_SELECT_LINE;
}, NULL);

///////////////////////////////////////////////////////////////////

Menu infoMenu("InfoMenu", (uint8_t*) OSD_INFO_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        taskManager.StopTask(&debugTask);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    taskManager.StartTask(&debugTask);
});

///////////////////////////////////////////////////////////////////

Menu mainMenu("MainMenu", (uint8_t*) OSD_MAIN_MENU, MENU_M_FIRST_SELECT_LINE, MENU_M_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine) {
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu->StoreMenuActiveLine(MENU_M_FIRST_SELECT_LINE);
        closeOSD();
        return;
    }
    if (CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        switch (menu_activeLine) {
            case MENU_M_OR:
                currentMenu = &outputResMenu;
                currentMenu->Display();
                break;
            case MENU_M_VM:
                currentMenu = &videoModeMenu;
                currentMenu->Display();
                break;
            case MENU_M_SL:
                currentMenu = &scanlinesMenu;
                currentMenu->Display();
                break;
            case MENU_M_FW:
                currentMenu = &firmwareMenu;
                currentMenu->Display();
                break;
            case MENU_M_INF:
                currentMenu = &infoMenu;
                currentMenu->Display();
                break;
        }
        return;
    }
}, NULL, NULL);

///////////////////////////////////////////////////////////////////
// <-- Menus end
///////////////////////////////////////////////////////////////////

FPGATask fpgaTask(1, [](uint16_t controller_data) {
    if (!OSDOpen && CHECK_BIT(controller_data, CTRLR_TRIGGER_OSD)) {
        openOSD();
        return;
    }
    if (OSDOpen) {
        //DBG_OUTPUT_PORT.printf("Menu: %s %x\n", currentMenu->Name(), controller_data);
        currentMenu->HandleClick(controller_data);
    }
});

// poll I2C slave and wait for a no error condition with a maximum number of tries
void waitForI2CRecover() {
    int retryCount = I2C_RECOVER_TRIES;
    int prev_last_error = NO_ERROR;
    DBG_OUTPUT_PORT.printf("... PRE: prev_last_error/last_error %i (%u/%u)\n", retryCount, prev_last_error, last_error);
    while (retryCount >= 0) {
        fpgaTask.Read(I2C_PING, 1, NULL); 
        fpgaTask.ForceLoop();
        if (prev_last_error != NO_ERROR && last_error == NO_ERROR) {
            break;
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

void readCurrentResolution() {
    _readFile("/etc/video/resolution", configuredResolution, 16, DEFAULT_VIDEO_RESOLUTION);
    CurrentResolution = cfgRes2Int(configuredResolution);
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
    }
    scanlinesThickness = false;
}

void writeScanlinesThickness() {
    _writeFile("/etc/scanlines/thickness", scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN, 32);
}

/////////
/////////

void setupOutputResolution() {
    int retryCount = 5000;
    int retries = 0;

    readVideoMode();
    readCurrentResolution();

    DBG_OUTPUT_PORT.printf(">> Setting up output resolution: %x\n", ForceVGA | CurrentResolution);
    while (retryCount >= 0) {
        retries++;
        fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, NULL);
        fpgaTask.ForceLoop();
        retryCount--;
        if (last_error == NO_ERROR) {
            break;
        }
        delayMicroseconds(500);
        yield();
    }
    DBG_OUTPUT_PORT.printf("   retry loops needed: %i\n", retries);
}

uint8_t getScanlinesUpperPart() {
    return (scanlinesIntensity >> 1);
}

uint8_t getScanlinesLowerPart() {
    return (scanlinesIntensity << 7) | (scanlinesThickness << 6) | (scanlinesOddeven << 5) | (scanlinesActive << 4);
}

void setupScanlines() {
    int retryCount = 5000;
    int retries = 0;

    readScanlinesActive();
    readScanlinesIntensity();
    readScanlinesOddeven();
    readScanlinesThickness();

    uint8_t upper = getScanlinesUpperPart();
    uint8_t lower = getScanlinesLowerPart();

    DBG_OUTPUT_PORT.printf(">> Setting up scanlines:\n");
    while (retryCount >= 0) {
        bool saved_error = NO_ERROR;
        retries++;
        fpgaTask.Write(I2C_SCANLINE_UPPER, upper, NULL); fpgaTask.ForceLoop();
        saved_error = last_error;
        fpgaTask.Write(I2C_SCANLINE_LOWER, lower, NULL); fpgaTask.ForceLoop();
        retryCount--;
        if (saved_error == NO_ERROR && last_error == NO_ERROR) {
            break;
        }
        delayMicroseconds(500);
        yield();
    }
    DBG_OUTPUT_PORT.printf("   retry loops needed: %i\n", retries);
}

void setup(void) {

    DBG_OUTPUT_PORT.begin(115200);
    DBG_OUTPUT_PORT.printf("\n>> FirmwareManager starting...\n");
    DBG_OUTPUT_PORT.setDebugOutput(DEBUG);

    pinMode(NCE, INPUT);    
    pinMode(NCONFIG, INPUT);

    setupI2C();
    setupSPIFFS();
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
    DBG_OUTPUT_PORT.println(">> Ready.");
}

void loop(void){
    ArduinoOTA.handle();
    taskManager.Loop();
}
