#ifndef FLASH_ERASE_TASK_H
#define FLASH_ERASE_TASK_H

#include "../global.h"
#include <Task.h>

extern SPIFlash flash;
extern TaskManager taskManager;

class FlashEraseTask : public Task {

    public:
        FlashEraseTask(uint8_t v) :
            Task(1),
            dummy(v)
        { };

    private:
        uint8_t dummy;

        virtual bool OnStart() {
            currentJobDone = false;
            totalLength = -1;
            readLength = 0;
            last_error = NO_ERROR;

            DEBUG("erasing spi flash ... ");
            flash.enable();
            flash.chip_erase_async();
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (!flash.is_busy_async()) {
                taskManager.StopTask(this);
            }
        }

        virtual void OnStop() {
            flash.disable();
            totalLength = 1;
            readLength = 1;
            currentJobDone = true;
            last_error = NO_ERROR;
            DEBUG("done.\n");
        }
};

#endif