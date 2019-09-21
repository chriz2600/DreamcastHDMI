#ifndef FLASH_ESP_TASK_H
#define FLASH_ESP_TASK_H

#include "../global.h"
#include <Task.h>

extern MD5Builder md5;
extern File flashFile;

extern int totalLength;
extern int readLength;
extern int last_error;
extern bool currentJobDone;

void _writeFile(const char *filename, const char *towrite, unsigned int len);
void _readFile(const char *filename, char *target, unsigned int len);

extern TaskManager taskManager;

#define BUFFER_SIZE 1024
#define DEBUG_UPDATER

class FlashESPTask : public Task {

    public:
        FlashESPTask(uint8_t v) :
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
        uint8_t buffer[BUFFER_SIZE];
        int prevPercentComplete;

        virtual bool OnStart() {
            currentJobDone = false;
            totalLength = -1;
            readLength = 0;
            prevPercentComplete = -1;
            last_error = NO_ERROR;

            md5.begin();
            flashFile = SPIFFS.open(ESP_FIRMWARE_FILE, "r");

            if (flashFile) {
                totalLength = flashFile.size();

                if (!Update.begin(totalLength, U_FLASH)) { //start with max available size
                    Update.printError(DBG_OUTPUT_PORT);
                    DEBUG("ERROR");
                    last_error = ERROR_FILE_SIZE;
                    InvokeCallback(false);
                    return false;
                }

                return true;
            } else {
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            int bytes_read = 0;

            if (flashFile.available()) {
                bytes_read = flashFile.readBytes((char *) buffer, BUFFER_SIZE);
                
                if (bytes_read == 0) {
                    taskManager.StopTask(this);
                } else {
                    md5.add(buffer, bytes_read);
                    if ((int) Update.write(buffer, bytes_read) != bytes_read) {
                        Update.printError(DBG_OUTPUT_PORT);
                        last_error = ERROR_ESP_FLASH;
                        taskManager.StopTask(this);
                    }
                }
            } else {
                taskManager.StopTask(this);
            }
            
            readLength += bytes_read;
            int percentComplete = (totalLength <= 0 ? 0 : (int)(readLength * 100 / totalLength));
            if (prevPercentComplete != percentComplete) {
                prevPercentComplete = percentComplete;
                InvokeCallback(false);
            }
        }

        virtual void OnStop() {
            flashFile.close();
            if (!Update.end()) {
                last_error = ERROR_ESP_FLASH_END;
            }
            if (last_error == NO_ERROR) {
                md5.calculate();
                String md5sum = md5.toString();
                _writeFile("/etc/last_esp_flash_md5", md5sum.c_str(), md5sum.length());
            }
            currentJobDone = true;
            InvokeCallback(true);
            DEBUG("2: flashing ESP finished.\n");
        }

        void InvokeCallback(bool done) {
            if (progressCallback != NULL) {
                progressCallback(readLength, totalLength, done, last_error);
            }
        }
};

#endif