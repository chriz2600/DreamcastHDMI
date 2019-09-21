#ifndef UTIL_H
#define UTIL_H

#include "global.h"
#include <FS.h>

extern bool OSDOpen;

bool fpgaDisabled = false;

void enableFPGA();
void resetFPGAConfiguration();

void resetall() {
    DEBUG2("resetall...\n");
    taskManager.StopTask(&fpgaTask);
    fpgaTask.Write(I2C_ACTIVATE_HDMI, 0, NULL); fpgaTask.ForceLoop();
    enableFPGA();
    resetFPGAConfiguration();
    //ESP.eraseConfig();
    MDNS.end();
    ESP.restart();
}

void _writeFile(const char *filename, const char *towrite, unsigned int len) {
    File f = SPIFFS.open(filename, "w");
    if (f) {
        f.write((const uint8_t*) towrite, len);
        f.close();
        DEBUG2(">> _writeFile: %s:[%s]\n", filename, towrite);
    }
}

void _readFile(const char *filename, char *target, unsigned int len, const char* defaultValue) {
    bool exists = SPIFFS.exists(filename);
    bool readFromFile = false;
    if (exists) {
        File f = SPIFFS.open(filename, "r");
        if (f) {
            f.readBytes(target, len);
            f.close();
            DEBUG2(">> _readFile: %s:[%s]\n", filename, target);
            readFromFile = true;
        }
    }
    if (!readFromFile) {
        snprintf(target, len, "%s", defaultValue);
        DEBUG2(">> _readFile: %s:[%s] (default)\n", filename, target);
    }
}

int writeProgress(uint8_t *buffer, size_t maxLen, int progress) {
    char msg[5];
    uint len = 4;
    int alen = (len > maxLen ? maxLen : len);

    sprintf(msg, "% 3i\n", progress);
    //len = strlen(msg);
    memcpy(buffer, msg, alen);
    return alen;
}

bool forceI2CWrite(uint8_t addr1, uint8_t val1, uint8_t addr2, uint8_t val2) {
    int retryCount = 5000;
    int retries = 0;
    bool success = false;

    while (retryCount >= 0) {
        retries++;
        fpgaTask.Write(addr1, val1, NULL); fpgaTask.ForceLoop();
        if (last_error == NO_ERROR) { // only try second command, if first was successful
            DEBUG2("   success 1st command: %02x %02x (%i)\n", addr1, val1, retries);
            fpgaTask.Write(addr2, val2, NULL); fpgaTask.ForceLoop();
        }
        retryCount--;
        if (last_error == NO_ERROR) {
            DEBUG2("   success 2nd command: %02x %02x (%i)\n", addr2, val2, retries);
            success = true;
            break;
        }
        delayMicroseconds(500);
        yield();
    }
    DEBUG2("   retry loops needed: %i\n", retries);
    return success;
}

void waitForI2CRecover(bool waitForError) {
    int retryCount = I2C_RECOVER_TRIES;
    int prev_last_error = NO_ERROR;
    DEBUG2("... PRE: prev_last_error/last_error %i (%u/%u)\n", retryCount, prev_last_error, last_error);
    while (retryCount >= 0) {
        fpgaTask.Read(I2C_PING, 1, NULL); 
        fpgaTask.ForceLoop();
        if (waitForError) {
            if (prev_last_error != NO_ERROR && last_error == NO_ERROR) break;
        } else {
            if (last_error == NO_ERROR) break;
        }
        prev_last_error = last_error;
        retryCount--;
        delayMicroseconds(I2C_RECOVER_RETRY_INTERVAL_US);
        yield();
    }
    DEBUG2("... POST: prev_last_error/last_error %i (%u/%u)\n", retryCount, prev_last_error, last_error);
}

void disableFPGA() {
    pinMode(NCE, OUTPUT);
    digitalWrite(NCE, HIGH);
    fpgaDisabled = true;
}

void enableFPGA() {
    if (fpgaDisabled) {
        digitalWrite(NCE, LOW);
        pinMode(NCE, INPUT);
        fpgaDisabled = false;
    }
}

void startFPGAConfiguration() {
    pinMode(NCONFIG, OUTPUT);
    digitalWrite(NCONFIG, LOW);
}

void endFPGAConfiguration() {
    digitalWrite(NCONFIG, HIGH);
    pinMode(NCONFIG, INPUT);    
}

void resetFPGAConfiguration() {
    startFPGAConfiguration();
    delay(1);
    endFPGAConfiguration();
}

bool isValidV2FPGAFirmwareBundle() {
    bool isValid = false;
    uint8_t header[16];
    File file = SPIFFS.open(FIRMWARE_FILE, "r");
    if (file) {
        file.readBytes((char *) header, 16);
        /* new fpga firmware bundle must be v2 
           and must contain at least 2 files */
        if (header[4] >= 0x02 && header[12] >= 2) {
            isValid = true;
        }
        file.close();
    }
    return isValid;
}

#endif