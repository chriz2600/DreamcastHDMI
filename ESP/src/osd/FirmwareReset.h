#include "../global.h"
#include "../Menu.h"

Menu firmwareResetMenu("FirmwareResetMenu", OSD_FIRMWARE_RESET_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = previousMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        resetall();
        return;
    }
}, NULL, NULL, true);

