#include "../global.h"
#include "../Menu.h"

extern char ssid[64];
extern char password[64];
extern Menu wifiEditMenu;

char wifiEdit_Name[MENU_WIDTH];
char wifiEdit_Value[MENU_WIDTH];
uint8_t wifiEdit_CursorPos = 0;
uint8_t wifiEdit_activeLine = NO_SELECT_LINE;

Menu wifiMenu("WiFiMenu", (uint8_t*) OSD_WIFI_MENU, MENU_WIFI_FIRST_SELECT_LINE, MENU_WIFI_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        currentMenu = &mainMenu;
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
    char buffer[MENU_WIDTH] = "";

    snprintf(buffer, 28, "%-27s", ssid);
    memcpy(&menu_text[MENU_WIFI_SSID_LINE * MENU_WIDTH + 12], buffer, 27);
    snprintf(buffer, 28, "%-27s", strlen(password) > 0 ? "<password-set>" : "<password-not-set>");
    memcpy(&menu_text[MENU_WIFI_PASSWORD_LINE * MENU_WIDTH + 12], buffer, 27);

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

Menu wifiEditMenu("WiFiEditMenu", (uint8_t*) OSD_WIFI_EDIT_MENU, NO_SELECT_LINE, NO_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
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
}, NULL, false);

