#include "../global.h"
#include "../Menu.h"
#include "../task/TimeoutTask.h"

extern TimeoutTask timeoutTask;
extern uint8_t PrevCurrentResolution;
extern uint8_t CurrentResolution;
extern uint8_t CurrentResolutionData;
extern uint8_t ForceVGA;
extern char configuredResolution[16];

void writeCurrentResolution();
void waitForI2CRecover(bool waitForError);
uint8_t cfgRes2Int(char* intResolution);
uint8_t remapResolution(uint8_t resd);

void switchResolution(uint8_t newValue) {
    CurrentResolution = mapResolution(newValue);
    DBG_OUTPUT_PORT.printf("   switchResolution: %02x %02x\n", newValue, CurrentResolution);
    fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, NULL);
}

void switchResolution() {
    switchResolution(CurrentResolution);
}

void safeSwitchResolution(uint8_t value, WriteCallbackHandlerFunction handler) {
    value = mapResolution(value);
    bool valueChanged = (value != CurrentResolution);
    PrevCurrentResolution = CurrentResolution;
    CurrentResolution = value;
    DBG_OUTPUT_PORT.printf("setting output resolution: %02x\n", (ForceVGA | CurrentResolution));
    currentMenu->startTransaction();
    fpgaTask.Write(I2C_OUTPUT_RESOLUTION, ForceVGA | CurrentResolution, [ handler, valueChanged ](uint8_t Address, uint8_t Value) {
        DBG_OUTPUT_PORT.printf("safe switch resolution callback: %02x\n", Value);
        if (valueChanged) {
            waitForI2CRecover(false);
        }
        DBG_OUTPUT_PORT.printf("Turn FOLLOWUP save menu on!\n");
        currentMenu->endTransaction();
        handler(Address, Value);
    });
}

Menu outputResSaveMenu("OutputResSaveMenu", (uint8_t*) OSD_OUTPUT_RES_SAVE_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
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

Menu outputResMenu("OutputResMenu", (uint8_t*) OSD_OUTPUT_RES_MENU, MENU_OR_FIRST_SELECT_LINE, MENU_OR_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
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
    if (!isRepeat && (CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT) || CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT))) {
        offset_240p = (offset_240p == 20 ? 0 : 20);
        write240pOffset();
        fpgaTask.Write(I2C_240P_OFFSET, offset_240p, [](uint8_t Address, uint8_t Value) {
            char buffer[MENU_WIDTH] = "";
            snprintf(buffer, 4, "%s    ", Value == 20 ? "On" : "Off");
            fpgaTask.DoWriteToOSD(24, MENU_OFFSET + MENU_OR_240P_ADJUST_LINE, (uint8_t*) buffer);
        });
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    char buffer[MENU_WIDTH] = "";
    // restore original menu text
    for (int i = (MENU_OR_LAST_SELECT_LINE-3) ; i <= MENU_OR_LAST_SELECT_LINE ; i++) {
        menu_text[i * MENU_WIDTH] = '-';
    }
    menu_text[(MENU_OR_LAST_SELECT_LINE - cfgRes2Int(configuredResolution)) * MENU_WIDTH] = '>';

    snprintf(buffer, 4, "%s    ", offset_240p == 20 ? "On" : "Off");
    memcpy(&menu_text[MENU_OR_240P_ADJUST_LINE * MENU_WIDTH + 24], buffer, 3);

    return (MENU_OR_LAST_SELECT_LINE - remapResolution(CurrentResolution));
}, NULL, true);

void storeResolutionData(uint8_t data) {
    if ((data & 0x1F) == 0) {
        CurrentResolutionData = data;
    } else {
        DBG_OUTPUT_PORT.printf("   invalid resolution data: %02x\n", data);
    }
}

uint8_t remapResolution(uint8_t resd) {
    return resd & 0x0F;
}

uint8_t mapResolution(uint8_t resd) {
    uint8_t targetres = remapResolution(resd);

    if (CurrentResolutionData & RESOLUTION_DATA_240P) {
        targetres |= RESOLUTION_MOD_240p;
    } else if (CurrentResolutionData & RESOLUTION_DATA_LINE_DOUBLER 
            && CurrentDeinterlaceMode == DEINTERLACE_MODE_PASSTHRU) 
    {
        if (CurrentResolutionData & RESOLUTION_DATA_IS_PAL) {
            targetres |= RESOLUTION_MOD_576i;
        } else {
            targetres |= RESOLUTION_MOD_480i;
        }
    }

    DBG_OUTPUT_PORT.printf("   mapResolution: %02x %02x %02x\n", targetres, CurrentResolutionData, CurrentDeinterlaceMode);
    return targetres;
}
