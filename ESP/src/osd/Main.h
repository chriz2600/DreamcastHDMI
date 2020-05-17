#include "../global.h"
#include "../Menu.h"
#include "../util.h"

extern Menu outputResMenu;
extern Menu videoModeMenu;
extern Menu advancedVideoMenu;
extern Menu scanlinesMenu;
extern Menu firmwareMenu;
extern Menu wifiMenu;
extern Menu resetMenu;
extern Menu dcResetConfirmMenu;
extern Menu optResetConfirmMenu;
extern Menu infoMenu;
extern Menu changelogMenu;
extern Menu *currentMenu;
extern uint8_t CurrentResetMode;
extern uint8_t CurrentDeinterlaceMode480i;
extern uint8_t CurrentDeinterlaceMode576i;
extern uint8_t CurrentResolutionData;
extern uint8_t CurrentProtectedMode;

void closeOSD();
void waitForI2CRecover(bool waitForError);

Menu mainMenu("MainMenu", OSD_MAIN_MENU, MENU_M_FIRST_SELECT_LINE, MENU_M_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu->StoreMenuActiveLine(MENU_M_FIRST_SELECT_LINE);
        closeOSD();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        switch (menu_activeLine) {
            case MENU_M_OR:
                currentMenu = &outputResMenu;
                currentMenu->Display();
                break;
            case MENU_M_AVS:
                currentMenu = &advancedVideoMenu;
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
            case MENU_M_INF:
                currentMenu = &infoMenu;
                currentMenu->Display();
                break;
        }
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, CTRLR_BUTTON_X)) {
        currentMenu = &dcResetConfirmMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && (CurrentResetMode == RESET_MODE_GDEMU || CurrentResetMode == RESET_MODE_MODE) && CHECK_CTRLR_MASK(controller_data, CTRLR_BUTTON_Y)) {
        currentMenu = &optResetConfirmMenu;
        currentMenu->Display();
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    if (CurrentResetMode == RESET_MODE_GDEMU) {
        memcpy(&menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH], MENU_RST_GDEMU_BUTTON_LINE, MENU_WIDTH);
    } else if (CurrentResetMode == RESET_MODE_MODE) {
        memcpy(&menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH], MENU_RST_MODE_BUTTON_LINE, MENU_WIDTH);
    } else {
        memcpy(&menu_text[(MENU_BUTTON_LINE - 1) * MENU_WIDTH], MENU_RST_NORMAL_BUTTON_LINE, MENU_WIDTH);
    }
    return menu_activeLine;
}, NULL, true);

Menu dcResetConfirmMenu("DCResetConfirm", OSD_DC_RESET_CONFIRM_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, CTRLR_BUTTON_Y)) {
        DEBUG("full reset dreamcast!!!!! %x\n", controller_data);
        resetall();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        DEBUG("reset dreamcast!!!!! %x\n", controller_data);
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_DC_RESET, 0, [](uint8_t Address, uint8_t Value) {
            DEBUG("reset dreamcast callback: %u\n", Value);
            waitForI2CRecover(false);
            DEBUG("reset dreamcast recover!\n");
            currentMenu->endTransaction();
            currentMenu = &mainMenu;
            closeOSD();
        });
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL, true);

Menu optResetConfirmMenu("OptResetConfirm", OSD_OPT_RESET_CONFIRM_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        DEBUG("secondary reset!!!!! %x\n", controller_data);
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_OPT_RESET, 0, [](uint8_t Address, uint8_t Value) {
            DEBUG("secondary reset callback: %u\n", Value);
            waitForI2CRecover(false);
            DEBUG("secondary reset recover!\n");
            currentMenu->endTransaction();
            currentMenu = &mainMenu;
            currentMenu->Display();
        });
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    if (CurrentResetMode == RESET_MODE_GDEMU) {
        memcpy_P(menu_text, OSD_OPT_RESET_CONFIRM_MENU_GDEMU, MENU_WIDTH*4);
    } else if (CurrentResetMode == RESET_MODE_MODE) {
        memcpy_P(menu_text, OSD_OPT_RESET_CONFIRM_MENU_MODE, MENU_WIDTH*4);
    }
    return menu_activeLine;
}, NULL, true);

