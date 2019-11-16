#include "../global.h"
#include "../Menu.h"

extern char md5FPGA[48];
extern char md5ESP[48];
extern char md5IndexHtml[48];
extern char firmwareVersion[64];
extern char firmwareVariant[64];
extern char firmwareServer[256];
extern char firmwareServerPath[256];

bool firmwareCheckStarted;
bool md5CheckResult;

void md5Cascade(int pos);
void readStoredMD5Sum(int pos, int line, const char* fname, char* md5sum);
ContentCallback createMD5Callback(int pos, int line, char* storedMD5Sum);
void getMD5SumFromServer(String host, String url, ContentCallback contentCallback);
void handleChangelogDownload(AsyncWebServerRequest *request, ProgressCallback progressCallback);
void _readFile(const char *filename, char *target, unsigned int len, const char* defaultValue);

ProgressCallback createCheckProgressCallback(int pos, int line) {
    return [ pos, line ](int read, int total, bool done, int error) {
        if (error != NO_ERROR) {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[ ERROR DOWNLOADING  ] done.", [ pos ]() {
                md5Cascade(pos + 1);
            });
            return;
        }

        if (done) {
            fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) "[********************] done.", [ pos ]() {
                md5Cascade(pos + 1);
            });
            return;
        }

        displayProgress(read, total, line);
    };
}

Menu firmwareCheckMenu("FirmwareCheckMenu", OSD_FIRMWARE_CHECK_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        if (!firmwareCheckStarted) {
            firmwareCheckStarted = true;
            md5CheckResult = false;
            md5Cascade(0);
        } else {
            currentMenu = &changelogMenu;
            currentMenu->Display();
        }
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    firmwareCheckStarted = false;
}, true);

void md5Cascade(int pos) {
    DEBUG("md5Cascade: %i\n", pos);
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
            getMD5SumFromServer(firmwareServer, REMOTE_FPGA_MD5, createMD5Callback(pos, MENU_FWC_FPGA_LINE, md5FPGA));
            break;
        case 6:
            getMD5SumFromServer(firmwareServer, REMOTE_ESP_MD5, createMD5Callback(pos, MENU_FWC_ESP_LINE, md5ESP));
            break;
        case 7:
            getMD5SumFromServer(firmwareServer, REMOTE_ESP_INDEX_MD5, createMD5Callback(pos, MENU_FWC_INDEXHTML_LINE, md5IndexHtml));
            break;
        case 8:
            handleChangelogDownload(NULL, createCheckProgressCallback(pos, MENU_FWC_CHANGELOG_LINE));
            break;
        case 9:
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
                fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE, (uint8_t*) MENU_FWC_VIEW_CHANGELOG, [ pos ]() {
                    md5Cascade(pos + 1);
                });
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

