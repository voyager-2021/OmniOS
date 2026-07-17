#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   OmniOS - ATA PIO Disk Driver
   Reads sectors from primary ATA drive using PIO mode.
   ============================================================ */

#define ATA_PRIMARY_IO      0x1F0
#define ATA_PRIMARY_CTRL    0x3F6

#define ATA_MASTER          0xE0
#define ATA_SLAVE           0xF0

#define ATA_CMD_READ        0x20
#define ATA_CMD_IDENTIFY    0xEC

#define ATA_SR_BSY          0x80
#define ATA_SR_DRDY         0x40
#define ATA_SR_DRQ          0x08
#define ATA_SR_ERR          0x01

typedef struct {
    uint16_t io_base;
    uint16_t ctrl_base;
    uint8_t  drive_select;   /* ATA_MASTER or ATA_SLAVE */
    bool     present;
    uint32_t sector_count;
    char     model[41];
} ATA_Drive;

/* Initialise and detect primary master drive */
bool ATA_Initialize(ATA_Drive *drive);

/* Read sectors using 28-bit LBA PIO */
bool ATA_ReadSectors(ATA_Drive *drive, uint32_t lba, uint8_t count, void *buffer);

/* Get drive info */
void ATA_PrintInfo(ATA_Drive *drive);