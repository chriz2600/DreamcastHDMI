#include "../global.h"
#include "../Menu.h"

extern bool scanlinesActive;
extern int scanlinesIntensity;
extern bool scanlinesOddeven;
extern bool scanlinesThickness;

void readScanlinesActive();
void writeScanlinesActive();
void readScanlinesIntensity();
void writeScanlinesIntensity();
void readScanlinesOddeven();
void writeScanlinesOddeven();
void readScanlinesThickness();
void writeScanlinesThickness();
uint8_t getScanlinesUpperPart();
uint8_t getScanlinesLowerPart();
void setScanlines(uint8_t upper, uint8_t lower, WriteCallbackHandlerFunction handler);

Menu scanlinesMenu("ScanlinesMenu", OSD_SCANLINES_MENU, MENU_SL_FIRST_SELECT_LINE, MENU_SL_LAST_SELECT_LINE, [](uint16_t controller_data, uint8_t menu_activeLine, bool isRepeat) {
    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_CANCEL)) {
        // restoreScanlines
        readScanlinesActive();
        readScanlinesIntensity();
        readScanlinesThickness();
        readScanlinesOddeven();
        setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
            currentMenu = &mainMenu;
            currentMenu->Display();
        });
        return;
    }

    if (!isRepeat && CHECK_CTRLR_MASK(controller_data, MENU_OK)) {
        // restoreScanlines
        writeScanlinesActive();
        writeScanlinesIntensity();
        writeScanlinesThickness();
        writeScanlinesOddeven();
        currentMenu = &mainMenu;
        currentMenu->Display();
        return;
    }

    bool isLeft = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_LEFT);
    bool isRight = CHECK_CTRLR_MASK(controller_data, CTRLR_PAD_RIGHT);

    if (isLeft || isRight) {
        switch (menu_activeLine) {
            case MENU_SL_ACTIVE:
                scanlinesActive = !scanlinesActive;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED));
                    fpgaTask.DoWriteToOSD(MENU_SL_COLUMN, MENU_OFFSET + MENU_SL_ACTIVE, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_THICKNESS:
                scanlinesThickness = !scanlinesThickness;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN));
                    fpgaTask.DoWriteToOSD(MENU_SL_COLUMN, MENU_OFFSET + MENU_SL_THICKNESS, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_ODDEVEN:
                scanlinesOddeven = !scanlinesOddeven;
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6s", (scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN));
                    fpgaTask.DoWriteToOSD(MENU_SL_COLUMN, MENU_OFFSET + MENU_SL_ODDEVEN, (uint8_t*) buffer);
                });
                break;
            case MENU_SL_INTENSITY:
                if (isLeft) {
                    if (scanlinesIntensity > 0) {
                        scanlinesIntensity -= 1;
                    }
                } else if (isRight) {
                    if (scanlinesIntensity < 256) {
                        scanlinesIntensity += 1;
                    }
                }
                setScanlines(getScanlinesUpperPart(), getScanlinesLowerPart(), [](uint8_t Address, uint8_t Value) {
                    char buffer[MENU_WIDTH] = "";
                    snprintf(buffer, 7, "%6d", scanlinesIntensity);
                    fpgaTask.DoWriteToOSD(MENU_SL_COLUMN, MENU_OFFSET + MENU_SL_INTENSITY, (uint8_t*) buffer);
                });
                break;
        }
    }
}, [](uint8_t* menu_text, uint8_t menu_activeLine) {
    // write current values to menu
    char buffer[MENU_WIDTH] = "";

    snprintf(buffer, 7, "%6s", (scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED));
    memcpy(&menu_text[MENU_SL_ACTIVE * MENU_WIDTH + MENU_SL_COLUMN], buffer, 6);
    snprintf(buffer, 7, "%6d", scanlinesIntensity);
    memcpy(&menu_text[MENU_SL_INTENSITY * MENU_WIDTH + MENU_SL_COLUMN], buffer, 6);
    snprintf(buffer, 7, "%6s", (scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN));
    memcpy(&menu_text[MENU_SL_THICKNESS * MENU_WIDTH + MENU_SL_COLUMN], buffer, 6);
    snprintf(buffer, 7, "%6s", (scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN));
    memcpy(&menu_text[MENU_SL_ODDEVEN * MENU_WIDTH + MENU_SL_COLUMN], buffer, 6);

    return MENU_SL_FIRST_SELECT_LINE;
}, NULL, true);

void setScanlines(uint8_t upper, uint8_t lower, WriteCallbackHandlerFunction handler) {
    fpgaTask.Write(I2C_SCANLINE_UPPER, upper, [ lower, handler ](uint8_t Address, uint8_t Value) {
        fpgaTask.Write(I2C_SCANLINE_LOWER, lower, handler);
    });
}
