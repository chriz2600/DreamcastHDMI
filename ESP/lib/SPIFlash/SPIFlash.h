#ifndef SPI_FLASHY_H
#define SPI_FLASHY_H

#include <Arduino.h>
#include <SPI.h>

#define WB_WRITE_ENABLE       0x06
#define WB_WRITE_DISABLE      0x04
#define WB_CHIP_ERASE         0xc7
#define WB_READ_STATUS_REG_1  0x05
#define WB_READ_DATA          0x03
#define WB_PAGE_PROGRAM       0x02
#define WB_JEDEC_ID           0x9f

class SPIFlash
{
  public:
    SPIFlash(int cs);
    void page_read(unsigned int page_number, uint8_t *page_buffer);
    void page_write(unsigned int page_number, uint8_t *page_buffer);
    void chip_erase();

    void page_read_async(unsigned int page_number, uint8_t *page_buffer);
    void page_write_async(unsigned int page_number, uint8_t *page_buffer);
    void chip_erase_async();
    bool is_busy_async();
    
    void enable();
    void disable();
private:
    void not_busy();
    int _cs;
};

#endif