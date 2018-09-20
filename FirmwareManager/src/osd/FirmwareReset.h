#include "../global.h"
#include "../Menu.h"

Menu firmwareResetMenu("FirmwareResetMenu", (uint8_t*) OSD_FIRMWARE_RESET_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        currentMenu = previousMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_A)) {
        resetall();
        return;
    }
}, NULL, NULL, true);

