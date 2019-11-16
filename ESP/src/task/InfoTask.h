#ifndef INFO_TASK_H
#define INFO_TASK_H

#include "../global.h"
#include <Task.h>

int getWiFiQuality(int dBm);

extern TaskManager taskManager;

class InfoTask : public Task {

    public:
        InfoTask(uint8_t v) :
            Task(v)
        { };

        void ResetCounters() {
            resetCounters = true;
        }

    private:
        bool isStopped = true;
        bool isRunning = false;
        bool resetCounters = false;

        uint32_t pll_adv_lockloss_offset = 0;
        uint32_t hpd_low_offset = 0;
        uint32_t pll54_lockloss_offset = 0;
        uint32_t pll_hdmi_lockloss_offset = 0;
        uint32_t control_resync_out_offset = 0;
        uint32_t monitor_sense_low_offset = 0;

        virtual bool OnStart() {
            DEBUG("InfoTask: OnStart\n");
            isRunning = true;
            isStopped = false;
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            if (!isRunning || isStopped) {
                DEBUG("DebugTask: paused or stopped\n");
                return;
            }

            isRunning = false;
            fpgaTask.Read(I2C_TESTDATA_BASE, I2C_TESTDATA_LENGTH, [&](uint8_t address, uint8_t* buffer, uint8_t len) {
                char result[(MENU_INF_RESULT_HEIGHT * MENU_WIDTH) + 1] = "";

                if (address == I2C_TESTDATA_BASE && len == I2C_TESTDATA_LENGTH) {
                    /*
                        Test data
                        ---------
                        buffer[0]: \
                        buffer[1]:  |__ pll_adv_lockloss_count
                        buffer[2]:  |   MSB first
                        buffer[3]: /

                        buffer[4]: \
                        buffer[5]:  |__ hpd_low_count
                        buffer[6]:  |   MSB first
                        buffer[7]: /

                        buffer[8]:  \
                        buffer[9]:   |__ pll54_lockloss_count
                        buffer[10]:  |   MSB first
                        buffer[11]: /

                        buffer[12]: \
                        buffer[13]:  |__ pll_hdmi_lockloss_count
                        buffer[14]:  |   MSB first
                        buffer[15]: /

                        buffer[16]: 00 / pinok1[21:16]
                        buffer[17]: pinok1[15:11] / pinok2[10:8]
                        buffer[18]: pinok2[7:0]
                        buffer[19]: resolutionX[11:4]
                        buffer[20]: resolutionX[3:0] / resolutionY[11:8]
                        buffer[21]: resolutionY[7:0]
                        buffer[22]: rgbData (currently not used)
                        buffer[23]: rgbData (currently not used)
                        buffer[24]: rgbData (currently not used)

                        buffer[25]: \
                        buffer[26]:  |__ control_resync_out_count
                        buffer[27]:  |   MSB first
                        buffer[28]: /

                        buffer[29]: \
                        buffer[30]:  |__ monitor_sense_low_count
                        buffer[31]:  |   MSB first
                        buffer[32]: /
                    */
                    uint32_t pll_adv_lockloss_count = (buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]);
                    uint32_t hpd_low_count = (buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7]);
                    uint32_t pll54_lockloss_count = (buffer[8] << 24 | buffer[9] << 16 | buffer[10] << 8 | buffer[11]);
                    uint32_t pll_hdmi_lockloss_count = (buffer[12] << 24 | buffer[13] << 16 | buffer[14] << 8 | buffer[15]);
                    uint32_t control_resync_out_count = (buffer[25] << 24 | buffer[26] << 16 | buffer[27] << 8 | buffer[28]);
                    uint32_t monitor_sense_low_count = (buffer[29] << 24 | buffer[30] << 16 | buffer[31] << 8 | buffer[32]);

                    if (resetCounters) {
                        resetCounters = false;
                        pll_adv_lockloss_offset = pll_adv_lockloss_count;
                        hpd_low_offset = hpd_low_count;
                        pll54_lockloss_offset = pll54_lockloss_count;
                        pll_hdmi_lockloss_offset = pll_hdmi_lockloss_count;
                        control_resync_out_offset = control_resync_out_count;
                        monitor_sense_low_offset = monitor_sense_low_count;
                    }

                    uint16_t pinok1 = (buffer[16] << 5) | (buffer[17] >> 3);
                    uint16_t pinok2 = ((buffer[17] & 0x7) << 8) | buffer[18];
                    uint16_t resolX = (buffer[19] << 4) | (buffer[20] >> 4);
                    uint16_t resolY = ((buffer[20] & 0xF) << 8) | buffer[21];
                    snprintf(result, MENU_INF_RESULT_HEIGHT * MENU_WIDTH,
                        "No signal should be X on VMU screen!    "
                        "  00 01 02 03 04 05 06 07 08 09 10 11   "
                        "   %c  %c  %c  %c  %c  %c  %c  %c  %c  %c  %c  %c   "
                        "Raw Input Resolution: %03ux%03u  WiFi:%3d%%"
                        "Output Mode re-m/map: %02x %02x      ch: %2d "
                        "Res data/deint data : %02x %02x        %02x %02x"
                        "Raw data: %02x %02x %02x %02x %02x %02x %04x %04x   "
                        "advll: %05u   hpdl: %05u  msen: %05u "
                        "p54ll: %05u  phdll: %05u  rsyc: %05u ",
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
                        (resolX + 1) / 2, (resolY + 1), getWiFiQuality(WiFi.RSSI()),
                        remapResolution(CurrentResolution), mapResolution(CurrentResolution, true), WiFi.channel(),
                        CurrentResolutionData, CurrentDeinterlaceMode480i,
                        buffer[33], buffer[34],
                        buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[21],
                        pinok1, pinok2,
                        pll_adv_lockloss_count - pll_adv_lockloss_offset,
                        hpd_low_count - hpd_low_offset,
                        monitor_sense_low_count - monitor_sense_low_offset,
                        pll54_lockloss_count - pll54_lockloss_offset,
                        pll_hdmi_lockloss_count - pll_hdmi_lockloss_offset,
                        control_resync_out_count - control_resync_out_offset
                    );

                    if (!isStopped) {
                        fpgaTask.DoWriteToOSD(0, MENU_OFFSET + MENU_INF_RESULT_LINE, (uint8_t*) result, [&]() {
                            isRunning = true;
                        });
                    } else {
                        DEBUG("InfoTask: already stopped\n");
                    }
                }
            });
        }

        virtual void OnStop() {
            DEBUG("InfoTask: OnStop\n");
            isRunning = false;
            isStopped = true;
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
