#ifndef FLASH_BLANK_CHECK_TASK_H
#define FLASH_BLANK_CHECK_TASK_H

#include "../global.h"
#include <Task.h>

extern SPIFlash flash;
extern TaskManager taskManager;

void _readFile(const char *filename, char *target, unsigned int len, const char* defaultValue);

typedef std::function<void(bool isOk)> FlashCheckCallback;

class FlashCheckTask : public Task {

    public:
        FlashCheckTask(uint8_t v, FlashCheckCallback callback) :
            Task(1),
            dummy(v),
            flashCheckCallback(callback)
        { };

    private:
        uint8_t dummy;
        FlashCheckCallback flashCheckCallback;
        unsigned int page;
        char storedMD5Sum[33];
        unsigned int pages;
        MD5Builder spiMD5;
        uint8_t buffer[256];

        virtual bool OnStart() {
            DEBUG("SPI flash check starting...\n");
            page = 0;
            _readFile("/etc/last_flash_spi_md5", storedMD5Sum, 33, DEFAULT_MD5_SUM);
            char _pages[8];
            _readFile("/etc/last_flash_spi_pages", _pages, 8, "0");
            pages = atoi(_pages);
            spiMD5.begin();
            flash.enable();
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (!flash.is_busy_async()) {
                if (page > pages || page >= PAGES) {
                    taskManager.StopTask(this);
                } else {
                    flash.page_read_async(page, buffer);
                    spiMD5.add(buffer, 256);
                    page++;
                }
            }
        }

        void InvokeCallback(bool isOk) {
            DEBUG("SPI flash check finished: %u\n", isOk);
            if (flashCheckCallback != NULL) {
                flashCheckCallback(isOk);
            }
        }

        virtual void OnStop() {
            flash.disable();
            spiMD5.calculate();
            String actualMD5 = spiMD5.toString();
            DEBUG("Pages: %u\nComparing MD5 stored/actual:\n  [%s]\n  [%s]\n", page, storedMD5Sum, actualMD5.c_str());
            InvokeCallback(strncmp(storedMD5Sum, actualMD5.c_str(), 32) == 0);
        }

        void reverseBitOrder(uint8_t *buffer, int length) {
            for (int i = 0 ; i < length ; i++) { 
                buffer[i] = (buffer[i] & 0xF0) >> 4 | (buffer[i] & 0x0F) << 4;
                buffer[i] = (buffer[i] & 0xCC) >> 2 | (buffer[i] & 0x33) << 2;
                buffer[i] = (buffer[i] & 0xAA) >> 1 | (buffer[i] & 0x55) << 1;
            }
        }
};

#endif