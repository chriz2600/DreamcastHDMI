#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <functional>

//////////////////////////////////////////////////////////////////////////////////

#define DBG_OUTPUT_PORT Serial
//#define DEBUG(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#define DEBUG(...) void(0)
#define DEBUG1(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
//#define DEBUG2(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#define DEBUG2(_1, ...) DBG_OUTPUT_PORT.printf_P(PSTR(_1), ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_SSID ""
#define DEFAULT_PASSWORD ""
#define DEFAULT_OTA_PASSWORD ""
#define DEFAULT_FW_SERVER "dc.i74.de"
#define DEFAULT_FW_SERVER_PATH ""
#define DEFAULT_FW_VERSION "master"
#define DEFAULT_FW_VARIANT FIRMWARE_STANDARD_FLAVOUR
#define DEFAULT_HTTP_USER "dchdmi"
#define DEFAULT_HTTP_PASS ""
#define DEFAULT_CONF_IP_ADDR ""
#define DEFAULT_CONF_IP_GATEWAY ""
#define DEFAULT_CONF_IP_MASK ""
#define DEFAULT_CONF_IP_DNS ""
#define DEFAULT_HOST "dc-firmware-manager"
#define DEFAULT_VIDEO_MODE VIDEO_MODE_STR_CABLE_DETECT
#define DEFAULT_VIDEO_RESOLUTION RESOLUTION_STR_VGA
#define DEFAULT_SCANLINES_ACTIVE SCANLINES_DISABLED
#define DEFAULT_SCANLINES_INTENSITY "175"
#define DEFAULT_240P_OFFSET "0"
#define DEFAULT_VGA_OFFSET VGA_OFFSET_AUTO_MODE_STR
#define DEFAULT_UPSCALING_MODE "0"
#define DEFAULT_COLOR_EXPANSION_MODE "3"
#define DEFAULT_GAMMA_MODE "15"
#define DEFAULT_COLOR_SPACE "0"
#define DEFAULT_SCANLINES_ODDEVEN SCANLINES_EVEN
#define DEFAULT_SCANLINES_THICKNESS SCANLINES_THIN
#define DEFAULT_RESET_MODE RESET_MODE_STR_LED
#define DEFAULT_DEINTERLACE_MODE DEINTERLACE_MODE_STR_BOB
#define DEFAULT_PROTECTED_MODE PROTECTED_MODE_STR_OFF
#define DEFAULT_KEYBOARD_LAYOUT US

#define US "us"
#define DE "de"
#define JP "jp"
//////////////////////////////////////////////////////////////////////////////////

#define CS 16
#define NCE 4
#define NCONFIG 5

#define FPGA_I2C_ADDR 0x3c
#define FPGA_I2C_FREQ_KHZ 733
#define FPGA_I2C_SCL 0
#define FPGA_I2C_SDA 2
#define CLOCK_STRETCH_TIMEOUT 200

#define CHANGELOG_FILE "/changelog"
#define FIRMWARE_FILE "/firmware.dc"
#define FIRMWARE_EXTENSION "dc"

#define FIRMWARE_STANDARD_FLAVOUR "std"
#define FIRMWARE_RELAXED_FLAVOUR "hq2x"

#define ESP_FIRMWARE_FILE "/firmware.bin"
#define ESP_FIRMWARE_EXTENSION "bin"
#define ESP_INDEX_FILE "/index.html.gz"
#define ESP_INDEX_STAGING_FILE "/esp.index.html.gz"

#define PAGES 8192 // 8192 pages x 256 bytes = 2MB = 16MBit
#define DEFAULT_MD5_SUM "00000000000000000000000000000000"
#define DEFAULT_MD5_SUM_ALT "ffffffffffffffffffffffffffffffff"

#define NO_ERROR 0
#define UNKNOWN_ERROR 255

#define ERROR_WRONG_MAGIC 16
#define ERROR_WRONG_VERSION 17
#define ERROR_FILE 18
#define ERROR_FILE_SIZE 19
#define ERROR_END_I2C_TRANSACTION 20
#define ERROR_ESP_FLASH 21
#define ERROR_ESP_FLASH_END 22
#define ERROR_ESP_INDEX_FLASH 23

#define FW_VERSION DCHDMI_VERSION

#define CHECK_BIT(var,pos) ((var) & (pos))
#define CHECK_CTRLR_MASK(var,pos) ((var | CTRLR_DATA_VALID) == (pos | CTRLR_DATA_VALID))

#define FPGA_UPDATE_OSD_DONE 0x00
#define FPGA_WRITE_DONE ((uint8_t) 0x01)

#define RESOLUTION_1080p (0x00)
#define RESOLUTION_960p (0x01)
#define RESOLUTION_480p (0x02)
#define RESOLUTION_VGA (0x03)
#define RESOLUTION_MOD_288p (0x04)
#define RESOLUTION_MOD_576p (0x08)
#define RESOLUTION_MOD_240p (0x10)
#define RESOLUTION_MOD_480i (0x20)
#define RESOLUTION_MOD_576i (0x40)

#define RESOLUTION_STR_1080p "1080p"
#define RESOLUTION_STR_960p "960p"
#define RESOLUTION_STR_480p "480p"
#define RESOLUTION_STR_VGA "VGA"

#define VIDEO_MODE_STR_FORCE_VGA "ForceVGA"
#define VIDEO_MODE_STR_CABLE_DETECT "CableDetect"
#define VIDEO_MODE_STR_SWITCH_TRICK "SwitchTrick"

#define VGA_OFF (0x00)
#define VGA_ON (0x80)

#define RESOLUTION_DATA_ADD_LINE (0x80)
#define RESOLUTION_DATA_LINE_DOUBLER (0x40)
#define RESOLUTION_DATA_IS_PAL (0x20)
#define RESOLUTION_DATA_OSD_STATE (0x08)

#define GENERATE_TIMING_AND_VIDEO (0x03)
#define FORCE_GENERATE_TIMING_AND_VIDEO (0x10)

#define I2C_OSD_ADDR_OFFSET (0x80)
#define I2C_OSD_ENABLE (0x81)
#define I2C_OSD_ACTIVE_LINE (0x82)
#define I2C_OUTPUT_RESOLUTION (0x83)
#define I2C_VIDEO_GEN (0x84)
#define I2C_CONTROLLER_AND_DATA_BASE (0x85)
#define I2C_METADATA (0x87)
#define I2C_CSEDATA_BASE (0x88)
#define I2C_SCANLINE_UPPER (0xF5)
#define I2C_SCANLINE_LOWER (0xF6)
#define I2C_240P_OFFSET (0x90)
#define I2C_ACTIVATE_HDMI (0x91)
#define I2C_UPSCALING_MODE (0x92)
#define I2C_COLOR_SPACE (0x93)
#define I2C_VGA_OFFSET (0x94)
#define I2C_COLOR_EXPANSION_AND_GAMMA_MODE (0xD1)
#define I2C_KEYBOARD_BASE (0xE0)
#define I2C_DC_RESET (0xF0)
#define I2C_OPT_RESET (0xF1)
#define I2C_RESET_CONF (0xF2)
#define I2C_NBP_RESET (0xF3)
#define I2C_PLL_RESET (0xF4)
#define I2C_PING (0xFF)

#define I2C_RECOVER_TRIES 100000
#define I2C_RECOVER_RETRY_INTERVAL_US 200

// nbp data
#define I2C_NBP_BASE (0xC3)
#define I2C_NBP_LENGTH 3

// controller and data
#define I2C_CONTROLLER_AND_DATA_BASE_LENGTH 6
#define I2C_KEYBOARD_LENGTH 4

// pinok data
#define I2C_TESTDATA_BASE (0xA0)
#define I2C_TESTDATA_LENGTH 38

#define I2C_CSEDATA_LENGTH 3

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
#define CTRLR_TRIGGER_DEFAULT_RESOLUTION (1<<(3))
#define CTRLR_DATA_VALID (1)

#define HQ2X_MODE_FLAG 0x01

typedef std::function<void(std::string data, int error)> ContentCallback;
typedef std::function<void(int read, int total, bool done, int error)> ProgressCallback;

#define PROGRESS_CALLBACK(done, err) ((progressCallback != NULL) ? progressCallback(readLength, totalLength, done, err) : (void)NULL)

#define LOCAL_FPGA_MD5 "/etc/last_flash_md5"
#define STAGED_FPGA_MD5 "/firmware.dc.md5"
#define SERVER_FPGA_MD5 "/server/firmware.dc.md5"
#define REMOTE_FPGA_MD5 (String(firmwareServerPath) + "/fw/" + String(firmwareVersion) + "/DCxPlus-v2.dc.md5")

#define LOCAL_ESP_MD5 "/etc/last_esp_flash_md5"
#define STAGED_ESP_MD5 "/firmware.bin.md5"
#define SERVER_ESP_MD5 "/server/firmware.bin.md5"
#define REMOTE_ESP_MD5 (String(firmwareServerPath) + "/esp/" + String(firmwareVersion) + "/4MB-firmware.bin.md5")

#define LOCAL_ESP_INDEX_MD5 "/index.html.gz.md5"
#define STAGED_ESP_INDEX_MD5 "/esp.index.html.gz.md5"
#define SERVER_ESP_INDEX_MD5 "/server/esp.index.html.gz.md5"
#define REMOTE_ESP_INDEX_MD5 (String(firmwareServerPath) + "/esp/" + String(firmwareVersion) + "/esp.index.html.gz.md5")

#define DEBUG_BASE_ADDRESS 0x90
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
#define DBG_DATA_HDMI_INT_PROCESSED_COUNT 23
#define DBG_DATA_NOT_READY_COUNT 24
#define DBG_DATA_RESYNC_COUNT 25
#define DEBUG_DATA_LEN 26

#define SCANLINES_ENABLED "on"
#define SCANLINES_DISABLED "off"
#define SCANLINES_ODD "odd"
#define SCANLINES_EVEN "even"
#define SCANLINES_THICK "thick"
#define SCANLINES_THIN "thin"

#define UPSCALING_MODE_2X 0x00
#define UPSCALING_MODE_HQ2X 0x01

#define COLOR_SPACE_AUTO 0x00
#define COLOR_SPACE_FULL 0x01
#define COLOR_SPACE_LIMITED 0x02

#define RESOLUTION_SWITCH_TIMEOUT 20000

#define RESET_MODE_STR_LED "led"
#define RESET_MODE_STR_GDEMU "gdemu"
#define RESET_MODE_STR_USBGDROM "usb-gdrom"
#define RESET_MODE_STR_MODE "mode"

#define RESET_MODE_LED (0x00)
#define RESET_MODE_GDEMU (0x01)
#define RESET_MODE_USBGDROM (0x02)
#define RESET_MODE_MODE (0x03)

#define DEINTERLACE_MODE_BOB (0x00)
#define DEINTERLACE_MODE_PASSTHRU (0x01)
#define DEINTERLACE_MODE_STR_BOB "bob"
#define DEINTERLACE_MODE_STR_PASSTHRU "passthru"

#define PROTECTED_MODE_ON (0x01)
#define PROTECTED_MODE_OFF (0x00)
#define PROTECTED_MODE_STR_ON "on"
#define PROTECTED_MODE_STR_OFF "off"

#define OSD_RESOLUTION(res) sprintf(data, res "%s", isRelaxedFirmware && UpscalingMode == UPSCALING_MODE_HQ2X && !(CurrentResolutionData & RESOLUTION_DATA_LINE_DOUBLER) ? " HQ" : "");

#define MENU_SPACER "\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07"

#define VGA_OFFSET_AUTO_MODE 1
#define VGA_OFFSET_AUTO_MODE_STR "1"
#define VGA_REFERENCE_POSITION 52

#define COLOR_EXP_RGB555 0
#define COLOR_EXP_RGB565 1
#define COLOR_EXP_OFF 3
#define COLOR_EXP_AUTO 7

#endif
