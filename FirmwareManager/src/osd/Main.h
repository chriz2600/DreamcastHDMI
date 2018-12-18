#include "../global.h"
#include "../Menu.h"

extern Menu outputResMenu;
extern Menu videoModeMenu;
extern Menu scanlinesMenu;
extern Menu firmwareMenu;
extern Menu wifiMenu;
extern Menu resetMenu;
extern Menu dcResetConfirmMenu;
extern Menu optResetConfirmMenu;
extern Menu infoMenu;
extern Menu *currentMenu;
extern uint8_t CurrentResetMode;
extern uint8_t offset_240p;
extern uint8_t CurrentResolutionData;

void closeOSD();
void waitForI2CRecover(bool waitForError);
void write240pOffset();

Menu mainMenu("MainMenu", (uint8_t*) OSD_MAIN_MENU, MENU_M_FIRST_SELECT_LINE, MENU_M_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, MENU_CANCEL)) {
        currentMenu->StoreMenuActiveLine(MENU_M_FIRST_SELECT_LINE);
        closeOSD();
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, MENU_OK)) {
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
            case MENU_M_WIFI:
                currentMenu = &wifiMenu;
                currentMenu->Display();
                break;
            // case MENU_M_RST:
            //     currentMenu = &resetMenu;
            //     currentMenu->Display();
            //     break;
            case MENU_M_INF:
                currentMenu = &infoMenu;
                currentMenu->Display();
                break;
        }
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_X)) {
        currentMenu = &dcResetConfirmMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_Y)) {
        currentMenu = &optResetConfirmMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && (CurrentResolutionData & 0x80) && CHECK_MASK(controller_data, CTRLR_BUTTON_START)) {
        offset_240p = (offset_240p == 20 ? 0 : 20);
        write240pOffset();
        fpgaTask.Write(I2C_240P_OFFSET, offset_240p, [](uint8_t Address, uint8_t Value) {
            uint8_t pos = (Value == 20 ? 1 : 0);
            char buffer[MENU_WIDTH + 1];
            snprintf(buffer, MENU_WIDTH, MENU_OFFSET_240P_SETTING_LINE, pos);
            fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_BUTTON_LINE - 2, (uint8_t*) buffer);
        });
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    uint8_t pos = (offset_240p == 20 ? 1 : 0);
    char buffer[MENU_WIDTH + 1];

    if (CurrentResolutionData & 0x80) {
        snprintf(buffer, MENU_WIDTH, MENU_OFFSET_240P_SETTING_LINE, pos);
    } else {
        snprintf(buffer, MENU_WIDTH, MENU_EMPTY_LINE);
    }
    memcpy(&menu_text[(MENU_BUTTON_LINE - 2) * MENU_WIDTH], buffer, MENU_WIDTH);

    if (CurrentResetMode == RESET_MODE_GDEMU) {
        memcpy(&menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH], MENU_RST_GDEMU_BUTTON_LINE, MENU_WIDTH);
    } else {
        memcpy(&menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH], MENU_RST_NORMAL_BUTTON_LINE, MENU_WIDTH);
    }
    return menu_activeLine;
}, NULL, true);

Menu dcResetConfirmMenu("DCResetConfirm", (uint8_t*) OSD_DC_RESET_CONFIRM_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, MENU_OK)) {
        DBG_OUTPUT_PORT.printf("reset dreamcast!!!!! %x\n", controller_data);
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_DC_RESET, 0, [](uint8_t Address, uint8_t Value) {
            DBG_OUTPUT_PORT.printf("reset dreamcast callback: %u\n", Value);
            waitForI2CRecover(true);
            DBG_OUTPUT_PORT.printf("reset dreamcast recover!\n");
            currentMenu->endTransaction();
            currentMenu = &mainMenu;
            closeOSD();
        });
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL, true);

Menu optResetConfirmMenu("OptResetConfirm", (uint8_t*) OSD_OPT_RESET_CONFIRM_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, MENU_OK)) {
        DBG_OUTPUT_PORT.printf("secondary reset!!!!! %x\n", controller_data);
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_OPT_RESET, 0, [](uint8_t Address, uint8_t Value) {
            DBG_OUTPUT_PORT.printf("secondary reset callback: %u\n", Value);
            waitForI2CRecover(false);
            DBG_OUTPUT_PORT.printf("secondary reset recover!\n");
            currentMenu->endTransaction();
            currentMenu = &mainMenu;
            currentMenu->Display();
        });
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL, true);

