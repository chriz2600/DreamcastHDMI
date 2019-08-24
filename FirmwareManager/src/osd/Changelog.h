#include "../global.h"
#include "../Menu.h"
#include <FS.h>

#define CHNGL_BUFFER_SIZE ((MENU_WIDTH * MENU_CHNGL_RESULT_HEIGHT) + 2)

File changelogFile;
int changelogFileSize = 0;
int changelogCurrentSeek = 0;
char *changelogBuffer;
int changelogBytesRead = 0;

void changelogDisplay() {
    changelogFile.seek(changelogCurrentSeek);
    changelogBytesRead = changelogFile.readBytes((char *) changelogBuffer, CHNGL_BUFFER_SIZE - 2);
    fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_CHNGL_RESULT_LINE, (uint8_t*) changelogBuffer, [&]() {
        char up = '-';
        char down = '-';
        if (changelogCurrentSeek > 0) {
            up = 0x1e;
        }
        if (changelogFileSize > (changelogCurrentSeek + (MENU_WIDTH * MENU_CHNGL_RESULT_HEIGHT))) {
            down = 0x1f;
        }
        snprintf(changelogBuffer, 2, "%c", down);
        fpgaTask.DoWriteToOSD(39, MENU_OFFSET + MENU_BUTTON_LINE - 1, (uint8_t*) changelogBuffer, [up]() {
            snprintf(changelogBuffer, 2, "%c", up);
            fpgaTask.DoWriteToOSD(39, MENU_OFFSET + MENU_CHNGL_RESULT_LINE - 1, (uint8_t*) changelogBuffer);
        });

    });
}

Menu changelogMenu("ChangelogMenu", OSD_CHANGELOG_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        changelogFile.close();
        free(changelogBuffer);
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }

    bool isUp = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_UP);
    bool isDown = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_DOWN);
    int dispChrs = (MENU_WIDTH * MENU_CHNGL_RESULT_HEIGHT);

    if (isDown) {
        if (changelogFileSize > (changelogCurrentSeek + dispChrs)) {
            if (changelogCurrentSeek > changelogFileSize) changelogCurrentSeek = changelogFileSize;
            changelogCurrentSeek += 40;
            changelogDisplay();
        }
        return;
    }

    if (isUp) {
        if (changelogCurrentSeek > 0) {
            changelogCurrentSeek -= 40;
            if (changelogCurrentSeek < 0) changelogCurrentSeek = 0;
            changelogDisplay();
        }
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    changelogBuffer = (char*) malloc(CHNGL_BUFFER_SIZE);
    changelogCurrentSeek = 0;

    DEBUG("ChangelogMenu started");
    changelogFile = SPIFFS.open(CHANGELOG_FILE, "r");
    changelogFileSize = changelogFile.size();
    changelogBytesRead = changelogFile.readBytes((char *) changelogBuffer, CHNGL_BUFFER_SIZE - 1);

    snprintf(changelogBuffer, changelogBytesRead, "%s", changelogBuffer);
    memcpy(&menu_text[MENU_CHNGL_RESULT_LINE * MENU_WIDTH], changelogBuffer, changelogBytesRead - 1);

    if (changelogFileSize > (changelogCurrentSeek + (MENU_WIDTH * MENU_CHNGL_RESULT_HEIGHT))) {
        menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH + 39] = 0x1f;
    }

    return NO_SELECT_LINE;
}, NULL, false);
