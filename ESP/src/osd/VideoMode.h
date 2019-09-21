#include "../global.h"
#include "../Menu.h"

extern char videoMode[16];

void writeVideoMode2(String vidMode);
void resetall();

Menu videoModeSaveMenu("VideoModeSaveMenu", OSD_VIDEO_MODE_SAVE_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        resetall();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &videoModeMenu;
        currentMenu->Display();
        return;
    }
}, NULL, NULL, true);

Menu videoModeMenu("VideoModeMenu", OSD_VIDEO_MODE_MENU, MENU_VM_FIRST_SELECT_LINE, MENU_VM_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        String vidMode = VIDEO_MODE_STR_CABLE_DETECT;

        switch (menu_activeLine) {
            case MENU_VM_FORCE_VGA_LINE:
                vidMode = VIDEO_MODE_STR_FORCE_VGA;
                break;
            case MENU_VM_CABLE_DETECT_LINE:
                vidMode = VIDEO_MODE_STR_CABLE_DETECT;
                break;
            case MENU_VM_SWITCH_TRICK_LINE:
                vidMode = VIDEO_MODE_STR_SWITCH_TRICK;
                break;
        }

        if (vidMode != String(videoMode)) {
            writeVideoMode2(vidMode);
            currentMenu = &videoModeSaveMenu;
            currentMenu->Display();
        }
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // restore original menu text
    for (int i = MENU_VM_FIRST_SELECT_LINE ; i <= MENU_VM_LAST_SELECT_LINE ; i++) {
        menu_text[i * MENU_WIDTH] = '-';
    }
    String vidMode = String(videoMode);
    uint8_t line = MENU_VM_CABLE_DETECT_LINE;
    if (vidMode == VIDEO_MODE_STR_FORCE_VGA) {
        line = MENU_VM_FORCE_VGA_LINE;
    } else if (vidMode == VIDEO_MODE_STR_SWITCH_TRICK) {
        line = MENU_VM_SWITCH_TRICK_LINE;
    }

    menu_text[line * MENU_WIDTH] = '>';
    return line;
}, NULL, true);

