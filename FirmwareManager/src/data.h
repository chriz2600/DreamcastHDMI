#ifndef DATA_H
#define DATA_H

#include "global.h"
#include "util.h"

extern uint8_t ForceVGA;
extern char resetMode[16];

bool DelayVGA = false;

uint8_t cfgRst2Int(char* rstMode);

void readVideoMode() {
    _readFile("/etc/video/mode", videoMode, 16, DEFAULT_VIDEO_MODE);
    String vidMode = String(videoMode);

    if (vidMode == VIDEO_MODE_STR_FORCE_VGA) {
        ForceVGA = VGA_ON;
        DelayVGA = false;
    } else if (vidMode == VIDEO_MODE_STR_SWITCH_TRICK) {
        ForceVGA = VGA_ON;
        DelayVGA = true;
    } else { // default: VIDEO_MODE_STR_CABLE_DETECT
        ForceVGA = VGA_OFF;
        DelayVGA = false;
    }
}

void writeVideoMode() {
    String vidMode = VIDEO_MODE_STR_CABLE_DETECT;

    if (ForceVGA == VGA_ON && !DelayVGA) {
        vidMode = VIDEO_MODE_STR_FORCE_VGA;
    } else if (ForceVGA == VGA_ON && DelayVGA) {
        vidMode = VIDEO_MODE_STR_SWITCH_TRICK;
    }

    writeVideoMode2(vidMode);
}

void writeVideoMode2(String vidMode) {
    _writeFile("/etc/video/mode", vidMode.c_str(), 16);
    snprintf(videoMode, 16, "%s", vidMode.c_str());
}

void readCurrentResetMode() {
    _readFile("/etc/reset/mode", resetMode, 16, DEFAULT_RESET_MODE);
    CurrentResetMode = cfgRst2Int(resetMode);
}

void readCurrentResolution() {
    _readFile("/etc/video/resolution", configuredResolution, 16, DEFAULT_VIDEO_RESOLUTION);
    CurrentResolution = cfgRes2Int(configuredResolution);
}

uint8_t cfgRst2Int(char* rstMode) {
    String cfgRst = String(rstMode);

    if (cfgRst == RESET_MODE_STR_GDEMU) {
        return RESET_MODE_GDEMU;
    } else if (cfgRst == RESET_MODE_STR_USBGDROM) {
        return RESET_MODE_USBGDROM;
    }
    // default is LED
    return RESET_MODE_LED;
}

void writeCurrentResetMode() {
    String cfgRst = RESET_MODE_STR_LED;

    if (CurrentResetMode == RESET_MODE_GDEMU) {
        cfgRst = RESET_MODE_STR_GDEMU;
    } else if (CurrentResetMode == RESET_MODE_USBGDROM) {
        cfgRst = RESET_MODE_STR_USBGDROM;
    }

    _writeFile("/etc/reset/mode", cfgRst.c_str(), 16);
    snprintf(resetMode, 16, "%s", cfgRst.c_str());
}

uint8_t cfgRes2Int(char* intResolution) {
    String cfgRes = String(intResolution);

    if (cfgRes == RESOLUTION_STR_960p) {
        return RESOLUTION_960p;
    } else if (cfgRes == RESOLUTION_STR_480p) {
        return RESOLUTION_480p;
    } else if (cfgRes == RESOLUTION_STR_VGA) {
        return RESOLUTION_VGA;
    } else if (cfgRes == RESOLUTION_STR_240Px3) {
        return RESOLUTION_240Px3;
    } else if (cfgRes == RESOLUTION_STR_240Px4) {
        return RESOLUTION_240Px4;
    } else if (cfgRes == RESOLUTION_STR_240P1080P) {
        return RESOLUTION_240P1080P;
    }
    // default is 1080p
    return RESOLUTION_1080p;
}

void writeCurrentResolution() {
    String cfgRes = RESOLUTION_STR_1080p;

    if (CurrentResolution == RESOLUTION_960p) {
        cfgRes = RESOLUTION_STR_960p;
    } else if (CurrentResolution == RESOLUTION_480p) {
        cfgRes = RESOLUTION_STR_480p;
    } else if (CurrentResolution == RESOLUTION_VGA) {
        cfgRes = RESOLUTION_STR_VGA;
    } else if (CurrentResolution == RESOLUTION_240Px3) {
        cfgRes = RESOLUTION_STR_240Px3;
    } else if (CurrentResolution == RESOLUTION_240Px4) {
        cfgRes = RESOLUTION_STR_240Px4;
    } else if (CurrentResolution == RESOLUTION_240P1080P) {
        cfgRes = RESOLUTION_STR_240P1080P;
    }

    _writeFile("/etc/video/resolution", cfgRes.c_str(), 16);
    snprintf(configuredResolution, 16, "%s", cfgRes.c_str());
}

/////////

void readScanlinesActive() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/active", buffer, 32, DEFAULT_SCANLINES_ACTIVE);
    if (strcmp(buffer, SCANLINES_ENABLED) == 0) {
        scanlinesActive = true;
        return;
    }
    scanlinesActive = false;
}

void writeScanlinesActive() {
    _writeFile("/etc/scanlines/active", scanlinesActive ? SCANLINES_ENABLED : SCANLINES_DISABLED, 32);
}

/////////

void readScanlinesIntensity() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/intensity", buffer, 32, DEFAULT_SCANLINES_INTENSITY);
    scanlinesIntensity = atoi(buffer);
    if (scanlinesIntensity < 0) {
        scanlinesIntensity = 0;
    } else if (scanlinesIntensity > 256) {
        scanlinesIntensity = 256;
    }
}

void writeScanlinesIntensity() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", scanlinesIntensity);
    _writeFile("/etc/scanlines/intensity", buffer, 32);
}

/////////

void readScanlinesOddeven() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/oddeven", buffer, 32, DEFAULT_SCANLINES_ODDEVEN);
    if (strcmp(buffer, SCANLINES_ODD) == 0) {
        scanlinesOddeven = true;
        return;
    }
    scanlinesOddeven = false;
}

void writeScanlinesOddeven() {
    _writeFile("/etc/scanlines/oddeven", scanlinesOddeven ? SCANLINES_ODD : SCANLINES_EVEN, 32);
}

/////////

void readScanlinesThickness() {
    char buffer[32] = "";
    _readFile("/etc/scanlines/thickness", buffer, 32, DEFAULT_SCANLINES_THICKNESS);
    if (strcmp(buffer, SCANLINES_THICK) == 0) {
        scanlinesThickness = true;
        return;
    }
    scanlinesThickness = false;
}

void writeScanlinesThickness() {
    _writeFile("/etc/scanlines/thickness", scanlinesThickness ? SCANLINES_THICK : SCANLINES_THIN, 32);
}

uint8_t getScanlinesUpperPart() {
    return (scanlinesIntensity >> 1);
}

uint8_t getScanlinesLowerPart() {
    return (scanlinesIntensity << 7) | (scanlinesThickness << 6) | (scanlinesOddeven << 5) | (scanlinesActive << 4);
}

#endif