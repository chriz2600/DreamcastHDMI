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
#include "fastlz.h"
#include "firmware-utils.h"

int main(int argc, char** argv) {
    FILE* in;
    FILE* out;
    char* input_file = 0;
    char* output_file = 0;
    unsigned int i = 0;
    unsigned long fsize = 0;
    unsigned char buffer[BLOCK_SIZE];
    unsigned char result[BLOCK_SIZE+256];
    unsigned char header[16];
    unsigned char chunk_header[2];
    unsigned long total_read = 0;
    unsigned long total_compressed = 0;
    int chunk_size = 0;

    for(i = 1; i <= argc; i++) {
        char* argument = argv[i];
        if(!input_file) {
            input_file = argument;
            continue;
        }
        if (!output_file) {
            output_file = argument;
            continue;
        }
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

    /* find size of the file */
    fseek(in, 0, SEEK_END);
    fsize = ftell(in);
    fseek(in, 0, SEEK_SET);

    // create header
    // "magic"
    header[0] = 'D';
    header[1] = 'C';
    header[2] = 0x07;
    header[3] = 0x04;
    // version
    header[4] = 0x01;
    header[5] = 0x00;
    // block size
    header[6] = BLOCK_SIZE & 255;
    header[7] = (BLOCK_SIZE >> 8) & 255;
    // file size 
    header[8] = fsize & 255;
    header[9] = (fsize >> 8) & 255;
    header[10] = (fsize >> 16) & 255;
    header[11] = (fsize >> 24) & 255;
    // currently not used
    header[12] = 0x00;
    header[13] = 0x00;
    header[14] = 0x00;
    header[15] = 0x00;

    fwrite(header, 16, 1, out);

    // pack it!
    for(;;) {
        size_t bytes_read = fread(buffer, 1, BLOCK_SIZE, in);
        if (bytes_read == 0)
            break;
        total_read += bytes_read;

        reverseBitOrder(buffer, bytes_read);
        chunk_size = fastlz_compress(buffer, bytes_read, result);

        chunk_header[0] = chunk_size & 255;
        chunk_header[1] = (chunk_size >> 8) & 255;

        fwrite(chunk_header, 2, 1, out);
        fwrite(result, 1, chunk_size, out);
        
        total_compressed += chunk_size + 2 /* header */;
        //printf("%i\n", chunk_size);
    }

    fclose(in);
    if(total_read != fsize) {
        printf("\n");
        printf("Error: reading %s failed!\n", input_file);
        return -1;
    }
    fclose(out);

    printf("%s: %lu / %lu\n", input_file, fsize, total_compressed);
    return 0;
}
