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

#define USAGE "Usage: %s [-v 1|2 -b <blocksize>] input.rbf [...] output.dc\n"

size_t block_size = DEFAULT_BLOCK_SIZE; // v0 default
enum { V1_OP, V2_OP } op_mode = V1_OP;  // Default set

int process(FILE* in, FILE* out, int total_files) {
    unsigned int i = 0;
    unsigned long fsize = 0;
    unsigned char header[16];
    unsigned char chunk_header[2];
    unsigned long total_read = 0;
    unsigned long total_compressed = 0;
    int chunk_size = 0;
    int max_chunk_size = 0;

    if (block_size < MIN_BLOCK_SIZE || block_size > MAX_BLOCK_SIZE) {
        printf("Error: block size must be between %zu and %zu. Aborted.\n\n", MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
        return -1;
    }

    unsigned char buffer[block_size];
    unsigned char result[block_size+256];

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
    header[4] = op_mode+1;
    header[5] = 0x00;
    // block size
    header[6] = block_size & 255;
    header[7] = (block_size >> 8) & 255;
    // file size 
    header[8] = fsize & 255;
    header[9] = (fsize >> 8) & 255;
    header[10] = (fsize >> 16) & 255;
    header[11] = (fsize >> 24) & 255;
    // currently not used
    header[12] = total_files;
    header[13] = 0x00;
    header[14] = 0x00;
    header[15] = 0x00;

    fwrite(header, 16, 1, out);

    // pack it!
    for(;;) {
        size_t bytes_read = fread(buffer, 1, block_size, in);
        if (bytes_read == 0)
            break;
        total_read += bytes_read;

        reverseBitOrder(buffer, bytes_read);
        chunk_size = fastlz_compress(buffer, bytes_read, result);

        if (chunk_size > max_chunk_size) {
            max_chunk_size = chunk_size;
        }

        chunk_header[0] = chunk_size & 255;
        chunk_header[1] = (chunk_size >> 8) & 255;

        fwrite(chunk_header, 2, 1, out);
        fwrite(result, 1, chunk_size, out);
        
        total_compressed += chunk_size + 2 /* header */;
        //printf("%i\n", chunk_size);
    }

    if(total_read != fsize) {
        printf("\n");
        printf("Error: reading file failed!\n");
        return -1;
    }

    printf("%lu / %lu (%zu) (%d)\n", fsize, total_compressed, block_size, max_chunk_size);
    return 0;
}

int main(int argc, char** argv) {
    int opt;
    int total_files = 0;
    char* input_file[MAX_FILES];
    char* output_file = 0;

    while ((opt = getopt(argc, argv, "v:b:")) != -1) {
        switch (opt) {
        case 'v':
            if (optarg && optarg[0] == '2') {
                op_mode = V2_OP;
            } else {
                op_mode = V1_OP;
            }
            break;
        case 'b':
            if (optarg) block_size = atoi(optarg);
            break;
        default:
            fprintf(stderr, USAGE, argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    fprintf(stdout, "Version: %s, block size: %lu\n", op_mode == V2_OP ? "v2" : "v1", block_size);

    /* Process file names or stdin */
    if (optind + 1 >= argc) {
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    } else {
        int i;
        for (i = optind; i < argc; i++) {
            if (i == argc - 1) {
                output_file = argv[i];
            } else {
                if (total_files >= MAX_FILES) {
                    fprintf(stderr, "WARNING: v2 can only pack 8 files, skipping: %s\n", argv[i]);
                } else if (op_mode == V2_OP || total_files == 0) {
                    input_file[total_files] = argv[i];
                    total_files++;
                } else {
                    fprintf(stderr, "WARNING: v1 can only pack one file, skipping: %s\n", argv[i]);
                }
            }
        }
    }

    FILE* in;
    FILE* out;
    unsigned long poses[MAX_FILES];
    unsigned char footer[POS_SIZE];

    fprintf(stdout, "Output file:\n");
    fprintf(stdout, "  %s\n", output_file);
    out = fopen(output_file, "wb");
    if(!out) {
        printf("Error: could not create %s. Aborted.\n\n", output_file);
        return -1;
    }

    fprintf(stdout, "%d input files:\n", total_files);
    for (int i = 0 ; i < total_files ; i++) {
        poses[i] = ftell(out);
        fprintf(stdout, "  %08lu %s: ", poses[i], input_file[i]);
        in = fopen(input_file[i], "rb");
        if (!in) {
            printf("Error: could not open %s\n", input_file[i]);
            return -1;
        }
        if (process(in, out, total_files) != 0) {
            return -1;
        }
        fclose(in);
    }

    for (int i = 0 ; i < total_files ; i++) {
        footer[0] = poses[i] & 255;
        footer[1] = (poses[i] >> 8) & 255;
        footer[2] = (poses[i] >> 16) & 255;
        footer[3] = (poses[i] >> 24) & 255;
        fwrite(footer, 4, 1, out);
        printf("%02x %02x %02x %02x\n", footer[0], footer[1], footer[2], footer[3]);
    }

    fclose(out);
    return 0;
}
