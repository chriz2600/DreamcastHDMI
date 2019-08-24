#include "../global.h"
#include "../Menu.h"

extern FlashTask flashTask;

extern bool newFWFlashed;
extern bool gotFWFlashError;
extern bool gotFWChecksumError;
extern char md5FPGAServer[48];

bool fpgaFlashStarted;

void flashFPGACascade(int pos, bool force);
void readStoredMD5SumFlash(int pos, bool force, const char* fname, char* md5sum);
void checkStoredMD5SumFlash(int pos, bool force, int line, const char* fname, char* storedMD5Sum, char* serverMD5Sum);
ProgressCallback createFPGAFlashProgressCallback(int pos, bool force, int line);

Menu fpgaFlashMenu("FPGAFlashMenu", OSD_FIRMWARE_CONFIG_RECONFIG_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        _readFile("/etc/firmware_variant", firmwareVariant, 64, DEFAULT_FW_VARIANT);
        currentMenu = &firmwareConfigMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        if (!fpgaFlashStarted) {
            fpgaFlashStarted = true;
            newFWFlashed = false;
            gotFWFlashError = false;
            gotFWChecksumError = false;
            flashFPGACascade(0, true);
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    fpgaFlashStarted = false;
}, true);

void flashFPGACascade(int pos, bool force) {
    DEBUG2("flashFPGACascade: %i\n", pos);
    switch (pos) {
        case 0:
            currentMenu->startTransaction();
            flashFPGACascade(pos + 1, force);
            break;
        case 1:
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE, (uint8_t*) MENU_BACK_LINE, [ pos, force ]() {
                flashFPGACascade(pos + 1, force);
            });
            break;
        /*
            FPGA
        */
        case 2: // Check for FPGA firmware version
            if (force) {
                flashFPGACascade(pos + 2, force);
            } else {
                _readFile(SERVER_FPGA_MD5, md5FPGAServer, 33, DEFAULT_MD5_SUM_ALT);
                readStoredMD5SumFlash(pos, force, STAGED_FPGA_MD5, md5FPGA);
            }
            break;
        case 3:
            checkStoredMD5SumFlash(pos, force, MENU_FWCONF_RECONF_FPGA_LINE, LOCAL_FPGA_MD5, md5FPGA, md5FPGAServer);
            break;
        case 4: // Flash FPGA firmware
            flashTask.SetProgressCallback(createFPGAFlashProgressCallback(pos, force, MENU_FWCONF_RECONF_FPGA_LINE));
            taskManager.StartTask(&flashTask);
            break;
        case 5:
            flashTask.ClearProgressCallback();
            const char* result;
            if (gotFWFlashError) {
                result = (
                    "       ERROR switching firmware!        "
                    "Please try again! DO NOT restart system!"
                );
            } else if (gotFWChecksumError) {
                result = (
                    "  Checksum error on one or more files!  "
                    "    Please download firmware again!     "
                );
            } else {
                if (newFWFlashed) {
                    result = (
                        "    Firmware successfully switched!     "
                    );
                } else {
                    result = (
                        "    Firmware is already up to date!"
                    );
                }
            }
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_FWCONF_RECONF_RESULT_LINE, (uint8_t*) result, [ pos, force ]() {
                flashFPGACascade(pos + 1, force);
            });
            break;
        case 6:
            if (!gotFWFlashError) {
                _writeFile("/etc/firmware_variant", firmwareVariant, 64);
                fpgaTask.DoResetFPGA();
            }
        default:
            currentMenu->endTransaction();
            break;
    }
}

ProgressCallback createFPGAFlashProgressCallback(int pos, bool force, int line) {
    return [ pos, force, line ](int read, int total, bool done, int error) {
        if (error != NO_ERROR) {
            gotFWFlashError = true;
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[ ERROR FLASHING     ] done.", [ pos, force ]() {
                flashFPGACascade(pos + 1, force);
            });
            return;
        }

        if (done) {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[********************] done.", [ pos, force ]() {
                // IMPORTANT: do only advance here, if done is true!!!!!
                newFWFlashed |= true;
                flashFPGACascade(pos + 1, force);
            });
            return;
        }

        displayProgress(read, total, line);
    };
}
