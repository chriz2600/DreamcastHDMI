#include "../global.h"
#include "../Menu.h"

extern char firmwareVariant[64];

Menu firmwareTransitionalMenu("FirmwareTransitionalMenu", OSD_FIRMWARE_TRANSITIONAL_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        _readFile("/etc/firmware_variant", firmwareVariant, 64, DEFAULT_FW_VARIANT);
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL, true);

Menu firmwareConfigMenu("FirmwareConfigMenu", OSD_FIRMWARE_CONFIG_MENU, MENU_FWCONF_FIRST_SELECT_LINE, MENU_FWCONF_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        // restore stored values
        _readFile("/etc/firmware_variant", firmwareVariant, 64, DEFAULT_FW_VARIANT);
        currentMenu = &firmwareMenu;
        currentMenu->Display();
        return;
    }

    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        if ((isRelaxedFirmware && String(firmwareVariant) == String(FIRMWARE_RELAXED_FLAVOUR)) || (!isRelaxedFirmware && String(firmwareVariant) == String(FIRMWARE_STANDARD_FLAVOUR))) {
            // configured firmware is already applied
            _writeFile("/etc/firmware_variant", firmwareVariant, 64);
            currentMenu = &firmwareMenu;
            currentMenu->Display();
        } else if (!isValidV2FPGAFirmwareBundle()) {
            currentMenu = &firmwareTransitionalMenu;
            currentMenu->Display();
        } else {
            currentMenu = &fpgaFlashMenu;
            currentMenu->Display();
        }
        return;
    }

    bool isLeft = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT);
    char buffer[MENU_WIDTH] = "";

    if (isLeft || isRight) {
        switch (menu_activeLine) {
            case MENU_FWCONF_VIEW_FLAVOUR:
                if (String(firmwareVariant) == String(FIRMWARE_RELAXED_FLAVOUR)) {
                    snprintf(firmwareVariant, 64, "%s", FIRMWARE_STANDARD_FLAVOUR);
                } else {
                    snprintf(firmwareVariant, 64, "%s", FIRMWARE_RELAXED_FLAVOUR);
                }
                snprintf(buffer, 13, "%-12s", String(firmwareVariant) == String(FIRMWARE_RELAXED_FLAVOUR) ? "Relaxed" : "Standard");
                fpgaTask.DoWriteToOSD(MENU_FWCONF_COLUMN, MENU_OFFSET + MENU_FWCONF_VIEW_FLAVOUR, (uint8_t*) buffer);
                break;
        }
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";
    snprintf(buffer, 13, "%-12s", String(firmwareVariant) == String(FIRMWARE_RELAXED_FLAVOUR) ? "Relaxed" : "Standard");
    memcpy(&menu_text[MENU_FWCONF_VIEW_FLAVOUR * MENU_WIDTH + MENU_FWCONF_COLUMN], buffer, 12);
    return MENU_FWCONF_VIEW_FLAVOUR;
}, NULL, true);
