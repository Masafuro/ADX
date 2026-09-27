#ifndef LN485_PROTOCOL_H
#define LN485_PROTOCOL_H

#include <stdint.h>

#define OL4_MAJVER 2
#define OL4_MINVER 0

#define PKT_STX               0x02
#define PKT_ETX               0x03

// Commands (Host -> Core-D)
#define CMD_PING              0x01
#define CMD_GET_CHIP_INFO     0x02
#define CMD_WRITE_PAGE        0x10
#define CMD_VERIFY_PAGE       0x11
#define CMD_BOOT_APP          0x20

// Responses (Core-D -> Host)
#define RESP_ACK              0x06
#define RESP_NAK              0x15

// Status Codes
#define STATUS_OK             0x00
#define STATUS_ERR_CRC        0x01
#define STATUS_ERR_PROTECTED  0x02
#define STATUS_ERR_UNKNOWN    0x03

// Flash Parameters (ATtiny1616)
#define FLASH_PAGE_SIZE       64
#define APP_START_ADDR        0x0400
#define APP_START_PAGE        0x10  // 0x0400 / 64 = 16 (0x10)
#define FLASH_TOTAL_PAGES     256   // 16KB / 64 = 256 (0x00 - 0xFF)

// Signature for ATtiny1616
#define CHIP_SIG0             0x1E
#define CHIP_SIG1             0x94
#define CHIP_SIG2             0x21

#endif // LN485_PROTOCOL_H
