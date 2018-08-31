#ifndef FLASH_TASK_H
#define FLASH_TASK_H

#include <global.h>
#include <Task.h>
#include <fastlz.h>

extern MD5Builder md5;
extern File flashFile;
extern SPIFlash flash;

extern int totalLength;
extern int readLength;
extern int last_error;

void reverseBitOrder(uint8_t *buffer);
void _writeFile(const char *filename, const char *towrite, unsigned int len);
void _readFile(const char *filename, char *target, unsigned int len);

extern TaskManager taskManager;

class FlashTask : public Task {

    public:
        FlashTask(uint8_t v) :
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

        uint8_t *buffer = NULL;
        uint8_t *result = NULL;
        uint8_t *result_start = NULL;
        uint8_t header[16];
        size_t bytes_in_result = 0;
        size_t block_size = 1536;
        int chunk_size = 0;
        unsigned int page;
        int prevPercentComplete;

        virtual bool OnStart() {
            page = 0;
            totalLength = -1;
            readLength = 0;
            prevPercentComplete = -1;
            last_error = NO_ERROR;
            // local
            chunk_size = 0;
            bytes_in_result = 0;

            md5.begin();
            flashFile = SPIFFS.open(FIRMWARE_FILE, "r");

            if (flashFile) {
                flashFile.readBytes((char *) header, 16);

                // check "magic"
                if (header[0] != 'D' || header[1] != 'C' || header[2] != 0x07 || header[3] != 0x04) {
                    last_error = ERROR_WRONG_MAGIC;
                    // TODO: report header bytes via DEBUG
                    InvokeCallback(false);
                    return false;
                }

                // check version
                if (header[4] != 0x01 || header[5] != 0x00) {
                    last_error = ERROR_WRONG_VERSION;
                    InvokeCallback(false);
                    return false;
                }

                // read block size
                block_size = header[6] + (header[7] << 8);

                // read file size and convert it to flash pages by dividing by 256
                totalLength = (header[8] + (header[9] << 8) + (header[10] << 16) + (header[11] << 24)) / 256;

                md5.add(header, 16);

                buffer = (uint8_t *) malloc(block_size);
                result = (uint8_t *) malloc(block_size);
                result_start = result;
                // initialze complete buffer with flash "default"
                initBuffer(result, block_size);

                // erase chip
                flash.enable();
                flash.chip_erase_async();

                return true;
            } else {
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (!flash.is_busy_async()) {
                if (page >= PAGES || doFlash() == -1) {
                    taskManager.StopTask(this);
                }
            }
            readLength = page;
            int percentComplete = (totalLength <= 0 ? 0 : (int)(readLength * 100 / totalLength));
            if (prevPercentComplete != percentComplete) {
                prevPercentComplete = percentComplete;
                InvokeCallback(false);
            }
        }

        void InvokeCallback(bool done) {
            if (progressCallback != NULL) {
                progressCallback(readLength, totalLength, done, last_error);
            }
        }

        void initBuffer(uint8_t *buffer, int len) {
            for (int i = 0 ; i < len ; i++) {
                // flash default (unwritten) is 0xff
                buffer[i] = 0xff; 
            }
        }

        int doFlash() {
            int bytes_write = 0;

            if (chunk_size > 0) {
                // step 3: decompress
                bytes_in_result = fastlz_decompress(buffer, chunk_size, result, block_size);
                chunk_size = 0;
                return 0; /* no bytes written yet */
            }

            if (bytes_in_result == 0) {
                // reset the result buffer pointer
                result = result_start;
                // step 1: read chunk header
                if (flashFile.readBytes((char *) buffer, 2) == 0) {
                    // no more chunks
                    return -1;
                }
                md5.add(buffer, 2);
                chunk_size = buffer[0] + (buffer[1] << 8);
                // step 2: read chunk length from file
                flashFile.readBytes((char *) buffer, chunk_size);
                md5.add(buffer, chunk_size);
                return 0; /* no bytes written yet */
            }

            // step 4: write data to flash, in 256 byte long pages
            bytes_write = bytes_in_result > 256 ? 256 : bytes_in_result;
            /* 
                even it's called async, it actually writes 256 bytes over SPI bus, it's just not calling the blocking "busy wait"
            */
            flash.page_write_async(page, result);
            /* 
                cleanup last 256 byte area afterwards, as spi flash memory is always written in chunks of 256 byte
            */
            initBuffer(result, 256);
            bytes_in_result -= bytes_write;
            result += bytes_write; /* advance the char* by written bytes */
            page++;
            return bytes_write; /* 256 bytes written */
        }

        virtual void OnStop() {
            flash.disable();
            flashFile.close();
            md5.calculate();
            String md5sum = md5.toString();
            _writeFile("/etc/last_flash_md5", md5sum.c_str(), md5sum.length());
            if (buffer != NULL)
                free(buffer);
            // make sure pointer is reset to original start
            result = result_start;
            if (result != NULL)
                free(result);
            buffer = NULL;
            result = NULL;
            result_start = NULL;
            InvokeCallback(true);
        }
};

#endif