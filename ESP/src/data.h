#ifndef DATA_H
#define DATA_H

#include "global.h"
#include "util.h"

extern uint8_t ForceVGA;
extern char resetMode[16];
extern char deinterlaceMode480i[16];
extern char deinterlaceMode576i[16];
extern char protectedMode[8];

bool DelayVGA = false;

uint8_t cfgRst2Int(char* rstMode);
uint8_t cfgDeint2Int(char* dMode);
uint8_t cfgProtcd2Int(char* pMode);

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

void writeCurrentResetMode() {
    String cfgRst = RESET_MODE_STR_LED;

    if (CurrentResetMode == RESET_MODE_GDEMU) {
        cfgRst = RESET_MODE_STR_GDEMU;
    } else if (CurrentResetMode == RESET_MODE_USBGDROM) {
        cfgRst = RESET_MODE_STR_USBGDROM;
    } else if (CurrentResetMode == RESET_MODE_MODE) {
        cfgRst = RESET_MODE_STR_MODE;
    }

    _writeFile("/etc/reset/mode", cfgRst.c_str(), 16);
    snprintf(resetMode, 16, "%s", cfgRst.c_str());
}

void readCurrentDeinterlaceMode480i() {
    _readFile("/etc/deinterlace/mode/480i", deinterlaceMode480i, 16, DEFAULT_DEINTERLACE_MODE);
    CurrentDeinterlaceMode480i = cfgDeint2Int(deinterlaceMode480i);
}

void readCurrentDeinterlaceMode576i() {
    _readFile("/etc/deinterlace/mode/576i", deinterlaceMode576i, 16, DEFAULT_DEINTERLACE_MODE);
    CurrentDeinterlaceMode576i = cfgDeint2Int(deinterlaceMode576i);
}

void readCurrentProtectedMode(bool skipRead) {
    if (!skipRead) {
        _readFile("/etc/protected/mode", protectedMode, 8, DEFAULT_PROTECTED_MODE);
    }
    CurrentProtectedMode = cfgProtcd2Int(protectedMode);
}

void readCurrentProtectedMode() {
    readCurrentProtectedMode(false);
}

void writeCurrentDeinterlaceMode480i() {
    String cfgDeint = DEINTERLACE_MODE_STR_BOB;

    if (CurrentDeinterlaceMode480i == DEINTERLACE_MODE_PASSTHRU) {
        cfgDeint = DEINTERLACE_MODE_STR_PASSTHRU;
    }

    _writeFile("/etc/deinterlace/mode/480i", cfgDeint.c_str(), 16);
    snprintf(deinterlaceMode480i, 16, "%s", cfgDeint.c_str());
}

void writeCurrentDeinterlaceMode576i() {
    String cfgDeint = DEINTERLACE_MODE_STR_BOB;

    if (CurrentDeinterlaceMode576i == DEINTERLACE_MODE_PASSTHRU) {
        cfgDeint = DEINTERLACE_MODE_STR_PASSTHRU;
    }

    _writeFile("/etc/deinterlace/mode/576i", cfgDeint.c_str(), 16);
    snprintf(deinterlaceMode576i, 16, "%s", cfgDeint.c_str());
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
    } else if (cfgRst == RESET_MODE_STR_MODE) {
        return RESET_MODE_MODE;
    }
    // default is LED
    return RESET_MODE_LED;
}

uint8_t cfgProtcd2Int(char* pMode) {
    String cfgProtcd = String(pMode);

    if (cfgProtcd == PROTECTED_MODE_STR_ON) {
        return PROTECTED_MODE_ON;
    }
    // default is bob deinterlacing
    return PROTECTED_MODE_OFF;
}

uint8_t cfgDeint2Int(char* dMode) {
    String cfgDeint = String(dMode);

    if (cfgDeint == DEINTERLACE_MODE_STR_PASSTHRU) {
        return DEINTERLACE_MODE_PASSTHRU;
    }
    // default is bob deinterlacing
    return DEINTERLACE_MODE_BOB;
}

uint8_t cfgRes2Int(char* intResolution) {
    String cfgRes = String(intResolution);

    if (cfgRes == RESOLUTION_STR_960p) {
        return RESOLUTION_960p;
    } else if (cfgRes == RESOLUTION_STR_480p) {
        return RESOLUTION_480p;
    } else if (cfgRes == RESOLUTION_STR_VGA) {
        return RESOLUTION_VGA;
    }

    // default is 1080p
    return RESOLUTION_1080p;
}

void writeCurrentResolution() {
    String cfgRes = RESOLUTION_STR_1080p;
    uint8_t saveres = remapResolution(CurrentResolution);

    if (saveres == RESOLUTION_960p) {
        cfgRes = RESOLUTION_STR_960p;
    } else if (saveres == RESOLUTION_480p) {
        cfgRes = RESOLUTION_STR_480p;
    } else if (saveres == RESOLUTION_VGA) {
        cfgRes = RESOLUTION_STR_VGA;
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

void read240pOffset() {
    char buffer[32] = "";
    _readFile("/etc/240p/offset", buffer, 32, DEFAULT_240P_OFFSET);
    Offset240p = atoi(buffer);
    if (Offset240p < 0) {
        Offset240p = 0;
    } else if (Offset240p > 255) {
        Offset240p = 255;
    }
}

void readVGAOffset() {
    char buffer[32] = "";
    _readFile("/etc/vga/offset", buffer, 32, DEFAULT_VGA_OFFSET);
    OffsetVGA = atoi(buffer);
    if (OffsetVGA < -120) {
        OffsetVGA = -120;
    } else if (OffsetVGA > 120) {
        OffsetVGA = 120;
    }
}

void readUpscalingMode() {
    char buffer[32] = "";
    _readFile("/etc/upscaling/mode", buffer, 32, DEFAULT_UPSCALING_MODE);
    UpscalingMode = atoi(buffer);
}

void readColorExpansionMode() {
    char buffer[32] = "";
    _readFile("/etc/color/expansion", buffer, 32, DEFAULT_COLOR_EXPANSION_MODE);
    ColorExpansionMode = atoi(buffer);
}

void readGammaMode() {
    char buffer[32] = "";
    _readFile("/etc/gamma/mode", buffer, 32, DEFAULT_GAMMA_MODE);
    GammaMode = atoi(buffer);
}

void readColorSpace() {
    char buffer[32] = "";
    _readFile("/etc/color/space", buffer, 32, DEFAULT_COLOR_SPACE);
    ColorSpace = atoi(buffer);
}

void writeScanlinesIntensity() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", scanlinesIntensity);
    _writeFile("/etc/scanlines/intensity", buffer, 32);
}

void write240pOffset() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", Offset240p);
    _writeFile("/etc/240p/offset", buffer, 32);
}

void writeVGAOffset() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", OffsetVGA);
    _writeFile("/etc/vga/offset", buffer, 32);
}

void writeUpscalingMode() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", UpscalingMode);
    _writeFile("/etc/upscaling/mode", buffer, 32);
}

void writeColorExpansionMode() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", ColorExpansionMode);
    _writeFile("/etc/color/expansion", buffer, 32);
}

void writeGammaMode() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", GammaMode);
    _writeFile("/etc/gamma/mode", buffer, 32);
}

void writeColorSpace() {
    char buffer[32] = "";
    snprintf(buffer, 31, "%d", ColorSpace);
    _writeFile("/etc/color/space", buffer, 32);
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