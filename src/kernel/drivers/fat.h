#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ata.h"

/* ============================================================
   OmniOS - Kernel FAT32 Reader (read-only)
   ============================================================ */

#define KFAT_SECTOR_SIZE      512
#define KFAT_MAX_FILES        8
#define KFAT_MAX_PATH         128
#define KFAT_CACHE_SECTORS    4

typedef struct {
    uint8_t  Name[11];
    uint8_t  Attributes;
    uint8_t  _Reserved;
    uint8_t  CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) KFAT_DirEntry;

#define KFAT_ATTR_READ_ONLY  0x01
#define KFAT_ATTR_HIDDEN     0x02
#define KFAT_ATTR_SYSTEM     0x04
#define KFAT_ATTR_VOLUME_ID  0x08
#define KFAT_ATTR_DIRECTORY  0x10
#define KFAT_ATTR_ARCHIVE    0x20
#define KFAT_ATTR_LFN        0x0F

typedef struct {
    bool     is_open;
    bool     is_dir;
    uint32_t size;
    uint32_t position;
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t sector_in_cluster;
    uint8_t  buffer[KFAT_SECTOR_SIZE];
} KFAT_File;

/* Initialise FAT32 driver using ATA drive, partition starts at lba_offset */
bool KFAT_Initialize(ATA_Drive *drive, uint32_t lba_offset);

/* List directory entries at given path (prints to VGA) */
bool KFAT_ListDir(const char *path);

/* Read file contents into buffer, returns bytes read */
int  KFAT_ReadFile(const char *path, void *buffer, uint32_t max_size);

/* Print file contents to VGA (cat command) */
bool KFAT_CatFile(const char *path);

/* Check if path exists */
bool KFAT_Exists(const char *path);

/* Check if path is directory */
bool KFAT_IsDir(const char *path);