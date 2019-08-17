#ifndef FLASH_VERIFY_TASK_H
#define FLASH_VERIFY_TASK_H

#include "../global.h"
#include <Task.h>

void _readFile(const char *filename, char *target, unsigned int len, const char* defaultValue);

typedef std::function<void(bool isOk)> FlashVerifyCallback;

extern TaskManager taskManager;

class FlashVerifyTask : public Task {

    public:
        FlashVerifyTask(uint8_t v) :
            Task(1),
            dummy(v),
            flashVerifyCallback(NULL)
        { };

        void Set(const char* _vfile, const char* _cfile, FlashVerifyCallback callback) {
            strcpy(vFile, _vfile);
            strcpy(cFile, _cfile);
            flashVerifyCallback = callback;
        }

    private:
        uint8_t dummy;
        FlashVerifyCallback flashVerifyCallback;
        unsigned int page;
        char storedMD5Sum[33];
        unsigned int pages;
        MD5Builder verifyMD5;
        uint8_t buffer[1024];
        File testFile;
        char vFile[256];
        char cFile[256];

        virtual bool OnStart() {
            if (strlen(vFile) == 0 || strlen(cFile) == 0) {
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }

            testFile = SPIFFS.open(vFile, "r");
            if (!testFile) {
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }
            _readFile(cFile, storedMD5Sum, 33, DEFAULT_MD5_SUM);
            verifyMD5.begin();
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            int bytesRead = testFile.readBytes((char *) buffer, 1024);
            if (!bytesRead) {
                taskManager.StopTask(this);
            } else {
                verifyMD5.add(buffer, bytesRead);
            }
        }

        void InvokeCallback(bool isOk) {
            if (flashVerifyCallback != NULL) {
                flashVerifyCallback(isOk);
            }
        }

        virtual void OnStop() {
            verifyMD5.calculate();
            String actualMD5 = verifyMD5.toString();
            DEBUG2("FlashVerifyTask: comparing MD5 stored/actual:\n [%s]\n [%s]\n", storedMD5Sum, actualMD5.c_str());
            InvokeCallback(strncmp(storedMD5Sum, actualMD5.c_str(), 32) == 0);
        }
};

#endif