#ifndef FS_MIGRATE_TASK_H
#define FS_MIGRATE_TASK_H

#include "../global.h"
#include "../config.h"
#include <Task.h>

extern int totalLength;
extern int readLength;
extern int last_error;
extern bool currentJobDone;
extern char fs_impl_name[16];
extern AsyncWebServer server;
extern AsyncStaticWebHandler* handler;
extern char httpAuthUser[64];
extern char httpAuthPass[64];

void _writeFile(const char *filename, const char *towrite, unsigned int len);
void _readFile(const char *filename, char *target, unsigned int len);

extern TaskManager taskManager;

class FSMigrateTask : public Task {

    public:
        FSMigrateTask(uint8_t v) :
            Task(1),
            dummy(v),
            progressCallback(NULL)
        { };

        void SetProgressCallback(ProgressCallback callback) {
            progressCallback = callback;
        }

        void ClearProgressCallback() {
            progressCallback = NULL;
        }

    private:
        uint8_t dummy;
        ProgressCallback progressCallback;
        int prevPercentComplete;
        Config config;

        virtual bool OnStart() {
            currentJobDone = false;
            totalLength = -1;
            readLength = 0;
            prevPercentComplete = -1;
            last_error = NO_ERROR;

            if (strcmp(fs_impl_name, "LittleFS") == 0) {
                return false;
            }

            totalLength = 5;
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            switch (readLength) {
                case 0:
                    readFullConfig(config);
                    DEBUG2("Config read\n");
                    readLength++;
                    break;
                case 1:
                    SPIFFS.end();
                    DEBUG2("SPIFFS unmounted\n");
                    readLength++;
                    break;
                case 2:
                    if (LittleFS.format()) {
                        DEBUG2("LittleFS formatted\n");
                        readLength++;
                    } else {
                        last_error = 1000;
                        taskManager.StopTask(this);
                    }
                    break;
                case 3:
                    if (LittleFS.begin()) {
                        DEBUG2("LittleFS mounted\n");
                        snprintf(fs_impl_name, 12, "LittleFS");
                        filesystem = &LittleFS;
                        readLength++;
                    } else {
                        last_error = 1001;
                        taskManager.StopTask(this);
                    }
                    break;
                case 4:
                    saveFullConfig(config);
                    DEBUG2("Config saved!\n");
                    readLength++;
                    break;
                case 5:
                    server.removeHandler(handler);
                    handler = &server.serveStatic("/", *filesystem, "/").setDefaultFile("index.html");
                    handler->setAuthentication(httpAuthUser, httpAuthPass);
                    DEBUG2("Done reconfiguring web server!\n");
                    taskManager.StopTask(this);
                    break;
            }

            int percentComplete = (totalLength <= 0 ? 0 : (int)(readLength * 100 / totalLength));
            if (prevPercentComplete != percentComplete) {
                prevPercentComplete = percentComplete;
                InvokeCallback(false);
            }
        }

        virtual void OnStop() {
            currentJobDone = true;
            InvokeCallback(true);
            DEBUG2("2: migrate FS finished.\n");
        }

        void InvokeCallback(bool done) {
            if (progressCallback != NULL) {
                progressCallback(readLength, totalLength, done, last_error);
            }
        }
};

#endif