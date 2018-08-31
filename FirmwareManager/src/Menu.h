#ifndef MENU_H
#define MENU_H

#include <inttypes.h>
#include <functional>
#include "global.h"
#include "FPGATask.h"

#define MENU_OFFSET 9
#define MENU_WIDTH 40

#define NO_SELECT_LINE 32
#define MENU_START_LINE "          A: Start    B: Back           "
#define MENU_BACK_LINE  "                B: Back                 "
#define MENU_BUTTON_LINE 12

#define MENU_M_OR 2
#define MENU_M_VM 3
#define MENU_M_SL 4
#define MENU_M_FW 5
#define MENU_M_INF 6
#define MENU_M_FIRST_SELECT_LINE 2
#define MENU_M_LAST_SELECT_LINE 6
char OSD_MAIN_MENU[521] = (
    "MainMenu                                "
    "                                        "
    "- Output Resolution                     "
    "- Video Mode Settings                   "
    "- Scanlines                             "
    "- Firmware Upgrade                      "
    "- Debug Info                            "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          A: Select  B: Exit            "
);

#define MENU_OR_LAST_SELECT_LINE 5
#define MENU_OR_FIRST_SELECT_LINE (MENU_OR_LAST_SELECT_LINE-3)
char OSD_OUTPUT_RES_MENU[521] = (
    "Output Resolution                       "
    "                                        "
    "- VGA                                   "
    "- 480p                                  "
    "- 960p                                  "
    "- 1080p                                 "
    "                                        "
    "  '>' marks the default setting         "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          A: Apply   B: Back            "
);

char OSD_OUTPUT_RES_SAVE_MENU[521] = (
    "Output Resolution                       "
    "                                        "
    "     Save this setting as default?      "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          A: Ok    B: Cancel            "
);

#define MENU_VM_FORCE_VGA_LINE 2
#define MENU_VM_CABLE_DETECT_LINE 3
#define MENU_VM_SWITCH_TRICK_LINE 4
#define MENU_VM_FIRST_SELECT_LINE 2
#define MENU_VM_LAST_SELECT_LINE 4
char OSD_VIDEO_MODE_MENU[521] = (
    "Video Mode Settings                     "
    "                                        "
    "- Force VGA                             "
    "- Cable Detect                          "
    "- Switch Trick VGA                      "
    "                                        "
    "  Save: After saving the Dreamcast      "
    "        has to be power cycled for      "
    "        for the changes to take         "
    "        effect.                         "
    "                                        "
    "                                        "
    "          A: Save  B: Cancel            "
);

#define MENU_FW_CHECK_LINE 2
#define MENU_FW_DOWNLOAD_LINE 3
#define MENU_FW_FLASH_LINE 4
#define MENU_FW_RESET_LINE 5
#define MENU_FW_FIRST_SELECT_LINE 2
#define MENU_FW_LAST_SELECT_LINE 5
char OSD_FIRMWARE_MENU[521] = (
    "Firmware                                "
    "                                        "
    "- Check                                 "
    "- Download                              "
    "- Flash                                 "
    "- Reset                                 "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          A: Select  B: Exit            "
);

#define MENU_FWC_FPGA_LINE 4
#define MENU_FWC_ESP_LINE 5
#define MENU_FWC_INDEXHTML_LINE 6
#define MENU_FWC_RESULT_LINE 8
char OSD_FIRMWARE_CHECK_MENU[521] = (
    "Check Firmware                          "
    "                                        "
    "Check, if newer firmware is available.  "
    "                                        "
    "FPGA        ________  ________          "
    "ESP         ________  ________          "
    "index.html  ________  ________          "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    MENU_START_LINE
);

#define MENU_FWD_FPGA_LINE 4
#define MENU_FWD_ESP_LINE 5
#define MENU_FWD_INDEXHTML_LINE 6
#define MENU_FWD_RESULT_LINE 8
char OSD_FIRMWARE_DOWNLOAD_MENU[521] = (
    "Download Firmware                       "
    "                                        "
    "Download firmware files.                "
    "                                        "
    "FPGA        [                    ]      "
    "ESP         [                    ]      "
    "index.html  [                    ]      "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    MENU_START_LINE
);

#define MENU_FWF_FPGA_LINE 4
#define MENU_FWF_ESP_LINE 5
#define MENU_FWF_INDEXHTML_LINE 6
#define MENU_FWF_RESULT_LINE 8
char OSD_FIRMWARE_FLASH_MENU[521] = (
    "Flash Firmware                          "
    "                                        "
    "Flash downloaded firmware files.        "
    "                                        "
    "FPGA        [                    ]      "
    "ESP         [                    ]      "
    "index.html  [                    ]      "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    MENU_START_LINE
);

char OSD_FIRMWARE_RESET_MENU[521] = (
    "Reset Firmware                          "
    "                                        "
    "             Reset DCHDMI?              "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          A: Ok    B: Cancel            "
);

#define MENU_SL_ACTIVE 2
#define MENU_SL_INTENSITY 3
#define MENU_SL_ODDEVEN 4
#define MENU_SL_THICKNESS 5
#define MENU_SL_FIRST_SELECT_LINE 2
#define MENU_SL_LAST_SELECT_LINE 5
char OSD_SCANLINES_MENU[521] = (
    "Scanlines                               "
    "                                        "
    "- On/Off:    _____                      "
    "- Intensity: _____                      "
    "- Odd/Even:  _____                      "
    "- Thickness: _____                      "
    "                                        "
    "  left/right (d-pad): change value.     "
    "  A: save settings and exit.            "
    "  B: discard changes and exit.          "
    "                                        "
    "                                        "
    "          A: Save  B: Cancel            "
);

#define MENU_INF_RESULT_LINE 2
#define MENU_INF_RESULT_HEIGHT 9
char OSD_INFO_MENU[521] = (
    "Debug Info                              "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                B: Back                 "
);

typedef std::function<void(uint16_t controller_data, uint8_t menu_activeLine)> ClickHandler;
typedef std::function<uint8_t(uint8_t* menu_text)> PreDisplayHook;

extern FPGATask fpgaTask;

class Menu
{
  public:
    Menu(const char* name, uint8_t* menu, uint8_t first_line, uint8_t last_line, ClickHandler handler, PreDisplayHook pre_hook, WriteCallbackHandlerFunction display_callback) :
        name(name),
        menu_text(menu),
        first_line(first_line),
        last_line(last_line),
        handler(handler),
        pre_hook(pre_hook),
        display_callback(display_callback),
        menu_activeLine(first_line),
        inTransaction(false)
    { };

    const char* Name() {
        return name;
    }

    void startTransaction() {
        inTransaction = true;
    }

    void endTransaction() {
        inTransaction = false;
    }

    void StoreMenuActiveLine(uint8_t line) {
        menu_activeLine = line;
    }

    void Display() {
        if (pre_hook != NULL) {
            menu_activeLine = pre_hook(menu_text);
        }
        fpgaTask.DoWriteToOSD(0, 9, menu_text, [&]() {
            //DBG_OUTPUT_PORT.printf("%i %i\n", menu_activeLine, MENU_OFFSET + menu_activeLine);
            fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine, display_callback);
        });
    }

    void HandleClick(uint16_t controller_data) {
        if (inTransaction) {
            DBG_OUTPUT_PORT.printf("%s in transaction!\n", name);
            return;
        }

        // pad up down is handled by menu
        if (CHECK_MASK(controller_data, CTRLR_PAD_UP)) {
            menu_activeLine = menu_activeLine <= first_line ? first_line : menu_activeLine - 1;
            fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
            return;
        }
        if (CHECK_MASK(controller_data, CTRLR_PAD_DOWN)) {
            menu_activeLine = menu_activeLine >= last_line ? last_line : menu_activeLine + 1;
            fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
            return;
        }
        // pass all other pads to handler
        handler(controller_data, menu_activeLine);
    }

    uint8_t* GetMenuText() {
        return menu_text;
    }

private:
    const char* name;
    uint8_t* menu_text;
    uint8_t first_line;
    uint8_t last_line;
    ClickHandler handler;
    PreDisplayHook pre_hook;
    WriteCallbackHandlerFunction display_callback;
    uint8_t menu_activeLine;
    bool inTransaction;
};

#endif
