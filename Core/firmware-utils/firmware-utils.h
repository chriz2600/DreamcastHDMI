#include <inttypes.h>
#define DEFAULT_BLOCK_SIZE ((size_t)1536)
#define MIN_BLOCK_SIZE ((size_t)256)
#define MAX_BLOCK_SIZE ((size_t)16384)

void reverseBitOrder(uint8_t *buffer, int length) {
    for (int i = 0 ; i < length ; i++) { 
        buffer[i] = (buffer[i] & 0xF0) >> 4 | (buffer[i] & 0x0F) << 4;
        buffer[i] = (buffer[i] & 0xCC) >> 2 | (buffer[i] & 0x33) << 2;
        buffer[i] = (buffer[i] & 0xAA) >> 1 | (buffer[i] & 0x55) << 1;
    }
}

