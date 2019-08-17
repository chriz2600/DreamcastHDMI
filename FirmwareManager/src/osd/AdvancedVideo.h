#include "../global.h"
#include "../Menu.h"

extern uint8_t Offset240p;
extern uint8_t UpscalingMode;
extern uint8_t ColorSpace;

void read240pOffset();
void write240pOffset();
void readUpscalingMode();
void writeUpscalingMode();
void readColorSpace();
void writeColorSpace();
void readCurrentDeinterlaceMode();
void writeCurrentDeinterlaceMode();
void safeSwitchResolution(uint8_t value, WriteCallbackHandlerFunction handler);
void writeVideoOutputLine();

Menu advancedVideoMenu("AdvancedVideoMenu", (uint8_t*) OSD_ADVANCED_VIDEO_MENU, MENU_AV_FIRST_SELECT_LINE, MENU_AV_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        // restore stored values
        read240pOffset();
        readUpscalingMode();
        readColorSpace();
        readCurrentDeinterlaceMode();
        fpgaTask.Write(I2C_240P_OFFSET, Offset240p, [](uint8_t Address, uint8_t Value) {
            fpgaTask.Write(I2C_UPSCALING_MODE, UpscalingMode, [](uint8_t Address, uint8_t Value) {
                fpgaTask.Write(I2C_COLOR_SPACE, ColorSpace, [](uint8_t Address, uint8_t Value) {
                    safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                        currentMenu = &mainMenu;
                        currentMenu->Display();
                    });
                });
            });
        });
        return;
    }

    if (CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_UP)) {
        menu_activeLine = menu_activeLine <= MENU_AV_FIRST_SELECT_LINE ? MENU_AV_FIRST_SELECT_LINE : menu_activeLine - 1;
        fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
        currentMenu->StoreMenuActiveLine(menu_activeLine);
        return;
    }
    if (CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_DOWN)) {
        uint8_t effectiveLastLine = isRelaxedFirmware ? MENU_AV_LAST_SELECT_LINE : MENU_AV_LAST_SELECT_LINE - MENU_AV_STD_LINE_OFFSET;
        menu_activeLine = menu_activeLine >= effectiveLastLine ? effectiveLastLine : menu_activeLine + 1;
        fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
        currentMenu->StoreMenuActiveLine(menu_activeLine);
        return;
    }

    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        write240pOffset();
        writeUpscalingMode();
        writeColorSpace();
        writeCurrentDeinterlaceMode();
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }

    bool isLeft = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT);

    if (isLeft || isRight) {
        switch (menu_activeLine) {
            case MENU_AV_DEINT:
                CurrentDeinterlaceMode = (CurrentDeinterlaceMode == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_PASSTHRU : DEINTERLACE_MODE_BOB);
                safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_DEINT, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_240POS:
                Offset240p = (Offset240p == 20 ? 0 : 20);
                fpgaTask.Write(I2C_240P_OFFSET, Offset240p, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", Value == 20 ? "On" : "Off");
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_240POS, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_COLOR_SPACE:
                switch (ColorSpace) {
                    case COLOR_SPACE_AUTO:
                        ColorSpace = isLeft ? COLOR_SPACE_LIMITED : COLOR_SPACE_FULL;
                        break;
                    case COLOR_SPACE_FULL:
                        ColorSpace = isLeft ? COLOR_SPACE_AUTO : COLOR_SPACE_LIMITED;
                        break;
                    case COLOR_SPACE_LIMITED:
                        ColorSpace = isLeft ? COLOR_SPACE_FULL : COLOR_SPACE_AUTO;
                        break;
                }
                fpgaTask.Write(I2C_COLOR_SPACE, ColorSpace, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    switch (Value) {
                        case COLOR_SPACE_AUTO:
                            snprintf(buffer, 9, "%-8s", "auto");
                            break;
                        case COLOR_SPACE_FULL:
                            snprintf(buffer, 9, "%-8s", "full");
                            break;
                        case COLOR_SPACE_LIMITED:
                            snprintf(buffer, 9, "%-8s", "limited");
                            break;
                    }
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_COLOR_SPACE, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_UPSCALING_MODE:
                UpscalingMode = (UpscalingMode == UPSCALING_MODE_2X ? UPSCALING_MODE_HQ2X : UPSCALING_MODE_2X);
                fpgaTask.Write(I2C_UPSCALING_MODE, UpscalingMode, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", Value == UPSCALING_MODE_2X ? "2x" : "hq2x");
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_UPSCALING_MODE, (uint8_t*) buffer, []() {
                        writeVideoOutputLine();
                    });
                });
                break;
        }
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";

    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
    memcpy(&menu_text[MENU_AV_DEINT * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    snprintf(buffer, 9, "%-8s", Offset240p == 20 ? "On" : "Off");
    memcpy(&menu_text[MENU_AV_240POS * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    switch (ColorSpace) {
        case COLOR_SPACE_AUTO:
            snprintf(buffer, 9, "%-8s", "auto");
            break;
        case COLOR_SPACE_FULL:
            snprintf(buffer, 9, "%-8s", "full");
            break;
        case COLOR_SPACE_LIMITED:
            snprintf(buffer, 9, "%-8s", "limited");
            break;
    }
    memcpy(&menu_text[MENU_AV_COLOR_SPACE * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    if (isRelaxedFirmware) {
        snprintf(buffer, 33, "- Upscaling mode:       %-8s", UpscalingMode == UPSCALING_MODE_2X ? "2x" : "hq2x");
        memcpy(&menu_text[MENU_AV_UPSCALING_MODE * MENU_WIDTH /*+ MENU_AV_COLUMN*/], buffer, 32);
    }
    return MENU_AV_FIRST_SELECT_LINE;
}, NULL, false);
