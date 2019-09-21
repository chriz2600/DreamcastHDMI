#include <SPIFlash.h>

SPIFlash::SPIFlash(int cs)
{
    _cs = cs;
    pinMode(_cs, INPUT);
}

bool SPIFlash::is_busy_async() {
    bool ret;
    digitalWrite(_cs, HIGH);  
    digitalWrite(_cs, LOW);
    SPI.transfer(WB_READ_STATUS_REG_1);       
    ret = (SPI.transfer(0) & 1);
    digitalWrite(_cs, HIGH);  
    return ret;
}

void SPIFlash::not_busy() {
    while (is_busy_async()) {
        yield();
    }
    // digitalWrite(_cs, HIGH);  
    // digitalWrite(_cs, LOW);
    // SPI.transfer(WB_READ_STATUS_REG_1);       
    // while (SPI.transfer(0) & 1) {
    //     yield();
    // }; 
    // digitalWrite(_cs, HIGH);  
}

void SPIFlash::page_read_async(unsigned int page_number, uint8_t *page_buffer) {
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);
    SPI.transfer(WB_READ_DATA);
    // Construct the 24-bit address from the 16-bit page
    // number and 0x00, since we will read 256 bytes (one
    // page).
    SPI.transfer((page_number >> 8) & 0xFF);
    SPI.transfer((page_number >> 0) & 0xFF);
    SPI.transfer(0);
    for (int i = 0; i < 256; ++i) {
        page_buffer[i] = SPI.transfer(0);
    }
    digitalWrite(_cs, HIGH);
}

void SPIFlash::page_read(unsigned int page_number, uint8_t *page_buffer) {
    page_read_async(page_number, page_buffer);
    not_busy();
}

void SPIFlash::chip_erase_async(void) {
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);  
    SPI.transfer(WB_WRITE_ENABLE);
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);
    SPI.transfer(WB_CHIP_ERASE);
    digitalWrite(_cs, HIGH);
}

void SPIFlash::chip_erase(void) {
    chip_erase_async();
    not_busy();
}

void SPIFlash::page_write_async(unsigned int page_number, uint8_t *page_buffer) {
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);  
    SPI.transfer(WB_WRITE_ENABLE);
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);  
    SPI.transfer(WB_PAGE_PROGRAM);
    SPI.transfer((page_number >>  8) & 0xFF);
    SPI.transfer((page_number >>  0) & 0xFF);
    SPI.transfer(0);
    for (int i = 0; i < 256; ++i) {
        SPI.transfer(page_buffer[i]);
    }
    digitalWrite(_cs, HIGH);
}    

void SPIFlash::page_write(unsigned int page_number, uint8_t *page_buffer) {
    page_write_async(page_number, page_buffer);
    not_busy();
}

/*
12	MISO
13	MOSI
14 (SCL) SCLK
*/

void SPIFlash::enable(void) {
    SPI.begin();
    SPI.setDataMode(0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setFrequency(40000000L);
    // initialze to not bother with _cs
    pinMode(_cs, OUTPUT);
}

void SPIFlash::disable(void) {
    SPI.end();
    // disable _cs
    pinMode(_cs, INPUT);
}
