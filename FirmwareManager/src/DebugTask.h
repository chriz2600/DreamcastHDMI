#ifndef DEBUG_TASK_H
#define DEBUG_TASK_H

#include <global.h>
#include <Task.h>
#include <Menu.h>

extern FPGATask fpgaTask;
extern TaskManager taskManager;

class DebugTask : public Task {

    public:
        DebugTask(uint8_t v) :
            Task(v)
        { };

    private:
        bool isRunning = false;

        virtual bool OnStart() {
            DBG_OUTPUT_PORT.printf("DebugTask: OnStart\n");
            isRunning = true;
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            fpgaTask.Read(DEBUG_BASE_ADDRESS, DEBUG_DATA_LEN, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                // leave, if task is no longer running
                if (!isRunning) {
                    DBG_OUTPUT_PORT.printf("DebugTask: in callback: no longer running\n");
                    return;
                }

                char result[MENU_INF_RESULT_HEIGHT * MENU_WIDTH] = "";

                if (address == DEBUG_BASE_ADDRESS && len == DEBUG_DATA_LEN) {
                    snprintf(result, MENU_INF_RESULT_HEIGHT * MENU_WIDTH,
                        "FRM:  %04d  RC/HR/NR: %03d/%03d/%03d       "
                        "PLLE:  %03d                              "
                        "CTS1:  %03d %03d %03d                      "
                        "CTS2:  %03d %03d %03d                      "
                        "CTS3:  %03d %03d %03d %03d                  "
                        "CID:   %03d %03d %03d                      "
                        "VIC:   %03d %03d                          "
                        "MISC:  %x",
                        (buffer[DBG_DATA_FRAMECOUNTER_HIGH] << 8) | buffer[DBG_DATA_FRAMECOUNTER_LOW], buffer[DBG_DATA_RESTART_COUNT], buffer[DBG_DATA_HDMI_INT_COUNT], buffer[DBG_DATA_NOT_READY_COUNT], 
                        buffer[DBG_DATA_PLL_ERRORS],
                        buffer[DBG_DATA_CTS1_STATUS], buffer[DBG_DATA_MAX_CTS1_STATUS], buffer[DBG_DATA_SUMMARY_CTS1_STATUS],
                        buffer[DBG_DATA_CTS2_STATUS], buffer[DBG_DATA_MAX_CTS2_STATUS], buffer[DBG_DATA_SUMMARY_CTS2_STATUS],
                        buffer[DBG_DATA_CTS3_STATUS], buffer[DBG_DATA_MAX_CTS3_STATUS], buffer[DBG_DATA_SUMMARY_CTS3_STATUS], buffer[DBG_DATA_SUMMARY_SUMMARY_CTS3_STATUS],
                        buffer[DBG_DATA_ID_CHECK_HIGH], buffer[DBG_DATA_ID_CHECK_LOW], buffer[DBG_DATA_CHIP_REVISION],
                        buffer[DBG_DATA_VIC_DETECTED] >> 2, buffer[DBG_DATA_VIC_TO_RX],
                        buffer[DBG_DATA_MISC_DATA]
                    );

                    fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_INF_RESULT_LINE, (uint8_t*) result);
                }
            });
        }

        virtual void OnStop() {
            DBG_OUTPUT_PORT.printf("DebugTask: OnStop\n");
            isRunning = false;
        }
};

#endif