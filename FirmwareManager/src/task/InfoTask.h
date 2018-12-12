#ifndef INFO_TASK_H
#define INFO_TASK_H

#include "../global.h"
#include <Task.h>

extern TaskManager taskManager;

class InfoTask : public Task {

    public:
        InfoTask(uint8_t v) :
            Task(v)
        { };

    private:
        bool isRunning = false;

        virtual bool OnStart() {
            DBG_OUTPUT_PORT.printf("InfoTask: OnStart\n");
            isRunning = true;
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            fpgaTask.Read(I2C_TESTDATA_BASE, I2C_TESTDATA_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                // leave, if task is no longer running
                if (!isRunning) {
                    DBG_OUTPUT_PORT.printf("DebugTask: in callback: no longer running\n");
                    return;
                }

                char result[MENU_INF_RESULT_HEIGHT * MENU_WIDTH] = "";

                if (address == I2C_TESTDATA_BASE && len == I2C_TESTDATA_LENGTH) {
                    /*
                        Test data
                        ---------
                        buffer[0]: 00 / pinok1[21:16]
                        buffer[1]: pinok1[15:11] / pinok2[10:8]
                        buffer[2]: pinok2[7:0]
                        buffer[3]: resolutionX[11:4]
                        buffer[4]: resolutionX[3:0] / resolutionY[11:8]
                        buffer[5]: resolutionY[7:0]
                    */
                    uint16_t pinok1 = (buffer[0] << 5) | (buffer[1] >> 3);
                    uint16_t pinok2 = ((buffer[1] & 0x7) << 8) | buffer[2];
                    uint16_t resolX = (buffer[3] << 4) | (buffer[4] >> 4);
                    uint16_t resolY = ((buffer[4] & 0xF) << 8) | buffer[5];
                    snprintf(result, MENU_INF_RESULT_HEIGHT * MENU_WIDTH,
                        "No signal should be X on VMU screen!    "
                        "Signal test:                            "
                        "  00 01 02 03 04 05 06 07 08 09 10 11   "
                        "   %c  %c  %c  %c  %c  %c  %c  %c  %c  %c  %c  %c   "
                        "                                        "
                        "Raw Input Resolution: %03ux%03u           "
                        "                                        "
                        "Raw data:                               "
                        "  %02x %02x %02x %02x %02x %02x %04x %04x           ",
                        checkPin(pinok1, pinok2, 0, 0),
                        checkPin(pinok1, pinok2, 0, 1),
                        checkPin(pinok1, pinok2, 1, 2),
                        checkPin(pinok1, pinok2, 2, 3),
                        checkPin(pinok1, pinok2, 3, 4),
                        checkPin(pinok1, pinok2, 4, 5),
                        checkPin(pinok1, pinok2, 5, 6),
                        checkPin(pinok1, pinok2, 6, 7),
                        checkPin(pinok1, pinok2, 7, 8),
                        checkPin(pinok1, pinok2, 8, 9),
                        checkPin(pinok1, pinok2, 9, 10),
                        checkPin(pinok1, pinok2, 10, 10),
                        (resolX + 1) / 2, (resolY + 1),
                        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
                        pinok1, pinok2
                    );

                    fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_INF_RESULT_LINE, (uint8_t*) result);
                }
            });
        }

        virtual void OnStop() {
            DBG_OUTPUT_PORT.printf("InfoTask: OnStop\n");
            isRunning = false;
        }

        char checkPin(uint16_t pinok1, uint16_t pinok2, uint8_t pos1, uint8_t pos2) {
            uint8_t isOk = (
                   CHECK_BIT(pinok1, (1 << pos1))
                && CHECK_BIT(pinok2, (1 << pos1))
                && CHECK_BIT(pinok1, (1 << pos2))
                && CHECK_BIT(pinok2, (1 << pos2))
            );

            return (isOk ? 0x03 : 'X');
        }
};

#endif
