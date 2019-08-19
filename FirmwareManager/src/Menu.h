#ifndef MENU_H
#define MENU_H

#include <inttypes.h>
#include <functional>
#include "global.h"
#include "task/FPGATask.h"
#include "keymap.h"

#define MENU_OFFSET 9
#define MENU_WIDTH 40

#define NO_SELECT_LINE 32
#define MENU_START_LINE "          " MENU_OK_STR ": Start    " MENU_CANCEL_STR ": Back           "
#define MENU_BACK_LINE  "                " MENU_CANCEL_STR ": Back                 "
#define MENU_BUTTON_LINE 12

#define MENU_RST_GDEMU_BUTTON_LINE    "     X: Reset DC  Y: GDEMU button       "
#define MENU_RST_NORMAL_BUTTON_LINE   "              X: Reset DC               "

#define MENU_OK CTRLR_RTRIGGER
#define MENU_CANCEL CTRLR_LTRIGGER

#define MENU_OK_STR "R"
#define MENU_CANCEL_STR "L"

#define MENU_M_OR 2
#define MENU_M_AVS 3
#define MENU_M_SL 4
#define MENU_M_VM 5
#define MENU_M_FW 6
#define MENU_M_WIFI 7
#define MENU_M_INF 8
#define MENU_M_FIRST_SELECT_LINE 2
#define MENU_M_LAST_SELECT_LINE MENU_M_INF
char OSD_MAIN_MENU[521] = (
    "Main Menu                               "
    "                                        "
    "- Output Resolution                     "
    "- Advanced Video Settings               "
    "- Scanlines                             "
    "- Video Mode Settings                   "
    "- Firmware Upgrade                      "
    "- WiFi Setup                            "
    "- Test/Info                             "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Select  " MENU_CANCEL_STR ": Exit            "
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
    "  '>' marks the stored setting          "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Apply   " MENU_CANCEL_STR ": Back            "
);

#define MENU_AV_DEINT 2
#define MENU_AV_240POS 3
#define MENU_AV_COLOR_SPACE 4
#define MENU_AV_UPSCALING_MODE 5
#define MENU_AV_COLUMN 24
#define MENU_AV_FIRST_SELECT_LINE 2
#define MENU_AV_LAST_SELECT_LINE MENU_AV_UPSCALING_MODE
#define MENU_AV_STD_LINE_OFFSET 1
char OSD_ADVANCED_VIDEO_MENU[521] = (
    "Advanced Video Settings                 "
    "                                        "
    "- Deinterlacer:         _______         "
    "- 240p adjust position: _______         "
    "- RGB color space:      _______         "
    "                                        "
    "                                        "
    "  left/right (d-pad): change value.     "
    "  " MENU_OK_STR ": save settings and exit.            "
    "  " MENU_CANCEL_STR ": discard changes and exit.          "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Save  " MENU_CANCEL_STR ": Cancel            "
);

#define MENU_SS_RESULT_LINE 4
char OSD_OUTPUT_RES_SAVE_MENU[521] = (
    "Output Resolution                       "
    "                                        "
    "         Keep this resolution?          "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Keep  " MENU_CANCEL_STR ": Revert            "
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
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Save  " MENU_CANCEL_STR ": Cancel            "
);

char OSD_VIDEO_MODE_SAVE_MENU[521] = (
    "Video Mode Settings                     "
    "                                        "
    "    Apply changes and reset console?    "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "         " MENU_OK_STR ": Reset  " MENU_CANCEL_STR ": Not now           "
);

char OSD_DC_RESET_CONFIRM_MENU[521] = (
    "Reset Dreamcast                         "
    "                                        "
    "      Do you really want to reset       "
    "          the dreamcast now?            "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "             Y: Full Reset              "
    "         " MENU_OK_STR ": Reset  " MENU_CANCEL_STR ": Not now           "
);

char OSD_OPT_RESET_CONFIRM_MENU[521] = (
    "GDEMU button                            "
    "                                        "
    "      Do you really want to press       "
    "          the GDEMU button?             "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "         " MENU_OK_STR ": Press  " MENU_CANCEL_STR ": Not now           "
);

#define MENU_FW_CONFIG_LINE 2
#define MENU_FW_CHECK_LINE 3
#define MENU_FW_DOWNLOAD_LINE 4
#define MENU_FW_FLASH_LINE 5
#define MENU_FW_RESET_LINE 6
#define MENU_FW_FIRST_SELECT_LINE 2
#define MENU_FW_LAST_SELECT_LINE MENU_FW_RESET_LINE
char OSD_FIRMWARE_MENU[521] = (
    "Firmware                                "
    "                                        "
    "- Configure                             "
    "- Check                                 "
    "- Download                              "
    "- Flash                                 "
    "- Restart                               "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Select  " MENU_CANCEL_STR ": Exit            "
);

#define MENU_FWCONF_VIEW_FLAVOUR 2
#define MENU_FWCONF_FIRST_SELECT_LINE 2
#define MENU_FWCONF_LAST_SELECT_LINE 2
#define MENU_FWCONF_COLUMN 20
char OSD_FIRMWARE_CONFIG_MENU[521] = (
    "Configure Firmware                      "
    "                                        "
    "- Firmware Flavour: ___________         "
    "                                        "
    "  Standard: fully HDMI compliant        "
    "  Relaxed:  relaxed HDMI timings        "
    "            allows e.g. HQ2X filtering  "
    "                                        "
    "  left/right (d-pad): change value.     "
    "  " MENU_OK_STR ": apply firmware flavour.            "
    "  " MENU_CANCEL_STR ": discard changes and exit.          "
    "                                        "
    "          " MENU_OK_STR ": Apply  " MENU_CANCEL_STR ": Cancel           "
);

#define MENU_FWCONF_RECONF_FPGA_LINE 4
#define MENU_FWCONF_RECONF_RESULT_LINE 6
char OSD_FIRMWARE_CONFIG_RECONFIG_MENU[521] = (
    "Apply firmware flavour                  "
    "                                        "
    "Flash FPGA firmware.                    "
    "                                        "
    "FPGA        [                    ]      "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    MENU_START_LINE
);

#define MENU_FWC_VIEW_CHANGELOG "       " MENU_OK_STR ": View changelog  " MENU_CANCEL_STR ": Back       "
#define MENU_FWC_FPGA_LINE 4
#define MENU_FWC_ESP_LINE 5
#define MENU_FWC_INDEXHTML_LINE 6
#define MENU_FWC_CHANGELOG_LINE 7
#define MENU_FWC_RESULT_LINE 9
char OSD_FIRMWARE_CHECK_MENU[521] = (
    "Check Firmware                          "
    "                                        "
    "Check, if newer firmware is available.  "
    "                                        "
    "FPGA        ________  ________          "
    "ESP         ________  ________          "
    "index.html  ________  ________          "
    "changelog   [                    ]      "
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
    "Restart Firmware                        "
    "                                        "
    "           Restart DCHDMI?              "
    "   This will also reset the dreamcast!  "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Ok    " MENU_CANCEL_STR ": Cancel            "
);

#define MENU_SL_ACTIVE 2
#define MENU_SL_INTENSITY 3
#define MENU_SL_ODDEVEN 4
#define MENU_SL_THICKNESS 5
#define MENU_SL_FIRST_SELECT_LINE 2
#define MENU_SL_LAST_SELECT_LINE 5
#define MENU_SL_COLUMN 12
char OSD_SCANLINES_MENU[521] = (
    "Scanlines                               "
    "                                        "
    "- On/Off:    _____                      "
    "- Intensity: _____                      "
    "- Odd/Even:  _____                      "
    "- Thickness: _____                      "
    "                                        "
    "  left/right (d-pad): change value.     "
    "  " MENU_OK_STR ": save settings and exit.            "
    "  " MENU_CANCEL_STR ": discard changes and exit.          "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Save  " MENU_CANCEL_STR ": Cancel            "
);

#define MENU_INF_RESULT_LINE 2
#define MENU_INF_RESULT_HEIGHT 9
char OSD_INFO_MENU[521] = (
    "Test/Info                               "
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
    "       " MENU_CANCEL_STR ": Back  " MENU_OK_STR ": Zero counters        "
);

#define MENU_CHNGL_RESULT_LINE 2
#define MENU_CHNGL_RESULT_HEIGHT 9
char OSD_CHANGELOG_MENU[521] = (
    "Changelog                               "
    "----------------------------------------"
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "----------------------------------------"
    "                " MENU_CANCEL_STR ": Back                 "
);

#define MENU_RST_LED_LINE 2
#define MENU_RST_GDEMU_LINE 3
#define MENU_RST_USB_GDROM_LINE 4
#define MENU_RST_FIRST_SELECT_LINE 2
#define MENU_RST_LAST_SELECT_LINE 4
char OSD_RESET_MENU[521] = (
    "Reset Options                           "
    "                                        "
    "- LED                                   "
    "- GDEMU                                 "
    "- USB-GDROM                             "
    "                                        "
    "  '>' marks the stored setting          "
    "                                        "
    "LED:       OPT -> not connected         "
    "GDEMU:     OPT -> GDEMU button          "
    "USB-GDROM: OPT -> USB-GDROM reset       "
    "                                        "
    "          " MENU_OK_STR ": Apply   " MENU_CANCEL_STR ": Back            "
);

#define MENU_WIFI_SSID_LINE 2
#define MENU_WIFI_PASSWORD_LINE 3
#define MENU_WIFI_RESTART_LINE 4
#define MENU_WIFI_FIRST_SELECT_LINE 2
#define MENU_WIFI_LAST_SELECT_LINE MENU_WIFI_RESTART_LINE
char OSD_WIFI_MENU[521] = (
    "WiFi Setup                              "
    "                                        "
    "- SSID:     ___________________________ "
    "- Password: ___________________________ "
    "- Restart to apply changes              "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "                                        "
    "   " MENU_OK_STR ": Select  Y: Reveal pw  " MENU_CANCEL_STR ": Back     "
);

#define MENU_WIFI_EDIT_NAME_LINE 2
#define MENU_WIFI_EDIT_VALUE_LINE 3
#define MENU_WIFI_EDIT_CURSOR_LINE 4
char OSD_WIFI_EDIT_MENU[521] = (
    "WiFi Setup Edit                         "
    "                                        "
    " ______________________________________ "
    "\x10______________________________________\x11"
    "                                        "
    "                                        "
    "- D-pad left/right to move cursor       "
    "- D-pad up/down to cylce thru chars     "
    "- Trailing whitespace is removed on save"
    "                                        "
    "                                        "
    "                                        "
    "          " MENU_OK_STR ": Save  " MENU_CANCEL_STR ": Cancel            "
);

typedef std::function<void(uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat)> ClickHandler;
typedef std::function<bool(uint8_t shiftcode, uint8_t chardata)> KeyboardHandler;
typedef std::function<uint8_t(uint8_t* menu_text, uint8_t menu_activeLine)> PreDisplayHook;

extern FPGATask fpgaTask;
bool OSDOpen = false;

class Menu
{
  public:
    Menu(const char* name, uint8_t* menu, uint8_t first_line, uint8_t last_line, ClickHandler handler, PreDisplayHook pre_hook, WriteCallbackHandlerFunction display_callback, bool autoUpDown) :
        name(name),
        menu_text(menu),
        first_line(first_line),
        last_line(last_line),
        handler(handler),
        pre_hook(pre_hook),
        display_callback(display_callback),
        menu_activeLine(first_line),
        inTransaction(false),
        autoUpDown(autoUpDown),
        kHandler(NULL)
    { };

    Menu(const char* name, uint8_t* menu, uint8_t first_line, uint8_t last_line, ClickHandler handler, PreDisplayHook pre_hook, WriteCallbackHandlerFunction display_callback, bool autoUpDown, KeyboardHandler kHandler) :
        name(name),
        menu_text(menu),
        first_line(first_line),
        last_line(last_line),
        handler(handler),
        pre_hook(pre_hook),
        display_callback(display_callback),
        menu_activeLine(first_line),
        inTransaction(false),
        autoUpDown(autoUpDown),
        kHandler(kHandler)
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
            menu_activeLine = pre_hook(menu_text, menu_activeLine);
        }
        fpgaTask.DoWriteToOSD(0, 9, menu_text, [&]() {
            //DEBUG("%i %i\n", menu_activeLine, MENU_OFFSET + menu_activeLine);
            fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine, display_callback);
        });
    }

    void HandleClick(uint16_t controller_data, bool isRepeat) {
        if (inTransaction) {
            DEBUG("%s in transaction!\n", name);
            return;
        }

        if (autoUpDown) {
            // pad up down is handled by menu
            if (CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_UP)) {
                menu_activeLine = menu_activeLine <= first_line ? first_line : menu_activeLine - 1;
                fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
                return;
            }
            if (CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_DOWN)) {
                menu_activeLine = menu_activeLine >= last_line ? last_line : menu_activeLine + 1;
                fpgaTask.Write(I2C_OSD_ACTIVE_LINE, MENU_OFFSET + menu_activeLine);
                return;
            }
        }
        // pass all other pads to handler
        handler(controller_data, menu_activeLine, isRepeat);
    }

    void HandleKeyboard(uint8_t shiftcode, uint8_t chardata, bool isRepeat) {
        uint16_t mapped_controller = 0;

        if (kHandler == NULL || !kHandler(shiftcode, chardata)) {
            mapped_controller = defaultKeyboardMap(shiftcode, chardata);
        }

        if (mapped_controller) {
            HandleClick(mapped_controller, isRepeat);
        }
    }

    uint16_t defaultKeyboardMap(uint8_t shiftcode, uint8_t chardata) {
        switch (chardata) {
            case KEYB_KEY_ESCAPE: // ESC
                return MENU_CANCEL;
            case KEYB_KEY_RETURN: // RETURN
                return MENU_OK;
            case KEYB_KEY_F11: // F11
                return CTRLR_BUTTON_X;
            case KEYB_KEY_F12: // F12
                return CTRLR_BUTTON_Y;
            case KEYB_KEY_RIGHT: // RIGHT ARROW
                return CTRLR_PAD_RIGHT;
            case KEYB_KEY_LEFT: // LEFT ARROW
                return CTRLR_PAD_LEFT;
            case KEYB_KEY_DOWN: // DOWN ARROW
                return CTRLR_PAD_DOWN;
            case KEYB_KEY_UP: // UP ARROW
                return CTRLR_PAD_UP;
        }
        return 0;
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
    bool autoUpDown;
    KeyboardHandler kHandler;
};

int last_progress = 100;
void displayProgress(int read, int total, int line) {
    // download size may be yet unknown
    if (total <= 0) {
        return;
    }

    int stars = (int)(read * 20 / total);
    int blanks = 20 - stars;
    int percent = (int)(read * 100 / total);
    char result[32];

    if (blanks > 0) {
        snprintf(result, 32, "[%.*s%*c] %3d%% ", stars, "********************", blanks, ' ', percent);
    } else {
        snprintf(result, 32, "[%.*s] %3d%% ", stars, "********************", percent);
    }
    fpgaTask.DoWriteToOSD(12, MENU_OFFSET + line, (uint8_t*) result);
    last_progress = (percent / 10);
}

#include "osd/Main.h"
#include "osd/OutputResolution.h"
#include "osd/AdvancedVideo.h"
#include "osd/VideoMode.h"
#include "osd/Firmware.h"
#include "osd/FirmwareConfig.h"
#include "osd/FirmwareCheck.h"
#include "osd/Changelog.h"
#include "osd/FirmwareDownload.h"
#include "osd/FirmwareFlash.h"
#include "osd/FirmwareReset.h"
#include "osd/FPGAFlash.h"
#include "osd/Scanlines.h"
#include "osd/Info.h"
#include "osd/Reset.h"
#include "osd/Wifi.h"

void setOSD(bool value, WriteCallbackHandlerFunction handler) {
    if (handler != NULL) {
        fpgaTask.Write(I2C_OSD_ENABLE, value, handler);
    } else {
        fpgaTask.Write(I2C_OSD_ENABLE, value, [](uint8_t Address, uint8_t Value) {
            OSDOpen = Value;
            DEBUG("setOSD: %u\n", OSDOpen);
        });
    }
}

void openOSD() {
    currentMenu = &mainMenu;
    setOSD(true, [](uint8_t Address, uint8_t Value) {
        currentMenu->Display();
        OSDOpen = Value;
        DEBUG("setOSD: %u\n", OSDOpen);
    });
}

void closeOSD() {
    setOSD(false, NULL);
}

FPGATask fpgaTask(1, [](uint16_t controller_data, bool isRepeat) {
    if (!isRepeat) {
        if (!OSDOpen && CHECK_BIT(controller_data, CTRLR_TRIGGER_OSD)) {
            openOSD();
            return;
        }
        if (CHECK_BIT(controller_data, CTRLR_TRIGGER_DEFAULT_RESOLUTION)) {
            DEBUG("FPGATask: switchResolution\n");
            switchResolution(RESOLUTION_VGA);
            return;
        }
    }
    if (OSDOpen) {
        //DEBUG("Menu: %s %x\n", currentMenu->Name(), controller_data);
        currentMenu->HandleClick(controller_data, isRepeat);
    }
}, [](uint8_t shiftcode, uint8_t chardata, bool isRepeat) {
    // uint8_t asciiCode = getASCIICode(shiftcode, chardata);
    // DEBUG1("kbClb: isRpt: %02x shiftcode: %02x chardata: %02x asciiCode: %02x %c\n", isRepeat, shiftcode, chardata, asciiCode, asciiCode);
    if (!isRepeat) {
        if (!OSDOpen && shiftcode == 0x03 && chardata == 0x29) {
            openOSD();
            return;
        }
    }
    if (OSDOpen) {
        currentMenu->HandleKeyboard(shiftcode, chardata, isRepeat);
    }
});

#endif
