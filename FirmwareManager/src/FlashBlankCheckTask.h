#ifndef FLASH_BLANK_CHECK_TASK_H
#define FLASH_BLANK_CHECK_TASK_H

#include <global.h>
#include <Task.h>

extern SPIFlash flash;
extern TaskManager taskManager;

typedef std::function<void(bool isBlank)> BlankCheckCallback;

class FlashBlankCheckTask : public Task {

    public:
        FlashBlankCheckTask(uint8_t v, BlankCheckCallback callback) :
            Task(1),
            dummy(v),
            blankCheckCallback(callback)
        { };

    private:
        uint8_t dummy;
        BlankCheckCallback blankCheckCallback;
        unsigned int page;
        bool isBlank;

        virtual bool OnStart() {
            DBG_OUTPUT_PORT.printf("Blank check starting...\n");
            page = 0;
            isBlank = true;
            flash.enable();
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (!flash.is_busy_async()) {
                if (page >= PAGES) {
                    taskManager.StopTask(this);
                } else {
                    DBG_OUTPUT_PORT.printf("checking page %u...", page);
                    uint8_t buffer[256];
                    flash.page_read_async(page, buffer);
                    isBlank = checkIfBlank(buffer, 256);
                    DBG_OUTPUT_PORT.printf(" isBlank: %u\n", isBlank);
                    if (!isBlank) {
                        taskManager.StopTask(this);
                    }
                    page++;
                }
            }
        }

        bool checkIfBlank(uint8_t *buffer, int len) {
            for (int i = 0 ; i < len ; i++) {
                if (buffer[i] != 0xff) {
                    return false;
                }
            }
            return true;
        }

        void InvokeCallback() {
            DBG_OUTPUT_PORT.printf("Blank check finished: %u\n", isBlank);
            if (blankCheckCallback != NULL) {
                blankCheckCallback(isBlank);
            }
        }

        virtual void OnStop() {
            flash.disable();
            InvokeCallback();
        }
};

#endif