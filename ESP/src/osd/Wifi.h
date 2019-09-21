#include "../global.h"
#include "../Menu.h"
#include "../keymap.h"

extern char ssid[64];
extern char password[64];
extern Menu wifiEditMenu;

extern bool inInitialSetupMode;
extern bool isHhttpAuthPassGenerated;
extern char httpAuthPass[64];
extern char httpAuthUser[64];
extern char AP_NameChar[64];
extern char WiFiAPPSK[12];

bool showPW = false;
char wifiEdit_Name[MENU_WIDTH];
char wifiEdit_Value[MENU_WIDTH];
uint8_t wifiEdit_CursorPos = 0;
uint8_t wifiEdit_activeLine = NO_SELECT_LINE;

Menu wifiMenu("WiFiMenu", OSD_WIFI_MENU, MENU_WIFI_FIRST_SELECT_LINE, MENU_WIFI_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        showPW = false;
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }

    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, CTRLR_BUTTON_Y)) {
        showPW = true;
        currentMenu->Display();
        return;
    }

    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        switch (menu_activeLine) {
            case MENU_WIFI_SSID_LINE:
                snprintf(wifiEdit_Name, 39, "SSID");
                snprintf(wifiEdit_Value, 39, "%-38s", ssid);
                break;
            case MENU_WIFI_PASSWORD_LINE:
                snprintf(wifiEdit_Name, 39, "Password");
                snprintf(wifiEdit_Value, 39, "%-38s", "");
                break;
            case MENU_WIFI_RESTART_LINE:
                previousMenu = &wifiMenu;
                currentMenu = &firmwareResetMenu;
                currentMenu->Display();
                return;
        }
        wifiEdit_activeLine = menu_activeLine;
        wifiEdit_CursorPos = 0;
        currentMenu = &wifiEditMenu;
        currentMenu->Display();
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH+1] = "";

    snprintf(buffer, 28, "%-27s", ssid);
    memcpy(&menu_text[MENU_WIFI_SSID_LINE * MENU_WIDTH + 12], buffer, 27);
    snprintf(buffer, 28, "%-27s", strlen(password) > 0 ? "<password-set>" : "<password-not-set>");
    memcpy(&menu_text[MENU_WIFI_PASSWORD_LINE * MENU_WIDTH + 12], buffer, 27);

    snprintf(buffer, 16, "%15s", WiFi.status() == WL_CONNECTED ? "[Connected]" : inInitialSetupMode ? "[Access point]" : "[Error]");
    memcpy(&menu_text[25], buffer, 15);

    uint8_t i = 3;
    IPAddress ipAddress = WiFi.localIP();

    if (inInitialSetupMode) {
        snprintf(buffer, 39, "Access point SSID:     %-15s", AP_NameChar);
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);

        snprintf(buffer, 39, "Access point password: %-15s", showPW ? WiFiAPPSK : "<hidden>");
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);

        snprintf(buffer, 39, "IP address:            192.168.4.1");
    } else {
        snprintf(buffer, 39, "IP address:            %d.%d.%d.%d", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
    }

    memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, strlen(buffer));

    if (inInitialSetupMode || isHhttpAuthPassGenerated || CurrentProtectedMode == PROTECTED_MODE_OFF) {
        snprintf(buffer, 39, "Web login username:    %-15s", httpAuthUser);
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);

        snprintf(buffer, 39, "Web login password:    %-15s", (isHhttpAuthPassGenerated || CurrentProtectedMode == PROTECTED_MODE_OFF) ? showPW ? httpAuthPass : "<hidden>" : "<password-set>");
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);

        snprintf(buffer, 41, "%-40s", "   " MENU_OK_STR ": Select  Y: Reveal pw  " MENU_CANCEL_STR ": Back     ");
    } else {
        snprintf(buffer, 39, "%-38s", "");
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);
        snprintf(buffer, 39, "%-38s", "");
        memcpy(&menu_text[(MENU_WIFI_PASSWORD_LINE + i++) * MENU_WIDTH + 2], buffer, 38);

        snprintf(buffer, 41, "%-40s", "          " MENU_OK_STR ": Select  " MENU_CANCEL_STR ": Back            ");
    }

    memcpy(&menu_text[MENU_BUTTON_LINE * MENU_WIDTH], buffer, strlen(buffer));

    return menu_activeLine;
}, NULL, true);

///////////////////////////////////////////////////////////////////

void rtrim(char *str) {
    int len = strlen(str);
    for (int i = len - 1 ; i >= 0 ; i--) {
        if (isspace(str[i])) {
            str[i] = '\0';
        } else if (isprint(str[i])) {
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////

Menu wifiEditMenu("WiFiEditMenu", OSD_WIFI_EDIT_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &wifiMenu;
        currentMenu->Display();
        return;
    }
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        char result[MENU_WIDTH] = "";
        snprintf(result, 39, "%s", wifiEdit_Value);
        rtrim(result);
        switch (wifiEdit_activeLine) {
            case MENU_WIFI_SSID_LINE:
                snprintf(ssid, 39, "%s", result);
                _writeFile("/etc/ssid", ssid, 64);
                break;
            case MENU_WIFI_PASSWORD_LINE:
                snprintf(password, 39, "%s", result);
                _writeFile("/etc/password", password, 64);
                break;
        }
        currentMenu = &wifiMenu;
        currentMenu->Display();
        return;
    }

    bool isLeft = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT);
    bool isUp = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_UP);
    bool isDown = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_DOWN);

    if (isLeft || isRight || isUp || isDown) {
        if (isLeft) {
            if (wifiEdit_CursorPos > 0) {
                wifiEdit_CursorPos--;
            }
        } else if (isRight) {
            if (wifiEdit_CursorPos < 37) {
                wifiEdit_CursorPos++;
            }
        } else if (isUp) {
            uint8_t c = wifiEdit_Value[wifiEdit_CursorPos] + 1;
            if (isprint(c)) {
                wifiEdit_Value[wifiEdit_CursorPos] = c;
            }
        } else if (isDown) {
            uint8_t c = wifiEdit_Value[wifiEdit_CursorPos] - 1;
            if (isprint(c)) {
                wifiEdit_Value[wifiEdit_CursorPos] = c;
            }
        }
        currentMenu->Display();
        return;
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";
    char buffer2[MENU_WIDTH] = "";

    snprintf(buffer, 39, "%-38s", wifiEdit_Name);
    memcpy(&menu_text[MENU_WIFI_EDIT_NAME_LINE * MENU_WIDTH + 1], buffer, 38);
    snprintf(buffer, 39, "%-38s", wifiEdit_Value);
    memcpy(&menu_text[MENU_WIFI_EDIT_VALUE_LINE * MENU_WIDTH + 1], buffer, 38);
    snprintf(buffer2, 39, "%*c", wifiEdit_CursorPos + 1, '^');
    snprintf(buffer, 39, "%-38s", buffer2);
    memcpy(&menu_text[MENU_WIFI_EDIT_CURSOR_LINE * MENU_WIDTH + 1], buffer, 38);

    return MENU_WIFI_EDIT_VALUE_LINE;
}, NULL, false, [](uint8_t shiftcode, uint8_t chardata) {
    char result[MENU_WIDTH] = "";
    int len;

    switch (chardata) {
        case KEYB_KEY_POS1:
            wifiEdit_CursorPos = 0;
            currentMenu->Display();
            return true;
        case KEYB_KEY_END:
            snprintf(result, 39, "%s", wifiEdit_Value);
            rtrim(result);
            len = strlen(result);
            wifiEdit_CursorPos = len <= 37 ? len : 37;
            currentMenu->Display();
            return true;
        case KEYB_KEY_BACKSPACE:
            if (wifiEdit_CursorPos > 0 && wifiEdit_CursorPos <= 37) {
                memcpy(&result, &wifiEdit_Value, wifiEdit_CursorPos - 1);
                strcpy(&result[wifiEdit_CursorPos-1], &wifiEdit_Value[wifiEdit_CursorPos]);
                rtrim(result);
                snprintf(wifiEdit_Value, 39, "%-38s", result);
                wifiEdit_CursorPos--;
                currentMenu->Display();
            }
            return true;
        case KEYB_KEY_DELETE:
            if (wifiEdit_CursorPos >= 0 && wifiEdit_CursorPos < 37) {
                char result[40] = "";
                memcpy(&result, &wifiEdit_Value, wifiEdit_CursorPos);
                strcpy(&result[wifiEdit_CursorPos], &wifiEdit_Value[wifiEdit_CursorPos+1]);
                rtrim(result);
                snprintf(wifiEdit_Value, 39, "%-38s", result);
                currentMenu->Display();
            }
            return true;
    }

    uint8_t c = getASCIICode(shiftcode, chardata);
    if (isprint(c)) {
        if (wifiEdit_CursorPos >= 0 && wifiEdit_CursorPos <= 37) {
            char result[40] = "";
            memcpy(&result, &wifiEdit_Value, wifiEdit_CursorPos);
            result[wifiEdit_CursorPos] = c;
            if (wifiEdit_CursorPos < 37) {
                strcpy(&result[wifiEdit_CursorPos+1], &wifiEdit_Value[wifiEdit_CursorPos]);
            }
            rtrim(result);
            snprintf(wifiEdit_Value, 39, "%-38s", result);
            if (wifiEdit_CursorPos < 37) {
                wifiEdit_CursorPos++;
            }
            currentMenu->Display();
        }
        return true;
    }

    return false;
});

