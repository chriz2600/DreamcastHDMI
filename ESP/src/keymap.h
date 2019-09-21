#ifndef KEYMAP_H
#define KEYMAP_H

#include <inttypes.h>
#include <functional>
#include "global.h"

#define KEYB_CTRL (1<<0)
#define KEYB_SHIFT (1<<1)
#define KEYB_SHIFT_R (1<<5)
#define KEYB_ALT (1<<2)
#define KEYB_ALT_GR (1<<6)

#define KEYB_KEY_ESCAPE 0x29
#define KEYB_KEY_RETURN 0x28
#define KEYB_KEY_TAB 0x2B
#define KEYB_KEY_BACKSPACE 0x2A
#define KEYB_KEY_INSERT 0x49
#define KEYB_KEY_DELETE 0x4C
#define KEYB_KEY_POS1 0x4A
#define KEYB_KEY_END 0x4D
#define KEYB_KEY_PAGEUP 0x4B
#define KEYB_KEY_PAGEDOWN 0x4E
#define KEYB_KEY_UP 0x52
#define KEYB_KEY_DOWN 0x51
#define KEYB_KEY_LEFT 0x50
#define KEYB_KEY_RIGHT 0x4f
#define KEYB_KEY_F11 0x44
#define KEYB_KEY_F12 0x45

extern char keyboardLayout[8];

uint8_t getASCIICodeDE(uint8_t shiftcode, uint8_t chardata) {
    bool shift = CHECK_BIT(shiftcode, KEYB_SHIFT) || CHECK_BIT(shiftcode, KEYB_SHIFT_R);
    //bool ctrl = CHECK_BIT(shiftcode, KEYB_CTRL);
    //bool alt = CHECK_BIT(shiftcode, KEYB_ALT);
    bool altgr = CHECK_BIT(shiftcode, KEYB_ALT_GR);

    // a-z
    if (chardata >= 0x04 && chardata <= 0x1d) {
        // @ sign
        if (altgr && chardata == 0x14) {
            return 0x40;
        }

        // swap y and z
        if (chardata == 0x1C) {
            chardata = 0x1D;
        } else if (chardata == 0x1D) {
            chardata = 0x1C;
        }

        return chardata + (shift ? 0x3D : 0x5D);
    }

    switch (chardata) {
        case 0x1e: // KBD_KEY_1
            return shift ? 0x21 : 0x31;
        case 0x1f: // KBD_KEY_2
            return shift ? 0x22 : 0x32;
        case 0x20: // KBD_KEY_3
            return shift ? 0x00 : 0x33;
        case 0x21: // KBD_KEY_4
            return shift ? 0x24 : 0x34;
        case 0x22: // KBD_KEY_5
            return shift ? 0x25 : 0x35;
        case 0x23: // KBD_KEY_6
            return shift ? 0x26 : 0x36;
        case 0x24: // KBD_KEY_7
            return shift ? 0x2F : altgr ? 0x7B : 0x37;
        case 0x25: // KBD_KEY_8
            return shift ? 0x28 : altgr ? 0x5B : 0x38;
        case 0x26: // KBD_KEY_9
            return shift ? 0x29 : altgr ? 0x5D : 0x39;
        case 0x27: // KBD_KEY_0
            return shift ? 0x3D : altgr ? 0x7D : 0x30;
        case 0x2c: // KBD_KEY_SPACE
            return 0x20;
        case 0x2d: // KBD_KEY_MINUS
            return shift ? 0x3F : altgr ? 0x5C : 0x00;
        case 0x2e: // KBD_KEY_PLUS
            return shift ? 0x60 : 0x00;
        case 0x30: // KBD_KEY_RBRACKET
            return shift ? 0x2A : altgr ? 0x7E : 0x2B;
        case 0x32: // ?????
            return shift ? 0x27 : 0x23;
        case 0x35: // KBD_KEY_TILDE
            return shift ? 0x00 : 0x5E;
        case 0x36: // KBD_KEY_COMMA
            return shift ? 0x3B : 0x2C;
        case 0x37: // KBD_KEY_PERIOD
            return shift ? 0x3A : 0x2E;
        case 0x38: // KBD_KEY_SLASH
            return shift ? 0x5F : 0x2D;
        case 0x64: // ??????????????
            return shift ? 0x3E : altgr ? 0x7C : 0x3C;
        default:
            return 0;
    }
}

uint8_t getASCIICodeUS(uint8_t shiftcode, uint8_t chardata) {
    bool shift = CHECK_BIT(shiftcode, KEYB_SHIFT) || CHECK_BIT(shiftcode, KEYB_SHIFT_R);

    // a-z
    if (chardata >= 0x04 && chardata <= 0x1d) {
        return chardata + (shift ? 0x3D : 0x5D);
    }

    switch (chardata) {
        case 0x1e: // KBD_KEY_1
            return shift ? 0x21 : 0x31;
        case 0x1f: // KBD_KEY_2
            return shift ? 0x40 : 0x32;
        case 0x20: // KBD_KEY_3
            return shift ? 0x23 : 0x33;
        case 0x21: // KBD_KEY_4
            return shift ? 0x24 : 0x34;
        case 0x22: // KBD_KEY_5
            return shift ? 0x25 : 0x35;
        case 0x23: // KBD_KEY_6
            return shift ? 0x5E : 0x36;
        case 0x24: // KBD_KEY_7
            return shift ? 0x26 : 0x37;
        case 0x25: // KBD_KEY_8
            return shift ? 0x2A : 0x38;
        case 0x26: // KBD_KEY_9
            return shift ? 0x28 : 0x39;
        case 0x27: // KBD_KEY_0
            return shift ? 0x29 : 0x30;
        case 0x2c: // KBD_KEY_SPACE
            return 0x20;
        case 0x2d: // KBD_KEY_MINUS
            return shift ? 0x5F : 0x2D;
        case 0x2e: // KBD_KEY_PLUS
            return shift ? 0x2B : 0x3D;
        case 0x2f: // KBD_KEY_LBRACKET
            return shift ? 0x7B : 0x5B;
        case 0x30: // KBD_KEY_RBRACKET
            return shift ? 0x7D : 0x5D;
        case 0x31: // KBD_KEY_BACKSLASH
            return shift ? 0x7C : 0x5C;
        case 0x33: // KBD_KEY_SEMIKOLON
            return shift ? 0x3A : 0x3B;
        case 0x34: // KBD_KEY_QUOTE
            return shift ? 0x22 : 0x27;
        case 0x35: // KBD_KEY_TILDE
            return shift ? 0x7E : 0x60;
        case 0x36: // KBD_KEY_COMMA
            return shift ? 0x3C : 0x2C;
        case 0x37: // KBD_KEY_PERIOD
            return shift ? 0x3E : 0x2E;
        case 0x38: // KBD_KEY_SLASH
            return shift ? 0x3F : 0x2F;
        default:
            return 0;
    }
}

uint8_t getASCIICode(uint8_t shiftcode, uint8_t chardata) {
    if (strcmp(keyboardLayout, DE) == 0) {
        return getASCIICodeDE(shiftcode, chardata);
    }

    return getASCIICodeUS(shiftcode, chardata);
}

#endif