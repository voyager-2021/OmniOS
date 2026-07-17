/* ============================================================
   OmniOS - ATA PIO Disk Driver
   28-bit LBA, PIO mode, primary master only.
   ============================================================ */
#include "ata.h"
#include "../arch/i686/io.h"
#include "../stdio.h"
#include <stdint.h>
#include <stdbool.h>

/* ---- Port offsets from io_base ---- */
#define ATA_REG_DATA        0
#define ATA_REG_ERROR       1
#define ATA_REG_SEC_COUNT   2
#define ATA_REG_LBA_LO      3
#define ATA_REG_LBA_MID     4
#define ATA_REG_LBA_HI      5
#define ATA_REG_DRIVE_HEAD  6
#define ATA_REG_STATUS      7
#define ATA_REG_COMMAND     7

static void ata_wait_bsy(uint16_t io)
{
    while (i686_inb(io + ATA_REG_STATUS) & ATA_SR_BSY)
        ;
}

static void ata_wait_drq(uint16_t io)
{
    while (!(i686_inb(io + ATA_REG_STATUS) & ATA_SR_DRQ))
        ;
}

static bool ata_poll(uint16_t io)
{
    /* Read status 4 times to give drive time */
    for (int i = 0; i < 4; i++)
        i686_inb(io + ATA_REG_STATUS);

    ata_wait_bsy(io);

    uint8_t status = i686_inb(io + ATA_REG_STATUS);
    if (status & ATA_SR_ERR) return false;
    if (!(status & ATA_SR_DRQ)) return false;
    return true;
}

static void ata_read_words(uint16_t io, uint16_t *buf, int count)
{
    for (int i = 0; i < count; i++) {
        uint16_t lo = i686_inb(io + ATA_REG_DATA);
        uint16_t hi = i686_inb(io + ATA_REG_DATA);
        /* Actually need to read 16-bit port; use inline asm */
        __asm__ volatile("inw %1, %0" : "=a"(buf[i]) : "Nd"((uint16_t)(io + ATA_REG_DATA)));
    }
}

/* Read a single 16-bit word from data port */
static inline uint16_t ata_inw(uint16_t port)
{
    uint16_t val;
    __asm__ volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

bool ATA_Initialize(ATA_Drive *drive)
{
    drive->io_base = ATA_PRIMARY_IO;
    drive->ctrl_base = ATA_PRIMARY_CTRL;
    drive->drive_select = ATA_MASTER;
    drive->present = false;
    drive->sector_count = 0;
    drive->model[0] = '\0';

    uint16_t io = drive->io_base;

    /* Select drive */
    i686_outb(io + ATA_REG_DRIVE_HEAD, drive->drive_select);

    /* Wait */
    for (int i = 0; i < 4; i++)
        i686_inb(io + ATA_REG_STATUS);

    /* Zero out sector count and LBA registers */
    i686_outb(io + ATA_REG_SEC_COUNT, 0);
    i686_outb(io + ATA_REG_LBA_LO, 0);
    i686_outb(io + ATA_REG_LBA_MID, 0);
    i686_outb(io + ATA_REG_LBA_HI, 0);

    /* Send IDENTIFY command */
    i686_outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    /* Read status */
    uint8_t status = i686_inb(io + ATA_REG_STATUS);
    if (status == 0) {
        /* No drive present */
        return false;
    }

    /* Wait for BSY to clear */
    ata_wait_bsy(io);

    /* Check for non-ATA */
    if (i686_inb(io + ATA_REG_LBA_MID) != 0 || i686_inb(io + ATA_REG_LBA_HI) != 0) {
        /* Not ATA (might be ATAPI) */
        return false;
    }

    /* Wait for DRQ or ERR */
    while (1) {
        status = i686_inb(io + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) return false;
        if (status & ATA_SR_DRQ) break;
    }

    /* Read IDENTIFY data (256 words = 512 bytes) */
    uint16_t identify[256];
    for (int i = 0; i < 256; i++)
        identify[i] = ata_inw(io + ATA_REG_DATA);

    /* Extract sector count (words 60-61 = 28-bit LBA sector count) */
    drive->sector_count = (uint32_t)identify[60] | ((uint32_t)identify[61] << 16);

    /* Extract model string (words 27-46, byte-swapped) */
    for (int i = 0; i < 20; i++) {
        drive->model[i * 2]     = (char)(identify[27 + i] >> 8);
        drive->model[i * 2 + 1] = (char)(identify[27 + i] & 0xFF);
    }
    drive->model[40] = '\0';

    /* Trim trailing spaces */
    for (int i = 39; i >= 0; i--) {
        if (drive->model[i] == ' ') drive->model[i] = '\0';
        else break;
    }

    drive->present = true;
    return true;
}

bool ATA_ReadSectors(ATA_Drive *drive, uint32_t lba, uint8_t count, void *buffer)
{
    if (!drive->present) return false;
    if (count == 0) return false;

    uint16_t io = drive->io_base;
    uint16_t *buf = (uint16_t *)buffer;

    /* Wait for drive ready */
    ata_wait_bsy(io);

    /* Select drive + top 4 bits of LBA */
    i686_outb(io + ATA_REG_DRIVE_HEAD,
              (drive->drive_select) | ((lba >> 24) & 0x0F));

    /* Sector count */
    i686_outb(io + ATA_REG_SEC_COUNT, count);

    /* LBA bytes */
    i686_outb(io + ATA_REG_LBA_LO,  (uint8_t)(lba & 0xFF));
    i686_outb(io + ATA_REG_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    i686_outb(io + ATA_REG_LBA_HI,  (uint8_t)((lba >> 16) & 0xFF));

    /* Send READ command */
    i686_outb(io + ATA_REG_COMMAND, ATA_CMD_READ);

    /* Read each sector */
    for (int s = 0; s < count; s++) {
        if (!ata_poll(io)) return false;

        /* Read 256 words (512 bytes) */
        for (int i = 0; i < 256; i++) {
            *buf++ = ata_inw(io + ATA_REG_DATA);
        }
    }

    return true;
}

void ATA_PrintInfo(ATA_Drive *drive)
{
    if (!drive->present) {
        puts("  No ATA drive detected.\n");
        return;
    }
    printf("  Model : %s\n", drive->model);
    printf("  Sectors: %u (%u MB)\n",
           drive->sector_count,
           drive->sector_count / 2048);
}