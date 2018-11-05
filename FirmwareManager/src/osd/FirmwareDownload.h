#include "../global.h"
#include "../Menu.h"

bool firmwareDownloadStarted;
bool newFWDownloaded;

void handleFPGADownload(AsyncWebServerRequest *request, ProgressCallback progressCallback);
void handleESPDownload(AsyncWebServerRequest *request, ProgressCallback progressCallback);
void handleESPIndexDownload(AsyncWebServerRequest *request, ProgressCallback progressCallback);

void downloadCascade(int pos, bool forceDownload);
void readStoredMD5SumDownload(int pos, bool forceDownload, const char* fname, char* md5sum);
ProgressCallback createProgressCallback(int pos, bool forceDownload, int line);
ContentCallback createMD5DownloadCallback(int pos, bool forceDownload, int line, char* storedMD5Sum);
void displayProgress(int read, int total, int line);

Menu firmwareDownloadMenu("FirmwareDownloadMenu", (uint8_t*) OSD_FIRMWARE_DOWNLOAD_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, MENU_OK)) {
        if (!firmwareDownloadStarted) {
            firmwareDownloadStarted = true;
            newFWDownloaded = false;
            downloadCascade(0, false);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareDownloadStarted = false;
}, true);

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

