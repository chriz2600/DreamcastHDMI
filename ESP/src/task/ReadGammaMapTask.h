#ifndef READ_GAMMA_MAP_TASK_H
#define READ_GAMMA_MAP_TASK_H

#include "../global.h"
#include <Task.h>

extern uint8_t GammaMode;

extern int totalLength;
extern int readLength;
extern int last_error;
extern bool currentJobDone;

void _writeFile(const char *filename, const char *towrite, unsigned int len);
void _readFile(const char *filename, char *target, unsigned int len);

extern TaskManager taskManager;

#define RGM_BUFFER_SIZE 32
#define RGM_BURST_SIZE 16
#define WRITE_SLOTS_NEEDED 3

class ReadGammaMapTask : public Task {

    public:
        ReadGammaMapTask(uint8_t v) :
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
        uint8_t buffer[RGM_BUFFER_SIZE];
        int prevPercentComplete;
        File sourceFile;

        virtual bool OnStart() {
            currentJobDone = false;
            totalLength = -1;
            readLength = 0;
            prevPercentComplete = -1;
            last_error = NO_ERROR;
            
            sourceFile = SPIFFS.open("/mapper.gdata", "r");

            if (sourceFile) {
                DEBUG2("Started loading /mapper.gdata\n");
                totalLength = sourceFile.size();
                if (totalLength % RGM_BURST_SIZE == 0) {
                    return true;
                } else {
                    DEBUG2("ERROR starting loading /mapper.gdata, invalid length\n");
                    last_error = ERROR_FILE;
                    InvokeCallback(false);
                    return false;
                }
            } else {
                DEBUG2("ERROR starting loading /mapper.gdata\n");
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            int bytes_read = 0;

            if (sourceFile.available()) {
                if (fpgaTask.AvailableWriteSlots(WRITE_SLOTS_NEEDED * RGM_BUFFER_SIZE)) {
                    bytes_read = sourceFile.readBytes((char *) buffer, RGM_BUFFER_SIZE);
                    
                    if (bytes_read == 0) {
                        taskManager.StopTask(this);
                    } else {
                        for (int i = 0 ; i < bytes_read ; i += RGM_BURST_SIZE) {
                            uint8_t writez[RGM_BURST_SIZE];
                            switch (readLength + i) {
                                case 0:
                                    DEBUG2("...loading red\n");
                                    fpgaTask.Write(0xD2, 0x10);
                                    break;
                                case 256:
                                    DEBUG2("...loading green\n");
                                    fpgaTask.Write(0xD2, 0x11);
                                    break;
                                case 512:
                                    DEBUG2("...loading blue\n");
                                    fpgaTask.Write(0xD2, 0x12);
                                    break;
                            }
                            fpgaTask.Write(0xD3, ((readLength + i) % 256));
                            for (int j = 0 ; j < RGM_BURST_SIZE ; j++) {
                                writez[j] = buffer[i+j];
                            }
                            fpgaTask.Write(0xD4, writez, RGM_BURST_SIZE, NULL);
                        }
                    }
                } else {
                    DEBUG("waiting for free write slots slots\n");
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
            sourceFile.close();
            if (last_error == NO_ERROR) {
                GammaMode = 0x1F;
                fpgaTask.Write(I2C_COLOR_EXPANSION_AND_GAMMA_MODE, fpgaTask.GetColorExpansion() | GammaMode << 3);
            }
            currentJobDone = true;
            InvokeCallback(true);
            DEBUG2("Finished applying custom gamma map.\n");
        }

        void InvokeCallback(bool done) {
            if (progressCallback != NULL) {
                progressCallback(readLength, totalLength, done, last_error);
            }
        }
};

#endif