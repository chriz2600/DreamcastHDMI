#include "../global.h"
#include "../Menu.h"

extern uint8_t offset_240p;
extern uint8_t upscaling_mode;
extern uint8_t color_space;

void read240pOffset();
void write240pOffset();
void readUpscalingMode();
void writeUpscalingMode();
void readColorSpace();
void writeColorSpace();
void readCurrentDeinterlaceMode();
void writeCurrentDeinterlaceMode();
void safeSwitchResolution(uint8_t value, WriteCallbackHandlerFunction handler);

Menu advancedVideoMenu("AdvancedVideoMenu", (uint8_t*) OSD_ADVANCED_VIDEO_MENU, MENU_AV_FIRST_SELECT_LINE, MENU_AV_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        // restore stored values
        read240pOffset();
        readUpscalingMode();
        readColorSpace();
        readCurrentDeinterlaceMode();
        fpgaTask.Write(I2C_240P_OFFSET, offset_240p, [](uint8_t Address, uint8_t Value) {
            fpgaTask.Write(I2C_UPSCALING_MODE, upscaling_mode, [](uint8_t Address, uint8_t Value) {
                fpgaTask.Write(I2C_COLOR_SPACE, color_space, [](uint8_t Address, uint8_t Value) {
                    safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                        currentMenu = &mainMenu;
                        currentMenu->Display();
                    });
                });
            });
        });
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
                offset_240p = (offset_240p == 20 ? 0 : 20);
                fpgaTask.Write(I2C_240P_OFFSET, offset_240p, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", Value == 20 ? "On" : "Off");
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_240POS, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_UPSCALING_MODE:
                upscaling_mode = (upscaling_mode == UPSCALING_MODE_2X ? UPSCALING_MODE_HQ2X : UPSCALING_MODE_2X);
                fpgaTask.Write(I2C_UPSCALING_MODE, upscaling_mode, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", Value == UPSCALING_MODE_2X ? "2x" : "hq2x");
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_UPSCALING_MODE, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_COLOR_SPACE:
                switch (color_space) {
                    case COLOR_SPACE_AUTO:
                        color_space = isLeft ? COLOR_SPACE_LIMITED : COLOR_SPACE_FULL;
                        break;
                    case COLOR_SPACE_FULL:
                        color_space = isLeft ? COLOR_SPACE_AUTO : COLOR_SPACE_LIMITED;
                        break;
                    case COLOR_SPACE_LIMITED:
                        color_space = isLeft ? COLOR_SPACE_FULL : COLOR_SPACE_AUTO;
                        break;
                }
                fpgaTask.Write(I2C_COLOR_SPACE, color_space, [](uint8_t Address, uint8_t Value) {
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
        }
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";

    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
    memcpy(&menu_text[MENU_AV_DEINT * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    snprintf(buffer, 9, "%-8s", offset_240p == 20 ? "On" : "Off");
    memcpy(&menu_text[MENU_AV_240POS * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    snprintf(buffer, 9, "%-8s", upscaling_mode == UPSCALING_MODE_2X ? "2x" : "hq2x");
    memcpy(&menu_text[MENU_AV_UPSCALING_MODE * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    switch (color_space) {
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
    return MENU_AV_FIRST_SELECT_LINE;
}, NULL, true);
