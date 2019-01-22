#ifndef FPGA_TASK_H
#define FPGA_TASK_H

#include "../global.h"
#include <Task.h>
#include <inttypes.h>
#include <string.h>
#include <brzo_i2c.h>

#define MAX_ADDR_SPACE 128
#define REPEAT_DELAY 250
#define REPEAT_RATE 100

typedef std::function<void(uint16_t controller_data, bool isRepeat)> FPGAEventHandlerFunction;
typedef std::function<void(uint8_t Address, uint8_t Value)> WriteCallbackHandlerFunction;
typedef std::function<void(uint8_t address, uint8_t* buffer, uint8_t len)> ReadCallbackHandlerFunction;
typedef std::function<void()> WriteOSDCallbackHandlerFunction;

extern uint8_t ForceVGA;
extern uint8_t CurrentResolution;
extern uint8_t CurrentResolutionData;
void switchResolution();
void storeResolutionData(uint8_t data);

void setupI2C() {
    DEBUG(">> Setting up I2C master...\n");
    brzo_i2c_setup(FPGA_I2C_SDA, FPGA_I2C_SCL, CLOCK_STRETCH_TIMEOUT);
}

extern TaskManager taskManager;
uint8_t mapResolution(uint8_t data);

class FPGATask : public Task {

    public:
        FPGATask(uint8_t repeat, FPGAEventHandlerFunction chandler) :
            Task(repeat),
            controller_handler(chandler)
        { };

        virtual void DoWriteToOSD(uint8_t column, uint8_t row, uint8_t charData[]) {
            DoWriteToOSD(column, row, charData, NULL);
        }

        virtual void DoWriteToOSD(uint8_t column, uint8_t row, uint8_t charData[], WriteOSDCallbackHandlerFunction handler) {
            //DEBUG("DoWriteToOSD: %u %u %u\n", column, row, strlen((char*) charData));
            if (column > 39) { column = 39; }
            if (row > 23) { row = 23; }

            stringLength = strlen((char*) charData);
            localAddress = row * 40 + column;
            left = stringLength;
            if (data_in != NULL) {
                free(data_in); data_in = NULL;
            }
            data_in = (uint8_t*) malloc(stringLength);
            memcpy(data_in, charData, stringLength);
            updateOSDContent = true;
            write_osd_callback = handler;
        }

        virtual void Write(uint8_t address, uint8_t value) {
            Write(address, value, NULL);
        }

        virtual void Write(uint8_t address, uint8_t value, WriteCallbackHandlerFunction handler) {
            Address = address;
            Value = value;
            Update = true;
            write_callback = handler;
        }

        virtual void Read(uint8_t address, uint8_t len, ReadCallbackHandlerFunction handler) {
            Address = address;
            Value = len;
            DoRead = true;
            read_callback = handler;
        }

        void ForceLoop() {
            OnUpdate(0L);
        }

    private:
        FPGAEventHandlerFunction controller_handler;
        WriteCallbackHandlerFunction write_callback;
        WriteOSDCallbackHandlerFunction write_osd_callback;
        ReadCallbackHandlerFunction read_callback;

        uint8_t *data_in;

        uint8_t data_out[MAX_ADDR_SPACE+1];
        uint8_t data_write[MAX_ADDR_SPACE+1];
        uint16_t stringLength;
        uint16_t left;
        uint16_t localAddress;
        uint8_t upperAddress;
        uint8_t lowerAddress;
        uint8_t towrite;
        bool updateOSDContent;
        bool Update;
        bool DoRead;

        uint8_t Address;
        uint8_t Value;

        long eTime;
        uint8_t repeatCount;
        bool GotError = false;

        virtual bool OnStart() {
            return true;
        }

        virtual void OnUpdate(uint32_t deltaTime) {
            brzo_i2c_start_transaction(FPGA_I2C_ADDR, FPGA_I2C_FREQ_KHZ);
            if (updateOSDContent) {
                //DEBUG("updateOSDContent: stringLength: %u, left: %u\n", stringLength, left);
                if (left > 0) {
                    upperAddress = localAddress / MAX_ADDR_SPACE;
                    lowerAddress = localAddress % MAX_ADDR_SPACE;
                    data_write[0] = I2C_OSD_ADDR_OFFSET;
                    data_write[1] = upperAddress;
                    brzo_i2c_write(data_write, 2, false);
                    data_write[0] = lowerAddress;
                    towrite = MAX_ADDR_SPACE - lowerAddress;
                    if (towrite > left) { towrite = left; }
                    memcpy(&data_write[1], &data_in[stringLength-left], towrite);
                    brzo_i2c_write(data_write, towrite + 1, false);
                    left -= towrite;
                    localAddress += towrite;
                } else {
                    free(data_in); data_in = NULL;
                    updateOSDContent = false;
                    if (write_osd_callback != NULL) {
                        write_osd_callback();
                    }
                }
            } else if (Update) {
                //DEBUG("Write: %x %x\n", Address, Value);
                uint8_t buffer[2];
                buffer[0] = Address;
                buffer[1] = Value;
                brzo_i2c_write(buffer, 2, false);
                Update = false;
                if (write_callback != NULL) {
                    write_callback(Address, Value);
                }
            } else if (DoRead) {
                // Value is read len here
                //DEBUG("Read: %x %x\n", Address, Value);
                uint8_t buffer[1];
                uint8_t buffer2[Value];
                buffer[0] = Address;
                brzo_i2c_write(buffer, 1, false);
                brzo_i2c_read(buffer2, Value, false);
                DoRead = false;
                if (read_callback != NULL) {
                    read_callback(Address, buffer2, Value);
                }
            } else {
                // update controller data
                uint8_t buffer[1];
                uint8_t buffer2[I2C_CONTROLLER_AND_DATA_BASE_LENGTH];
                buffer[0] = I2C_CONTROLLER_AND_DATA_BASE;
                brzo_i2c_write(buffer, 1, false);
                brzo_i2c_read(buffer2, I2C_CONTROLLER_AND_DATA_BASE_LENGTH, false);

                // new controller data
                if (buffer2[0] != data_out[0]
                 || buffer2[1] != data_out[1])
                {
                    //DEBUG("I2C_CONTROLLER_AND_DATA_BASE, new controller data: %04x\n", buffer2[0] << 8 | buffer2[1]);
                    controller_handler(buffer2[0] << 8 | buffer2[1], false);
                    // reset repeat
                    eTime = millis();
                    repeatCount = 0;
                } else {
                    // check repeat
                    if (buffer2[0] != 0x00 || buffer2[1] != 0x00) {
                        unsigned long duration = (repeatCount == 0 ? REPEAT_DELAY : REPEAT_RATE);
                        if (millis() - eTime > duration) {
                            controller_handler(buffer2[0] << 8 | buffer2[1], true);
                            eTime = millis();
                            repeatCount++;
                        }
                    }
                }
                // new meta data
                if ((buffer2[2]) != (CurrentResolutionData) /*data_out[2]*/) {
                    DEBUG1("I2C_CONTROLLER_AND_DATA_BASE, switch to: %02x\n", buffer2[2]);
                    storeResolutionData(buffer2[2]);
                    switchResolution();
                }
                memcpy(data_out, buffer2, I2C_CONTROLLER_AND_DATA_BASE_LENGTH);
            }
            if (brzo_i2c_end_transaction()) {
                if (!GotError) {
                    last_error = ERROR_END_I2C_TRANSACTION;
                    DEBUG1("--> ERROR_END_I2C_TRANSACTION\n");
                }
                GotError = true;
            } else {
                if (GotError) {
                    last_error = NO_ERROR;
                    DEBUG1("<-- FINISHED_I2C_TRANSACTION\n");
                }
                GotError = false;
            }
        }

        virtual void OnStop() {
            DEBUG("OnStop\n");
            if (data_in != NULL) {
                free(data_in); data_in = NULL;
            }
        }
};

#endif