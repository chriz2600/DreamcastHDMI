#include "../global.h"
#include "../Menu.h"
#include "../task/TimeoutTask.h"

extern TimeoutTask timeoutTask;
extern uint8_t PrevCurrentResolution;
extern uint8_t CurrentResolution;
extern uint8_t CurrentResolutionData;
extern uint8_t ForceVGA;
extern uint8_t UpscalingMode;
extern uint8_t ColorExpansionMode;
extern uint8_t GammaMode;
extern char configuredResolution[16];
extern int8_t Offset240p;
extern int8_t OffsetVGA;
extern int8_t AutoOffsetVGA;
extern uint8_t ColorSpace;
extern uint8_t CurrentResetMode;

void writeCurrentResolution();
void waitForI2CRecover(bool waitForError);
uint8_t cfgRes2Int(char* intResolution);
uint8_t remapResolution(uint8_t resd);
void _osd_get_resolution(uint8_t res, char* data);
void osd_get_resolution(char* buffer);
uint8_t getScanlinesUpperPart();
uint8_t getScanlinesLowerPart();
void setScanlines(uint8_t upper, uint8_t lower, WriteCallbackHandlerFunction handler);
void setOSD(bool value, WriteCallbackHandlerFunction handler);
int8_t getEffectiveOffsetVGA();

void writeVideoOutputLine() {
    char buff[MENU_WIDTH+1];
    osd_get_resolution(buff);
    fpgaTask.DoWriteToOSD(0, MENU_WIDTH, (uint8_t*) buff);
}

void switchResolution(uint8_t newValue) {
    char data[16];
    CurrentResolution = mapResolution(newValue);
    _osd_get_resolution(CurrentResolution, data);
    DEBUG1("   switchResolution: %02x -> %02x (%s)\n", newValue, CurrentResolution, data);
    fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, [](uint8_t Address, uint8_t Value) {
        writeVideoOutputLine();
    });
}

void switchResolution() {
    char data[16];
    _osd_get_resolution(CurrentResolution, data);
    DEBUG1("CurrentResolution: %02x (%s)\n", CurrentResolution, data);
    switchResolution(CurrentResolution);
}

void safeSwitchResolution(uint8_t value, WriteCallbackHandlerFunction handler) {
    value = mapResolution(value);
    bool valueChanged = (value != CurrentResolution);
    PrevCurrentResolution = CurrentResolution;
    CurrentResolution = value;
    DEBUG("setting output resolution: %02x\n", (ForceVGA | CurrentResolution));
    currentMenu->startTransaction();
    fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, [ handler, valueChanged ](uint8_t Address, uint8_t Value) {
        DEBUG("safe switch resolution callback: %02x\n", Value);
        if (valueChanged) {
            waitForI2CRecover(false);
        }
        DEBUG("Turn FOLLOWUP save menu on!\n");
        char buff[MENU_WIDTH+1];
        osd_get_resolution(buff);
        fpgaTask.DoWriteToOSD(0, 24, (uint8_t*) buff, [ handler, Address, Value ]() {
            currentMenu->endTransaction();
            handler(Address, Value);
        });
    });
}

Menu outputResSaveMenu("OutputResSaveMenu", OSD_OUTPUT_RES_SAVE_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        taskManager.StopTask(&timeoutTask);
        safeSwitchResolution(PrevCurrentResolution, [](uint8_t Address, uint8_t Value){
            currentMenu = &outputResMenu;
            currentMenu->Display();
        });
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        taskManager.StopTask(&timeoutTask);
        writeCurrentResolution();
        currentMenu = &outputResMenu;
        currentMenu->Display();
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    timeoutTask.setTimeout(RESOLUTION_SWITCH_TIMEOUT);
    timeoutTask.setTimeoutCallback([](uint32_t timedone, bool done) {
        if (done) {
            taskManager.StopTask(&timeoutTask);
            safeSwitchResolution(PrevCurrentResolution, [](uint8_t Address, uint8_t Value){
                currentMenu = &outputResMenu;
                currentMenu->Display();
            });
            return;
        }

        char result[MENU_WIDTH] = "";
        snprintf(result, MENU_WIDTH, "           Reverting in %02ds.           ", (int)(timedone / 1000));
        fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_SS_RESULT_LINE, (uint8_t*) result);
    });
    taskManager.StartTask(&timeoutTask);
}, true);

///////////////////////////////////////////////////////////////////

Menu outputResMenu("OutputResMenu", OSD_OUTPUT_RES_MENU, MENU_OR_FIRST_SELECT_LINE, MENU_OR_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        uint8_t value = RESOLUTION_1080p;

        switch (menu_activeLine) {
            case MENU_OR_LAST_SELECT_LINE-3:
                value = RESOLUTION_VGA;
                break;
            case MENU_OR_LAST_SELECT_LINE-2:
                value = RESOLUTION_480p;
                break;
            case MENU_OR_LAST_SELECT_LINE-1:
                value = RESOLUTION_960p;
                break;
            case MENU_OR_LAST_SELECT_LINE:
                value = RESOLUTION_1080p;
                break;
        }
        if (value != remapResolution(CurrentResolution)) {
            safeSwitchResolution(value, [](uint8_t Address, uint8_t Value) {
                currentMenu = &outputResSaveMenu;
                currentMenu->Display();
            });
        }
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // restore original menu text
    for (int i = (MENU_OR_LAST_SELECT_LINE-3) ; i <= MENU_OR_LAST_SELECT_LINE ; i++) {
        menu_text[i * MENU_WIDTH] = '-';
    }
    menu_text[(MENU_OR_LAST_SELECT_LINE - cfgRes2Int(configuredResolution)) * MENU_WIDTH] = '>';
    return (MENU_OR_LAST_SELECT_LINE - remapResolution(CurrentResolution));
}, NULL, true);

void storeResolutionData(uint8_t data) {
    if ((data & 0x07) == 0) {
        CurrentResolutionData = data;
        OSDOpen = (data & RESOLUTION_DATA_OSD_STATE);
        DEBUG("resdata: %02x\n", data);
    } else {
        DEBUG("   invalid resolution data: %02x\n", data);
    }
}

uint8_t remapResolution(uint8_t resd) {
    // only store base data
    return (resd & 0x03);
}

uint8_t mapResolution(uint8_t resd, bool skipDebug) {
    uint8_t targetres = remapResolution(resd);

    if (CurrentResolutionData & RESOLUTION_DATA_ADD_LINE) {
        if (CurrentResolutionData & RESOLUTION_DATA_IS_PAL) {
            targetres |= RESOLUTION_MOD_288p;
        } else {
            targetres |= RESOLUTION_MOD_240p;
        }
    } else if (CurrentResolutionData & FORCE_GENERATE_TIMING_AND_VIDEO) {
        targetres = RESOLUTION_1080p;
    } else if (
           CurrentResolutionData & RESOLUTION_DATA_LINE_DOUBLER 
        && CurrentDeinterlaceMode576i == DEINTERLACE_MODE_PASSTHRU 
        && CurrentResolutionData & RESOLUTION_DATA_IS_PAL
    )
    {
        targetres |= RESOLUTION_MOD_576i;
    } else if (
           CurrentResolutionData & RESOLUTION_DATA_LINE_DOUBLER 
        && CurrentDeinterlaceMode480i == DEINTERLACE_MODE_PASSTHRU
    )
    {
        targetres |= RESOLUTION_MOD_480i;
    } else if (CurrentResolutionData & RESOLUTION_DATA_IS_PAL) {
        targetres |= RESOLUTION_MOD_576p;
    }

    if (!skipDebug) {
        DEBUG1("   mapResolution: resd: %02x tres: %02x crd: %02x cdm: %02x|%02x\n", resd, targetres, CurrentResolutionData, CurrentDeinterlaceMode480i, CurrentDeinterlaceMode576i);
    }
    return targetres;
}

uint8_t mapResolution(uint8_t resd) {
    return mapResolution(resd, false);
}

void _osd_get_resolution(uint8_t res, char* data) {
    switch (res) {
        case 0x00: OSD_RESOLUTION("1080p"); break;
        case 0x01: OSD_RESOLUTION("960p"); break;
        case 0x02: sprintf(data, "480p"); break;
        case 0x03: sprintf(data, "VGA"); break;
        case 0x04: sprintf(data, "288p"); break;
        case 0x05: sprintf(data, "288p"); break;
        case 0x06: sprintf(data, "288p"); break;
        case 0x07: sprintf(data, "288p"); break;
        case 0x08: sprintf(data, "576p"); break;
        case 0x09: sprintf(data, "576p"); break;
        case 0x0A: sprintf(data, "576p"); break;
        case 0x0B: sprintf(data, "576p"); break;
        case 0x10: sprintf(data, "240p_1080p"); break;
        case 0x11: sprintf(data, "240p_960p"); break;
        case 0x12: sprintf(data, "240p_480p"); break;
        case 0x13: sprintf(data, "240p_VGA"); break;
        case 0x20: sprintf(data, "480i"); break;
        case 0x21: sprintf(data, "480i"); break;
        case 0x22: sprintf(data, "480i"); break;
        case 0x23: sprintf(data, "480i"); break;
        case 0x40: sprintf(data, "576i"); break;
        case 0x41: sprintf(data, "576i"); break;
        case 0x42: sprintf(data, "576i"); break;
        case 0x43: sprintf(data, "576i"); break;
        default: sprintf(data, "UNKNOWN"); break;
    }
}

void osd_get_resolution(char* buffer) {
    uint8_t res = mapResolution(CurrentResolution, true);
    char data[16];
    const char* data2;

    if (isRelaxedFirmware) {
        data2 = " " DCHDMI_VERSION "-rlx";
    } else {
        data2 = " " DCHDMI_VERSION "-std";
    }

    _osd_get_resolution(res, data);

    snprintf(buffer, 41, "%.*s%*s", 10, data, 31, " " MENU_SPACER);
    snprintf(&buffer[40 - strlen(data2)], 10, "%s", data2);
}

void reapplyFPGAConfig() {
    /*
        re-apply all fpga related config params:
        - reset mode
        - close OSD (for now) CHECK
        - scanlines CHECK
        - offset CHECK
        - upscaling mode CHECK
        - color space CHECK
        - resolution (incl. deinterlacer mode) CHECK
    */
    DEBUG1("reapplyFPGAConfig:\n");


    fpgaTask.Write(I2C_RESET_CONF, CurrentResetMode, [](uint8_t Address, uint8_t Value) {
        DEBUG1(" -> set reset mode %d\n", CurrentResetMode);
        //taskManager.StopTask(&infoTask);
        currentMenu = &mainMenu; // reset menu
        currentMenu->StoreMenuActiveLine(MENU_M_FIRST_SELECT_LINE);
        DEBUG1(" -> menu reset\n");
        // setOSD(false, [](uint8_t Address, uint8_t Value) {
        //     DEBUG1(" -> disabled OSD\n");
            setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                DEBUG1(" -> set scanlines %d/%d\n", getScanlinesUpperPart(), getScanlinesLowerPart());
                fpgaTask.Write(I2C_VGA_OFFSET, getEffectiveOffsetVGA(), [](uint8_t Address, uint8_t Value) {
                    DEBUG1(" -> set VGA offset %d\n", OffsetVGA);
                    fpgaTask.Write(I2C_240P_OFFSET, Offset240p, [](uint8_t Address, uint8_t Value) {
                        DEBUG1(" -> set 240p offset %d\n", Offset240p);
                        fpgaTask.Write(I2C_UPSCALING_MODE, UpscalingMode, [](uint8_t Address, uint8_t Value) {
                            DEBUG1(" -> set upscaling mode %d\n", UpscalingMode);
                            fpgaTask.Write(I2C_COLOR_SPACE, ColorSpace, [](uint8_t Address, uint8_t Value) {
                                DEBUG1(" -> set color space %d\n", ColorSpace);
                                fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, fpgaTask.GetColorExpansion() | GammaMode << 3, [](uint8_t Address, uint8_t Value) {
                                    DEBUG1(" -> set color expansion and gamma %d\n", ColorExpansionMode | GammaMode << 3);
                                    switchResolution();
                                });
                            });
                        });
                    });
                });
            });
        //});
    });
}

int8_t getEffectiveOffsetVGA() {
    if (OffsetVGA == VGA_OFFSET_AUTO_MODE) {
        return AutoOffsetVGA;
    }
    return OffsetVGA;
}
