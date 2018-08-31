/*
  6PACK - file compressor using FastLZ (lightning-fast compression library)

  Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "firmware-utils.h"
#define FPGA_I2C_ADDR 0x3c
#define FPGA_I2C_FREQ_KHZ 800
#define false 0
#define true 1

void brzo_i2c_start_transaction(uint8_t addr, int speed);
void brzo_i2c_write(const uint8_t *data, uint8_t len, uint8_t rs);
int brzo_i2c_end_transaction();

#include "osd_ram.h"

void brzo_i2c_start_transaction(uint8_t addr, int speed) {
    printf("writing to %02x with %u kHz\n", addr, speed);
}

int brzo_i2c_end_transaction() {
    return 0;
}

void brzo_i2c_write(const uint8_t *data, uint8_t len, uint8_t rs) {
    int c = 0;
    printf("BASE_ADDR 0x%02x (%03u)\n", data[0], data[0]);
    printf(" ");
    for (int i = 1 ; i < len ; i++) {
        printf(" 0x%02x (%c)", data[i], data[i]);
        if (c == 7) {
            c = 0;
            printf("\n ");
        } else {
            c++;
        }
    }
    printf("\n");
}




int main(int argc, char** argv) {
    char* charData = 0;
    int column = -1;
    int row = -1;
    int i;

    for(i = 1; i <= argc; i++) {
        char* argument = argv[i];
        if (!argument) {
            continue;
        }
        if(!charData) {
            charData = argument;
            continue;
        }
        if (column == -1) {
            column = atoi(argument);
            continue;
        }
        if (row == -1) {
            row = atoi(argument);
            continue;
        }
    }

    if (column == -1) { column = 0; }
    if (row == -1) { row = 0; }

    if (!charData) {
        printf("Error: usage: test <string>\n");
        return -1;
    }

    printf("%i %i %s\n", column, row, charData);

    writeToOSD(column, row, (uint8_t*) charData);

    return 0;
}
