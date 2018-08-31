#ifndef GLOBAL_H
#define GLOBAL_H

#define CS 16
#define NCE 4
#define NCONFIG 5
#define DBG_OUTPUT_PORT Serial

#define FPGA_I2C_ADDR 0x3c
#define FPGA_I2C_FREQ_KHZ 733
#define FPGA_I2C_SCL 0
#define FPGA_I2C_SDA 2
#define CLOCK_STRETCH_TIMEOUT 200

#define FIRMWARE_FILE "/firmware.dc"
#define FIRMWARE_EXTENSION "dc"

#define ESP_FIRMWARE_FILE "/firmware.bin"
#define ESP_FIRMWARE_EXTENSION "bin"
#define ESP_INDEX_FILE "/index.html.gz"
#define ESP_INDEX_STAGING_FILE "/esp.index.html.gz"

#define PAGES 8192 // 8192 pages x 256 bytes = 2MB = 16MBit
#define DEBUG true
#define DEFAULT_MD5_SUM "00000000000000000000000000000000"

#define NO_ERROR 0
#define UNKNOWN_ERROR 255

#define ERROR_WRONG_MAGIC 16
#define ERROR_WRONG_VERSION 17
#define ERROR_FILE 18
#define ERROR_FILE_SIZE 19
#define ERROR_END_I2C_TRANSACTION 20

#define FW_VERSION "__FW_VERSION__"

#define CHECK_BIT(var,pos) ((var) & (pos))
#define CHECK_MASK(var,pos) ((var) == (pos))

#define FPGA_UPDATE_OSD_DONE 0x00
#define FPGA_WRITE_DONE ((uint8_t) 0x01)

#define RESOLUTION_1080p (0x00)
#define RESOLUTION_960p (0x01)
#define RESOLUTION_480p (0x02)
#define RESOLUTION_VGA (0x03)

#define RESOLUTION_STR_1080p "1080p"
#define RESOLUTION_STR_960p "960p"
#define RESOLUTION_STR_480p "480p"
#define RESOLUTION_STR_VGA "VGA"

#define VIDEO_MODE_STR_FORCE_VGA "ForceVGA"
#define VIDEO_MODE_STR_CABLE_DETECT "CableDetect"
#define VIDEO_MODE_STR_SWITCH_TRICK "SwitchTrick"

#define VGA_OFF (0x00)
#define VGA_ON (0x80)

#define PLL_RESET_OFF (0x00)
#define PLL_RESET_ON (0x40)

#define I2C_OSD_ADDR_OFFSET (0x80)
#define I2C_OSD_ENABLE (0x81)
#define I2C_OSD_ACTIVE_LINE (0x82)
#define I2C_OUTPUT_RESOLUTION (0x83)
#define I2C_SCANLINE_UPPER (0x87)
#define I2C_SCANLINE_LOWER (0x88)
#define I2C_PING (0xFF)

#define I2C_RECOVER_TRIES 100000
#define I2C_RECOVER_RETRY_INTERVAL_US 200

// // controller data, int16
// /*
//     15: a, 14: b, 13: x, 12: y, 11: up, 10: down, 09: left, 08: right
//     07: start, 06: ltrigger, 05: rtrigger, 04: trigger_osd
// */
#define CTRLR_BUTTON_A (1<<(15))
#define CTRLR_BUTTON_B (1<<(14))
#define CTRLR_BUTTON_X (1<<(13))
#define CTRLR_BUTTON_Y (1<<(12))
#define CTRLR_PAD_UP (1<<(11))
#define CTRLR_PAD_DOWN (1<<(10))
#define CTRLR_PAD_LEFT (1<<(9))
#define CTRLR_PAD_RIGHT (1<<(8))
#define CTRLR_BUTTON_START (1<<(7))
#define CTRLR_LTRIGGER (1<<(6))
#define CTRLR_RTRIGGER (1<<(5))
#define CTRLR_TRIGGER_OSD (1<<(4))

typedef std::function<void(std::string data, int error)> ContentCallback;
typedef std::function<void(int read, int total, bool done, int error)> ProgressCallback;

#define PROGRESS_CALLBACK(done, err) ((progressCallback != NULL) ? progressCallback(readLength, totalLength, done, err) : (void)NULL)

#define REMOTE_FPGA_HOST "dc.i74.de"
#define REMOTE_ESP_HOST "esp.i74.de"

#define LOCAL_FPGA_MD5 "/etc/last_flash_md5"
#define STAGED_FPGA_MD5 "/firmware.dc.md5"
#define REMOTE_FPGA_MD5 ("/fw/" + String(firmwareVersion) + "/DCxPlus-default.dc.md5")

#define LOCAL_ESP_MD5 "/etc/last_esp_flash_md5"
#define STAGED_ESP_MD5 "/firmware.bin.md5"
#define REMOTE_ESP_MD5 ("/" + String(firmwareVersion) + "/4MB-firmware.bin.md5")

#define LOCAL_ESP_INDEX_MD5 "/index.html.gz.md5"
#define STAGED_ESP_INDEX_MD5 "/esp.index.html.gz.md5"
#define REMOTE_ESP_INDEX_MD5 ("/" + String(firmwareVersion) + "/esp.index.html.gz.md5")

#define DEBUG_BASE_ADDRESS 0x90
#define DEBUG_DATA_LEN 24

#define DBG_DATA_PLL_ERRORS 0
#define DBG_DATA_TEST 1
#define DBG_DATA_FRAMECOUNTER_LOW 2
#define DBG_DATA_FRAMECOUNTER_HIGH 3
#define DBG_DATA_PLL_STATUS 4
#define DBG_DATA_ID_CHECK_HIGH 5
#define DBG_DATA_ID_CHECK_LOW 6
#define DBG_DATA_CHIP_REVISION 7
#define DBG_DATA_VIC_DETECTED 8
#define DBG_DATA_VIC_TO_RX 9
#define DBG_DATA_MISC_DATA 10
#define DBG_DATA_RESTART_COUNT 11
#define DBG_DATA_CTS1_STATUS 12
#define DBG_DATA_CTS2_STATUS 13
#define DBG_DATA_CTS3_STATUS 14
#define DBG_DATA_MAX_CTS1_STATUS 15
#define DBG_DATA_MAX_CTS2_STATUS 16
#define DBG_DATA_MAX_CTS3_STATUS 17
#define DBG_DATA_SUMMARY_CTS1_STATUS 18
#define DBG_DATA_SUMMARY_CTS2_STATUS 19
#define DBG_DATA_SUMMARY_CTS3_STATUS 20
#define DBG_DATA_SUMMARY_SUMMARY_CTS3_STATUS 21
#define DBG_DATA_HDMI_INT_COUNT 22
#define DBG_DATA_NOT_READY_COUNT 23

#define SCANLINES_ENABLED "on"
#define SCANLINES_DISABLED "off"
#define SCANLINES_ODD "odd"
#define SCANLINES_EVEN "even"
#define SCANLINES_THICK "thick"
#define SCANLINES_THIN "thin"

#endif
