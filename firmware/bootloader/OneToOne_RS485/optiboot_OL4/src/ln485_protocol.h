#ifndef LN485_PROTOCOL_H
#define LN485_PROTOCOL_H

#include <stdint.h>

/*
 * Optiboot_OL4 Protocol Constants & PID Definitions
 * Target: ATtiny1616 (ADX Core-D)
 *
 * PID Parity Calculation:
 *   P0 = ID0 ^ ID1 ^ ID2 ^ ID4
 *   P1 = !(ID1 ^ ID3 ^ ID4 ^ ID5)
 *   PID = (P1 << 7) | (P0 << 6) | (ID & 0x3F)
 */

#define OL4_MAJVER 1
#define OL4_MINVER 2

// Protected Identifiers (PIDs)
#define PID_PING        0x80  // ID 0x00: Master-Pub -> Slave-Pub: Ping / Keep-Alive
#define PID_GET_INFO    0xC1  // ID 0x01: Master-Header -> Slave-Pub: Device Info & Signature
#define PID_SET_ADDR    0x42  // ID 0x02: Master-Pub -> Slave-Pub: Load 16-bit Flash Address
#define PID_WRITE_CHUNK 0x03  // ID 0x03: Master-Pub -> Slave-Pub: Write 8-byte Flash Chunk
#define PID_COMMIT_PAGE 0xC4  // ID 0x04: Master-Pub -> Slave-Pub: Erase & Write Page Buffer to Flash
#define PID_READ_CHUNK  0x85  // ID 0x05: Master-Pub -> Slave-Pub: Read 8-byte Flash Chunk
#define PID_REBOOT      0x06  // ID 0x06: Master-Pub: Reboot to Application (0x0400)

// Response Status Codes
#define STATUS_OK           0x00
#define STATUS_ERR_CRC      0x01
#define STATUS_ERR_ADDR     0x02
#define STATUS_ERR_FLASH    0x03
#define STATUS_ERR_UNKNOWN  0xFF

#define FLASH_PAGE_SIZE     64
#define CHUNK_SIZE          8
#define CHUNKS_PER_PAGE     8
#define APP_START_ADDR      0x0400

#endif // LN485_PROTOCOL_H
