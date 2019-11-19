#ifndef FLASH_TASK_H
#define FLASH_TASK_H

#include "../global.h"
#include <Task.h>
#include <fastlz.h>

extern MD5Builder md5;
extern File flashFile;
extern SPIFlash flash;

extern int totalLength;
extern int readLength;
extern int last_error;
extern bool currentJobDone;
extern char firmwareVariant[64];

void _writeFile(const char *filename, const char *towrite, unsigned int len);

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

        ////

        bool doStart() {
            return OnStart();
        }

        bool doUpdate() {
            OnUpdate(0l);
            return finished;
        }

        void doStop() {
            OnStop();
        }

    private:
        uint8_t dummy;
        ProgressCallback progressCallback;

        uint8_t *chunk_buffer = NULL;
        uint8_t *result = NULL;
        uint8_t *result_start = NULL;
        uint8_t header[16];
        uint8_t footer[4];
        uint8_t chunk_header[2];
        size_t bytes_in_result = 0;
        size_t block_size = 1536;
        int chunk_size = 0;
        unsigned int page;
        int prevPercentComplete;
        MD5Builder spiMD5;
        bool finished = false;
        bool headConsumed = false;
        bool headerParsed = false;
        bool tailConsumed = false;
        uint8_t consume_buffer[256];
        size_t totalBytesConsumed;
        unsigned int fwPosInFile = 0;
        int totalLengthBytes;
        int bytesReadFromArchive;

        virtual bool OnStart() {
            currentJobDone = false;
            page = 0;
            totalLength = -1;
            readLength = 0;
            prevPercentComplete = -1;
            last_error = NO_ERROR;
            // local
            chunk_size = 0;
            bytes_in_result = 0;
            finished = false;
            headConsumed = false;
            headerParsed = false;
            tailConsumed = false;
            totalBytesConsumed = 0;
            fwPosInFile = 0;
            totalLengthBytes = -1;
            bytesReadFromArchive = 0;

            md5.begin();
            spiMD5.begin();
            flashFile = SPIFFS.open(FIRMWARE_FILE, "r");

            if (flashFile) {
                // pre-parse header to get bundle information
                flashFile.readBytes((char *) header, 16);

                // check "magic"
                if (header[0] != 'D' || header[1] != 'C' || header[2] != 0x07 || header[3] != 0x04) {
                    last_error = ERROR_WRONG_MAGIC;
                    InvokeCallback(false);
                    return false;
                }

                // check version
                if (header[4] > 0x02 || header[5] != 0x00) {
                    last_error = ERROR_WRONG_VERSION;
                    InvokeCallback(false);
                    return false;
                }

                DEBUG2("FlashTask.OnStart: firmware file version: %u\n", header[4]);
                /* 
                    handle bundles, don't check version just check for files in file, 
                    to be able to handle transitional archives.
                */
                if (header[4] >= 2 && String(firmwareVariant) == String(FIRMWARE_RELAXED_FLAVOUR)) {
                    uint8_t file_to_extract = 1;
                    /*
                        use second archive in bundle, if length allows,
                        then read position of archive in bundle,
                        otherwise, just flash the first archive
                    */
                    DEBUG2("FlashTask.OnStart: firmware archives in bundle: %u\n", header[12]);
                    if (header[12] >= 2) {
                        flashFile.seek(((header[12] - file_to_extract) * 4), SeekEnd);
                        flashFile.readBytes((char*) footer, 4);
                        fwPosInFile = footer[0] + (footer[1] << 8) + (footer[2] << 16) + (footer[3] << 24);
                    }
                }

                // rewind to start
                flashFile.seek(0, SeekSet);
                return true;
            } else {
                last_error = ERROR_FILE;
                InvokeCallback(false);
                return false;
            }
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            /* 
                Read up to position, to add to md5sum (consumeHead),
                then flash and add to mdsum (doFlash) up to pages length,
                after that read to end and add to md5sum (consumeTail)
            */
            if (!flash.is_busy_async()) {
                if (!headConsumed) {
                    consumeHead();
                } else {
                    if (!headerParsed) {
                        parseHeader();
                    } else if (page >= (unsigned int) totalLength || doFlash() == -1) {
                        if (!tailConsumed) {
                            consumeTail();
                        } else {
                            finished = true;
                            taskManager.StopTask(this);
                        }
                    }
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

        void parseHeader() {
            flashFile.readBytes((char *) header, 16);
            // read block size
            block_size = header[6] + (header[7] << 8);

            // read file size and convert it to flash pages by dividing by 256
            totalLengthBytes = header[8] + (header[9] << 8) + (header[10] << 16) + (header[11] << 24);
            totalLength = (totalLengthBytes + 255) / 256;

            md5.add(header, 16);

            result = (uint8_t *) malloc(block_size);
            result_start = result;
            // initialze complete buffer with flash "default"
            initBuffer(result, block_size);

            // erase chip
            flash.enable();
            flash.chip_erase_async();
            headerParsed = true;

            DEBUG2("FlashTask.parseHeader: totalLengthBytes/totalLength(pages)/fwPosInFile(byte): %d/%d/%d\n", totalLengthBytes, totalLength, fwPosInFile);
        }

        void consumeHead() {
            int bytesRead = 0;
            int hmdwr = 256;

            hmdwr = totalBytesConsumed + 256 < fwPosInFile ? 256 : fwPosInFile - totalBytesConsumed;
            bytesRead = flashFile.readBytes((char*) consume_buffer, hmdwr);
            if (bytesRead <= 0) {
                headConsumed = true;
            } else {
                md5.add(consume_buffer, bytesRead);
            }
            totalBytesConsumed += bytesRead;
        }

        void consumeTail() {
            int bytesRead = flashFile.readBytes((char*) consume_buffer, 256);
            if (bytesRead <= 0) {
                tailConsumed = true;
            } else {
                md5.add(consume_buffer, bytesRead);
            }
        }

        int doFlash() {
            int bytes_write = 0;

            if (chunk_size > 0) {
                // step 3: decompress
                bytes_in_result = fastlz_decompress(chunk_buffer, chunk_size, result, block_size);
                chunk_size = 0;
                bytesReadFromArchive += bytes_in_result;
                return 0; /* no bytes written yet */
            }

            if (bytes_in_result == 0) {
                // reset the result buffer pointer
                result = result_start;
                // step 1: have we read all bytes in this archive
                if (bytesReadFromArchive >= totalLengthBytes) {
                    // no more chunks
                    return -1;
                }
                // step 2: read chunk header
                if (flashFile.readBytes((char *) chunk_header, 2) == 0) {
                    // no more chunks
                    return -1;
                }
                md5.add(chunk_header, 2);
                chunk_size = chunk_header[0] + (chunk_header[1] << 8);
                // step 3: read chunk length from file
                if (chunk_buffer != NULL) {
                    free(chunk_buffer);
                }
                chunk_buffer = (uint8_t *) malloc(chunk_size * sizeof(uint8_t));
                flashFile.readBytes((char *) chunk_buffer, chunk_size);
                md5.add(chunk_buffer, chunk_size);
                return 0; /* no bytes written yet */
            }

            // step 4: write data to flash, in 256 byte long pages
            bytes_write = bytes_in_result > 256 ? 256 : bytes_in_result;
            /*
                even it's called async, it actually writes 256 bytes over SPI bus, it's just not calling the blocking "busy wait"
            */
            flash.page_write_async(page, result);
            spiMD5.add(result, 256);
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
            DEBUG2("FlashTask.OnStop: wrote %u pages.\n", page);
            flash.disable();
            flashFile.close();
            // store md5 sum of last flashed firmware file
            md5.calculate();
            String md5sum = md5.toString();
            _writeFile("/etc/last_flash_md5", md5sum.c_str(), md5sum.length());
            // store md5 sum and page count of actual data written to flash, for check later
            spiMD5.calculate();
            String spiMD5sum = spiMD5.toString();
            _writeFile("/etc/last_flash_spi_md5", spiMD5sum.c_str(), spiMD5sum.length());
            _writeFile("/etc/last_flash_spi_pages", (String(totalLength)).c_str(), 8);
            // cleanup
            if (chunk_buffer != NULL)
                free(chunk_buffer);
            // make sure pointer is reset to original start
            result = result_start;
            if (result != NULL)
                free(result);
            chunk_buffer = NULL;
            result = NULL;
            result_start = NULL;
            currentJobDone = true;
            InvokeCallback(true);
        }
};

#endif