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
#include <unistd.h>
#include "fastlz.h"
#include "firmware-utils.h"

#define USAGE "Usage: %s -f<file_number> input.archive output.file\n"
#define MAX_SUPPORTED_VERSION 2

size_t bytes_in_result = 0;
unsigned char *buffer = NULL;
unsigned char *result = NULL;
unsigned char *result_start = NULL;
size_t block_size = DEFAULT_BLOCK_SIZE; // v0 default
FILE* in;
FILE* out;
unsigned long fsize = 0;

void initBuffer(uint8_t *buffer, int len) {
    int i;
    for (i = 0 ; i < len ; i++) { 
        buffer[i] = 0xff; 
    }
}

int simulateEvent() {
    int bytes_write = 0;
    int chunk_size = 0;

    if (bytes_in_result == 0) {
        // reset the result buffer pointer
        result = result_start;
        // step 1: read chunk header
        if (fread(buffer, 1, 2, in) == 0) {
            // no more chunks
            return -1;
        }
        chunk_size = buffer[0] + (buffer[1] << 8);
        // step 2: read chunk length from file
        fread(buffer, 1, chunk_size, in);
        // step 3: decompress
        bytes_in_result = fastlz_decompress(buffer, chunk_size, result, block_size);
        // step 4: reverseBitOrder
        reverseBitOrder(result, bytes_in_result);
        return 0; /* no bytes written yet */
    }
    
    bytes_write = bytes_in_result > 256 ? 256 : bytes_in_result;
    //fwrite(result, 1, 256, out);
    fwrite(result, 1, bytes_write, out);
    // cleanup buffer afterwards
    initBuffer(result, 256);
    bytes_in_result -= bytes_write;
    result += bytes_write; /* advance the char* by written bytes */
    return bytes_write; /* 256 bytes written */
}

int process() {
    unsigned char header[16];
    unsigned long total_uncompressed = 0;
    unsigned long file_size = 0;
    int bytes_written = 0;

    fread(header, 1, 16, in);

    // check "magic"
    if (header[0] != 'D' || header[1] != 'C' || header[2] != 0x07 || header[3] != 0x04) {
        printf("Error: Invalid magic.\n\n");
        return -1;
    }

    // check version
    if (header[4] > MAX_SUPPORTED_VERSION || header[5] != 0x00) {
        printf("Error: Unsupported version magic v%d, only v%d or below is supported.\n\n", header[4], MAX_SUPPORTED_VERSION);
        return -1;
    }

    // read block size
    block_size = header[6] + (header[7] << 8);

    // read file size
    file_size = header[8] + (header[9] << 8) + (header[10] << 16) + (header[11] << 24);

    // ignore the reset for v1

    /* initialize memory */
    buffer = malloc(block_size);
    result = malloc(block_size);
    result_start = result;
    initBuffer(result, block_size);

    // unpack it!
    for(;;) {
        bytes_written = simulateEvent();
        if (bytes_written == -1) {
            break;
        }
        total_uncompressed += bytes_written;
        if (total_uncompressed >= file_size) {
             break;
        }
    }

    free(buffer);
    result = result_start;
    free(result);

    fclose(in);
    fclose(out);

    printf("%lu / %lu / %lu (%zu)\n", file_size, total_uncompressed, fsize, block_size);
    return 0;
}

int main(int argc, char** argv) {

    int opt;
    int total_files = 0;
    int file_to_extract = 0;
    char* input_file = 0;
    char* output_file = 0;
    unsigned int i = 0;
    unsigned char header[16];
    unsigned long pos;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            if (optarg) file_to_extract = atoi(optarg);
            break;
        default:
            fprintf(stderr, USAGE, argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind + 1 >= argc) {
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    } else {
        input_file = argv[optind];
        output_file = argv[optind+1];
    }

    in = fopen(input_file, "rb");
    if (!in) {
        printf("Error: could not open %s\n", input_file);
        return -1;
    }

    out = fopen(output_file, "wb");
    if(!out) {
        printf("Error: could not create %s. Aborted.\n\n", output_file);
        return -1;
    }

    fread(header, 1, 16, in);

    // check "magic"
    if (header[0] != 'D' || header[1] != 'C' || header[2] != 0x07 || header[3] != 0x04) {
        printf("Error: Invalid magic.\n\n");
        return -1;
    }

    // check version
    if (header[4] > MAX_SUPPORTED_VERSION || header[5] != 0x00) {
        printf("Error: Unsupported version magic v%d, only v%d or below is supported.\n\n", header[4], MAX_SUPPORTED_VERSION);
        return -1;
    }

    fseek(in, 0, SEEK_END);
    fsize = ftell(in);
    if (header[4] == 0x01) {
        total_files = 1;
        file_to_extract = 0;
        pos = 0;
        /* find size of the file */
        fseek(in, 0, SEEK_SET);
    } else {
        total_files = header[12];
        if (file_to_extract >= total_files) {
            file_to_extract = total_files - 1;
        }
        fseek(in, -((total_files - file_to_extract) * 4), SEEK_END);
        fread(header, 1, 4, in);
        pos = header[0] + (header[1] << 8) + (header[2] << 16) + (header[3] << 24);
        fseek(in, pos, SEEK_SET);
    }

    fprintf(stdout, "file to extract: %d/%d pos:%08lu\n", (file_to_extract + 1), total_files, pos);
    if (process() != 0) {
        return -1;
    }

    return 0;
}
