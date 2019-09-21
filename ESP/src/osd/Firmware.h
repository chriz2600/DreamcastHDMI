#include "../global.h"
#include "../Menu.h"

extern Menu firmwareConfigMenu;
extern Menu firmwareCheckMenu;
extern Menu firmwareDownloadMenu;
extern Menu firmwareFlashMenu;
extern Menu fpgaFlashMenu;
extern Menu firmwareResetMenu;
extern Menu *previousMenu;

Menu firmwareMenu("FirmwareMenu", OSD_FIRMWARE_MENU, MENU_FW_FIRST_SELECT_LINE, MENU_FW_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu->StoreMenuActiveLine(MENU_FW_FIRST_SELECT_LINE);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        switch (menu_activeLine) {
            case MENU_FW_CONFIG_LINE:
                currentMenu = &firmwareConfigMenu;
                currentMenu->Display();
                break;
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
                previousMenu = &firmwareMenu;
                currentMenu = &firmwareResetMenu;
                currentMenu->Display();
                break;
        }
        return;
    }
}, NULL, NULL, true);

