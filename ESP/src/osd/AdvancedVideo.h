#include "../global.h"
#include "../Menu.h"

extern int8_t Offset240p;
extern int8_t OffsetVGA;
extern uint8_t UpscalingMode;
extern uint8_t ColorExpansionMode;
extern uint8_t GammaMode;
extern uint8_t ColorSpace;

void read240pOffset();
void write240pOffset();
void readVGAOffset();
void writeVGAOffset();
void readUpscalingMode();
void writeUpscalingMode();
void readColorExpansionMode();
void writeColorExpansionMode();
void readGammaMode();
void writeGammaMode();
void readColorSpace();
void writeColorSpace();
void readCurrentDeinterlaceMode480i();
void readCurrentDeinterlaceMode576i();
void writeCurrentDeinterlaceMode480i();
void writeCurrentDeinterlaceMode576i();
void safeSwitchResolution(uint8_t value, WriteCallbackHandlerFunction handler);
void writeVideoOutputLine();
int8_t getEffectiveOffsetVGA();

Menu advancedVideoMenu("AdvancedVideoMenu", OSD_ADVANCED_VIDEO_MENU, MENU_AV_FIRST_SELECT_LINE, MENU_AV_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        // restore stored values
        read240pOffset();
        readVGAOffset();
        readUpscalingMode();
        readColorExpansionMode();
        readGammaMode();
        readColorSpace();
        readCurrentDeinterlaceMode480i();
        readCurrentDeinterlaceMode576i();
        fpgaTask.Write(I2C_VGA_OFFSET, getEffectiveOffsetVGA(), [](uint8_t Address, uint8_t Value) {
            fpgaTask.Write(I2C_240P_OFFSET, Offset240p, [](uint8_t Address, uint8_t Value) {
                fpgaTask.Write(I2C_UPSCALING_MODE, UpscalingMode, [](uint8_t Address, uint8_t Value) {
                    fpgaTask.Write(I2C_COLOR_SPACE, ColorSpace, [](uint8_t Address, uint8_t Value) {
                        fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, fpgaTask.GetColorExpansion() | GammaMode << 3, [](uint8_t Address, uint8_t Value) {
                            safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                                currentMenu = &mainMenu;
                                currentMenu->Display();
                            });
                        });
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
        writeVGAOffset();
        writeUpscalingMode();
        writeColorExpansionMode();
        writeGammaMode();
        writeColorSpace();
        writeCurrentDeinterlaceMode480i();
        writeCurrentDeinterlaceMode576i();
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }

    bool isLeft = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT);

    if (isLeft || isRight) {
        switch (menu_activeLine) {
            case MENU_AV_DEINT_480I:
                CurrentDeinterlaceMode480i = (CurrentDeinterlaceMode480i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_PASSTHRU : DEINTERLACE_MODE_BOB);
                safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode480i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_DEINT_480I, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_DEINT_576I:
                CurrentDeinterlaceMode576i = (CurrentDeinterlaceMode576i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_PASSTHRU : DEINTERLACE_MODE_BOB);
                safeSwitchResolution(CurrentResolution, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode576i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_DEINT_576I, (uint8_t*) buffer);
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
            case MENU_AV_VGAPOS:
                if (isRight) {
                    if (OffsetVGA == 2) {
                        OffsetVGA = VGA_OFFSET_AUTO_MODE; // special value means auto
                    } else if (OffsetVGA == VGA_OFFSET_AUTO_MODE) {
                        OffsetVGA = 0;
                    } else if (OffsetVGA > -120) {
                        OffsetVGA = OffsetVGA - 2;
                    }
                } else if (isLeft) {
                    if (OffsetVGA == 0) {
                        OffsetVGA = VGA_OFFSET_AUTO_MODE; // special value means auto
                    } else if (OffsetVGA == VGA_OFFSET_AUTO_MODE) {
                        OffsetVGA = 2;
                    } else if (OffsetVGA < 120) {
                        OffsetVGA = OffsetVGA + 2;
                    }
                }
                fpgaTask.Write(I2C_VGA_OFFSET, getEffectiveOffsetVGA(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    if (OffsetVGA == VGA_OFFSET_AUTO_MODE) {
                        snprintf(buffer, 9, "%-8s", "auto");
                    } else {
                        snprintf(buffer, 9, "%-8d", -(OffsetVGA / 2));
                    }
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_VGAPOS, (uint8_t*) buffer);
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
            case MENU_AV_COLOR_EXPANSION:
                switch (ColorExpansionMode) {
                    case COLOR_EXP_RGB555:
                        ColorExpansionMode = isLeft ? COLOR_EXP_AUTO : COLOR_EXP_RGB565;
                        break;
                    case COLOR_EXP_RGB565:
                        ColorExpansionMode = isLeft ? COLOR_EXP_RGB555 : COLOR_EXP_OFF;
                        break;
                    case COLOR_EXP_OFF:
                        ColorExpansionMode = isLeft ? COLOR_EXP_RGB565 : COLOR_EXP_AUTO;
                        break;
                    case COLOR_EXP_AUTO:
                        ColorExpansionMode = isLeft ? COLOR_EXP_OFF : COLOR_EXP_RGB555;
                        break;
                }
                fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, fpgaTask.GetColorExpansion() | GammaMode << 3, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    switch (Value & 0x7) {
                        case COLOR_EXP_RGB555: // RGB555
                            snprintf(buffer, 9, "%-8s", "RGB555");
                            break;
                        case COLOR_EXP_RGB565: // RGB565
                            snprintf(buffer, 9, "%-8s", "RGB565");
                            break;
                        case COLOR_EXP_AUTO: // auto
                            snprintf(buffer, 9, "%-8s", "Auto");
                            break;
                        default:
                            snprintf(buffer, 9, "%-8s", "Off");
                            break;
                    }
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_COLOR_EXPANSION, (uint8_t*) buffer);
                });
                break;
            case MENU_AV_GAMMA_CORRECTION:
                if (isLeft && GammaMode > 0x0b) {
                    GammaMode--;
                } else if (isRight && GammaMode < 0x13) {
                    GammaMode++;
                }
                fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, fpgaTask.GetColorExpansion() | GammaMode << 3, [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    Value = (Value & 0xF8) >> 3;
                    if (Value == 0xF) {
                        snprintf(buffer, 9, "%-8s", "Off");
                    } else {
                        if (Value > 0xF) {
                            snprintf(buffer, 9, "+0.%-5d", (Value - 0xF));
                        } else {
                            snprintf(buffer, 9, "-0.%-5d", (0xF - Value));
                        }
                    }
                    fpgaTask.DoWriteToOSD(MENU_AV_COLUMN, MENU_OFFSET + MENU_AV_GAMMA_CORRECTION, (uint8_t*) buffer);
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

    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode480i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
    memcpy(&menu_text[MENU_AV_DEINT_480I * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    snprintf(buffer, 9, "%-8s", (CurrentDeinterlaceMode576i == DEINTERLACE_MODE_BOB ? DEINTERLACE_MODE_STR_BOB : DEINTERLACE_MODE_STR_PASSTHRU));
    memcpy(&menu_text[MENU_AV_DEINT_576I * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    snprintf(buffer, 9, "%-8s", Offset240p == 20 ? "On" : "Off");
    memcpy(&menu_text[MENU_AV_240POS * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    if (OffsetVGA == VGA_OFFSET_AUTO_MODE) {
        snprintf(buffer, 9, "%-8s", "auto");
    } else {
        snprintf(buffer, 9, "%-8d", -(OffsetVGA / 2));
    }
    memcpy(&menu_text[MENU_AV_VGAPOS * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
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
    switch (ColorExpansionMode) {
        case COLOR_EXP_RGB555: // RGB555
            snprintf(buffer, 9, "%-8s", "RGB555");
            break;
        case COLOR_EXP_RGB565: // RGB565
            snprintf(buffer, 9, "%-8s", "RGB565");
            break;
        case COLOR_EXP_AUTO: // auto
            snprintf(buffer, 9, "%-8s", "Auto");
            break;
        default:
            snprintf(buffer, 9, "%-8s", "Off");
            break;
    }
    memcpy(&menu_text[MENU_AV_COLOR_EXPANSION * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    if (GammaMode == 0xF) {
        snprintf(buffer, 9, "%-8s", "Off");
    } else {
        if (GammaMode > 0xF) {
            snprintf(buffer, 9, "+0.%-5d", (GammaMode - 0xF));
        } else {
            snprintf(buffer, 9, "-0.%-5d", (0xF - GammaMode));
        }
    }
    memcpy(&menu_text[MENU_AV_GAMMA_CORRECTION * MENU_WIDTH + MENU_AV_COLUMN], buffer, 8);
    if (isRelaxedFirmware) {
        snprintf(buffer, 33, "- Upscaling mode:       %-8s", UpscalingMode == UPSCALING_MODE_2X ? "2x" : "hq2x");
        memcpy(&menu_text[MENU_AV_UPSCALING_MODE * MENU_WIDTH /*+ MENU_AV_COLUMN*/], buffer, 32);
    }
    return MENU_AV_FIRST_SELECT_LINE;
}, NULL, false);
