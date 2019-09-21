#include "../global.h"
#include "../Menu.h"

extern FlashTask flashTask;
extern FlashESPTask flashESPTask;
extern FlashESPIndexTask flashESPIndexTask;

bool newFWFlashed;
bool firmwareFlashStarted;
bool gotFWFlashError;
bool gotFWChecksumError;

char md5FPGAServer[48];
char md5ESPServer[48];
char md5IndexHtmlServer[48];

bool isValidV2FPGAFirmwareBundle();
void flashCascade(int pos, bool force);
void readStoredMD5SumFlash(int pos, bool force, const char* fname, char* md5sum);
void checkStoredMD5SumFlash(int pos, bool force, int line, const char* fname, char* storedMD5Sum, char* serverMD5Sum);
ProgressCallback createFlashProgressCallback(int pos, bool force, int line);

Menu firmwareFlashMenu("FirmwareFlashMenu", OSD_FIRMWARE_FLASH_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        if (!firmwareFlashStarted) {
            firmwareFlashStarted = true;
            newFWFlashed = false;
            gotFWFlashError = false;
            gotFWChecksumError = false;
            flashCascade(0, false);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareFlashStarted = false;
}, true);

void flashCascade(int pos, bool force) {
    DEBUG("flashCascade: %i\n", pos);
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
                _readFile(SERVER_FPGA_MD5, md5FPGAServer, 33, DEFAULT_MD5_SUM_ALT);
                readStoredMD5SumFlash(pos, force, STAGED_FPGA_MD5, md5FPGA);
            }
            break;
        case 3:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_FPGA_LINE, LOCAL_FPGA_MD5, md5FPGA, md5FPGAServer);
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
                _readFile(SERVER_ESP_MD5, md5ESPServer, 33, DEFAULT_MD5_SUM_ALT);
                readStoredMD5SumFlash(pos, force, STAGED_ESP_MD5, md5ESP);
            }
            break;
        case 6:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_ESP_LINE, LOCAL_ESP_MD5, md5ESP, md5ESPServer);
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
                _readFile(SERVER_ESP_INDEX_MD5, md5IndexHtmlServer, 33, DEFAULT_MD5_SUM_ALT);
                readStoredMD5SumFlash(pos, force, STAGED_ESP_INDEX_MD5, md5IndexHtml);
            }
            break;
        case 9:
            checkStoredMD5SumFlash(pos, force, MENU_FWF_INDEXHTML_LINE, LOCAL_ESP_INDEX_MD5, md5IndexHtml, md5IndexHtmlServer);
            break;
        case 10: // Flash ESP index.html
            flashESPIndexTask.SetProgressCallback(createFlashProgressCallback(pos, force, MENU_FWF_INDEXHTML_LINE));
            taskManager.StartTask(&flashESPIndexTask);
            break;
        case 11:
            flashESPIndexTask.ClearProgressCallback();
            const char* result;
            if (gotFWFlashError) {
                result = (
                    "        ERROR flashing firmware!        "
                    " Please reflash! DO NOT restart system! "
                );
            } else if (gotFWChecksumError) {
                result = (
                    "  Checksum error on one or more files!  "
                    "    Please download firmware again!     "
                );
            } else {
                if (newFWFlashed) {
                    result = (
                        "     Firmware successfully flashed!     "
                        "         Please restart system!         "
                    );
                } else {
                    result = (
                        "    Firmware is already up to date!"
                    );
                }
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

void checkStoredMD5SumFlash(int pos, bool force, int line, const char* fname, char* storedMD5Sum, char* serverMD5Sum) {
    char value[9] = "";
    char md5Sum[48] = "";
    _readFile(fname, md5Sum, 33, DEFAULT_MD5_SUM);

    DEBUG("[%s] [%s] %i\n", storedMD5Sum, md5Sum, strncmp(storedMD5Sum, md5Sum, 32));

    if (strncmp(storedMD5Sum, DEFAULT_MD5_SUM, 32) == 0) {
        fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "No file to flash available. ", [ pos, force ]() {
            flashCascade(pos + 2, force);
        });
    } else if (strlen(storedMD5Sum) < 32 
     || strlen(serverMD5Sum) < 32 
     || strncmp(storedMD5Sum, serverMD5Sum, 32) != 0) 
    {
        gotFWChecksumError = true;
        fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "Checksum mismatch.          ", [ pos, force ]() {
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
            gotFWFlashError = true;
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[ ERROR FLASHING     ] done.", [ pos, force ]() {
                flashCascade(pos + 1, force);
            });
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
