#include "../global.h"
#include "../Menu.h"
#include "../task/DebugTask.h"

#include <HardwareSerial.h>

extern DebugTask debugTask;

Menu infoMenu("InfoMenu", (uint8_t*) OSD_INFO_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_B)) {
        taskManager.StopTask(&debugTask);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_MASK(controller_data, CTRLR_BUTTON_X)) {
        DBG_OUTPUT_PORT.printf("reset dreamcast!!!!! %x\n", controller_data);
        currentMenu->startTransaction();
        fpgaTask.Write(I2C_DC_RESET, 0, [](uint8_t Address, uint8_t Value) {
            DBG_OUTPUT_PORT.printf("reset dreamcast callback: %u\n", Value);
            waitForI2CRecover(true);
            DBG_OUTPUT_PORT.printf("reset dreamcast recover!\n");
            currentMenu->endTransaction();
        });
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    taskManager.StartTask(&debugTask);
}, true);

