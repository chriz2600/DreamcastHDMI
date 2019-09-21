#include "../global.h"
#include "../Menu.h"
#include "../task/InfoTask.h"

extern InfoTask infoTask;

Menu infoMenu("InfoMenu", OSD_INFO_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        taskManager.StopTask(&infoTask);
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        infoTask.ResetCounters();
        return;
    }
}, NULL, [](uint8_t Address, uint8_t Value) {
    taskManager.StartTask(&infoTask);
}, true);

