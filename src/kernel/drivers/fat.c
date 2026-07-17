/* ============================================================
   OmniOS - Kernel FAT32 Reader (read-only)
   Works with ATA PIO driver in protected mode.
   ============================================================ */
#include "fat.h"
#include "ata.h"
#include "../stdio.h"
#include "../arch/i686/vga_text.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- BPB structures ---- */
typedef struct {
    uint8_t  jump[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    /* FAT32 extended */
    uint32_t sectors_per_fat_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved2[12];
    uint8_t  drive_number;
    uint8_t  reserved3;
    uint8_t  boot_sig;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed)) FAT32_BPB;

/* ---- Global state ---- */
static ATA_Drive   *g_drive = NULL;
static uint32_t     g_part_offset = 0;
static FAT32_BPB    g_bpb;
static uint32_t     g_fat_lba;
static uint32_t     g_data_lba;
static uint32_t     g_sectors_per_fat;
static uint8_t      g_sectors_per_cluster;
static bool         g_initialized = false;

/* Sector buffer for reading */
static uint8_t      g_sector_buf[KFAT_SECTOR_SIZE];
static uint8_t      g_fat_buf[KFAT_CACHE_SECTORS * KFAT_SECTOR_SIZE];
static uint32_t     g_fat_buf_start = 0xFFFFFFFF;

/* ---- Helpers ---- */
static bool read_sector(uint32_t lba, void *buf)
{
    return ATA_ReadSectors(g_drive, g_part_offset + lba, 1, buf);
}

static bool read_sectors(uint32_t lba, uint8_t count, void *buf)
{
    return ATA_ReadSectors(g_drive, g_part_offset + lba, count, buf);
}

static uint32_t cluster_to_lba(uint32_t cluster)
{
    return g_data_lba + (cluster - 2) * g_sectors_per_cluster;
}

static uint32_t next_cluster(uint32_t cluster)
{
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_offset / KFAT_SECTOR_SIZE;
    uint32_t fat_entry_offset = fat_offset % KFAT_SECTOR_SIZE;

    /* Cache FAT sectors */
    if (fat_sector < g_fat_buf_start ||
        fat_sector >= g_fat_buf_start + KFAT_CACHE_SECTORS) {
        g_fat_buf_start = fat_sector;
        uint8_t count = KFAT_CACHE_SECTORS;
        if (fat_sector + count > g_sectors_per_fat)
            count = g_sectors_per_fat - fat_sector;
        read_sectors(g_fat_lba + fat_sector, count, g_fat_buf);
    }

    uint32_t entry_offset = (fat_sector - g_fat_buf_start) * KFAT_SECTOR_SIZE + fat_entry_offset;
    uint32_t val = *(uint32_t *)(g_fat_buf + entry_offset);
    return val & 0x0FFFFFFF;
}

static bool is_end_of_chain(uint32_t cluster)
{
    return cluster >= 0x0FFFFFF8;
}

/* ---- memcmp / toupper ---- */
static int fat_memcmp(const void *a, const void *b, int n)
{
    const uint8_t *pa = a, *pb = b;
    for (int i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}

static char fat_toupper(char c)
{
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

static void fat_make_short_name(const char *name, char out[11])
{
    for (int i = 0; i < 11; i++) out[i] = ' ';

    const char *dot = NULL;
    for (const char *p = name; *p; p++) {
        if (*p == '.') dot = p;
    }
    if (dot == NULL) dot = name + 128; /* no extension */

    int i = 0;
    for (const char *p = name; p < dot && i < 8; p++, i++)
        out[i] = fat_toupper(*p);

    if (*dot == '.') {
        dot++;
        for (int j = 0; j < 3 && dot[j]; j++)
            out[8 + j] = fat_toupper(dot[j]);
    }
}

/* ---- Directory entry name formatting ---- */
static void format_entry_name(KFAT_DirEntry *entry, char *out)
{
    int pos = 0;

    /* Base name (8 chars, trim trailing spaces) */
    for (int i = 0; i < 8; i++) {
        if (entry->Name[i] != ' ')
            out[pos++] = entry->Name[i];
    }

    /* Extension (3 chars) */
    if (entry->Name[8] != ' ') {
        if (!(entry->Attributes & KFAT_ATTR_DIRECTORY))
            out[pos++] = '.';
        for (int i = 8; i < 11; i++) {
            if (entry->Name[i] != ' ')
                out[pos++] = entry->Name[i];
        }
    }

    out[pos] = '\0';

    /* Lowercase for display */
    for (int i = 0; out[i]; i++) {
        if (out[i] >= 'A' && out[i] <= 'Z')
            out[i] += 32;
    }
}

/* ---- Read a cluster into a buffer ---- */
static bool read_cluster(uint32_t cluster, uint8_t *buf)
{
    uint32_t lba = cluster_to_lba(cluster);
    for (int i = 0; i < g_sectors_per_cluster; i++) {
        if (!read_sector(lba + i, buf + i * KFAT_SECTOR_SIZE))
            return false;
    }
    return true;
}

/* ---- Initialise ---- */
bool KFAT_Initialize(ATA_Drive *drive, uint32_t lba_offset)
{
    g_drive = drive;
    g_part_offset = lba_offset;
    g_initialized = false;
    g_fat_buf_start = 0xFFFFFFFF;

    if (!drive->present) return false;

    /* Read BPB (sector 0 of partition) */
    if (!read_sector(0, g_sector_buf)) return false;

    /* Copy BPB */
    const FAT32_BPB *bpb = (const FAT32_BPB *)g_sector_buf;
    g_bpb = *bpb;

    /* Validate */
    if (g_bpb.bytes_per_sector != 512) return false;

    g_sectors_per_cluster = g_bpb.sectors_per_cluster;

    /* FAT32: sectors_per_fat_16 == 0 */
    g_sectors_per_fat = g_bpb.sectors_per_fat_16;
    if (g_sectors_per_fat == 0)
        g_sectors_per_fat = g_bpb.sectors_per_fat_32;

    g_fat_lba = g_bpb.reserved_sectors;
    g_data_lba = g_fat_lba + g_bpb.fat_count * g_sectors_per_fat;

    g_initialized = true;
    return true;
}

/* ---- Find entry in directory cluster chain ---- */
static bool find_in_dir(uint32_t dir_cluster, const char *name, KFAT_DirEntry *out)
{
    char short_name[11];
    fat_make_short_name(name, short_name);

    uint32_t cluster = dir_cluster;
    uint32_t cluster_size = g_sectors_per_cluster * KFAT_SECTOR_SIZE;

    /* Use sector buffer to read one sector at a time */
    while (!is_end_of_chain(cluster)) {
        uint32_t lba = cluster_to_lba(cluster);

        for (int sec = 0; sec < g_sectors_per_cluster; sec++) {
            if (!read_sector(lba + sec, g_sector_buf)) return false;

            KFAT_DirEntry *entries = (KFAT_DirEntry *)g_sector_buf;
            int entries_per_sector = KFAT_SECTOR_SIZE / sizeof(KFAT_DirEntry);

            for (int i = 0; i < entries_per_sector; i++) {
                if (entries[i].Name[0] == 0x00) return false; /* end */
                if (entries[i].Name[0] == 0xE5) continue;     /* deleted */
                if (entries[i].Attributes == KFAT_ATTR_LFN) continue;

                if (fat_memcmp(entries[i].Name, short_name, 11) == 0) {
                    *out = entries[i];
                    return true;
                }
            }
        }

        cluster = next_cluster(cluster);
    }

    return false;
}

/* ---- Resolve path to cluster + entry ---- */
static bool resolve_path(const char *path, uint32_t *out_cluster, KFAT_DirEntry *out_entry, bool *is_root)
{
    if (!g_initialized) return false;

    *is_root = false;

    /* Skip leading slash */
    while (*path == '/') path++;

    /* Empty path = root */
    if (*path == '\0') {
        *out_cluster = g_bpb.root_cluster;
        *is_root = true;
        return true;
    }

    uint32_t current_cluster = g_bpb.root_cluster;

    while (*path) {
        /* Extract next path component */
        char component[64];
        int len = 0;
        while (*path && *path != '/' && len < 63) {
            component[len++] = *path++;
        }
        component[len] = '\0';
        while (*path == '/') path++;

        /* Find in current directory */
        KFAT_DirEntry entry;
        if (!find_in_dir(current_cluster, component, &entry))
            return false;

        if (*path != '\0') {
            /* More path components — must be directory */
            if (!(entry.Attributes & KFAT_ATTR_DIRECTORY))
                return false;
            current_cluster = entry.FirstClusterLow |
                              ((uint32_t)entry.FirstClusterHigh << 16);
        } else {
            /* Last component */
            *out_entry = entry;
            *out_cluster = entry.FirstClusterLow |
                           ((uint32_t)entry.FirstClusterHigh << 16);
            return true;
        }
    }

    return false;
}

/* ---- Public: ls ---- */
bool KFAT_ListDir(const char *path)
{
    if (!g_initialized) {
        puts("  Filesystem not mounted.\n");
        return false;
    }

    uint32_t dir_cluster;
    KFAT_DirEntry entry;
    bool is_root;

    if (path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        dir_cluster = g_bpb.root_cluster;
        is_root = true;
    } else {
        if (!resolve_path(path, &dir_cluster, &entry, &is_root)) {
            printf("  Path not found: %s\n", path);
            return false;
        }
        if (!is_root && !(entry.Attributes & KFAT_ATTR_DIRECTORY)) {
            printf("  Not a directory: %s\n", path);
            return false;
        }
    }

    /* Read directory entries */
    uint32_t cluster = dir_cluster;
    int file_count = 0, dir_count = 0;
    uint32_t total_size = 0;

    VGA_SetColor(VGA_COL_INFO);
    printf("  %-14s %8s  %s\n", "NAME", "SIZE", "TYPE");
    VGA_SetColor(VGA_COL_NORMAL);
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);

    while (!is_end_of_chain(cluster)) {
        uint32_t lba = cluster_to_lba(cluster);

        for (int sec = 0; sec < g_sectors_per_cluster; sec++) {
            if (!read_sector(lba + sec, g_sector_buf)) return false;

            KFAT_DirEntry *entries = (KFAT_DirEntry *)g_sector_buf;
            int entries_per_sector = KFAT_SECTOR_SIZE / sizeof(KFAT_DirEntry);

            for (int i = 0; i < entries_per_sector; i++) {
                if (entries[i].Name[0] == 0x00) goto done;
                if (entries[i].Name[0] == 0xE5) continue;
                if (entries[i].Attributes == KFAT_ATTR_LFN) continue;
                if (entries[i].Attributes & KFAT_ATTR_VOLUME_ID) continue;

                char name[13];
                format_entry_name(&entries[i], name);

                if (entries[i].Attributes & KFAT_ATTR_DIRECTORY) {
                    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
                    printf("  %-14s %8s  <DIR>\n", name, "");
                    dir_count++;
                } else {
                    VGA_SetColor(VGA_COL_NORMAL);
                    printf("  %-14s %8u  file\n", name, entries[i].Size);
                    file_count++;
                    total_size += entries[i].Size;
                }
            }
        }

        cluster = next_cluster(cluster);
    }

done:
    VGA_SetColor(VGA_COL_NORMAL);
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    VGA_SetColor(VGA_COL_INFO);
    printf("  %d file(s)  %d dir(s)  %u bytes total\n", file_count, dir_count, total_size);
    VGA_SetColor(VGA_COL_NORMAL);
    return true;
}

/* ---- Public: cat ---- */
bool KFAT_CatFile(const char *path)
{
    if (!g_initialized) {
        puts("  Filesystem not mounted.\n");
        return false;
    }

    uint32_t file_cluster;
    KFAT_DirEntry entry;
    bool is_root;

    if (!resolve_path(path, &file_cluster, &entry, &is_root)) {
        printf("  File not found: %s\n", path);
        return false;
    }

    if (entry.Attributes & KFAT_ATTR_DIRECTORY) {
        printf("  '%s' is a directory. Use 'ls' instead.\n", path);
        return false;
    }

    uint32_t remaining = entry.Size;
    uint32_t cluster = file_cluster;

    while (!is_end_of_chain(cluster) && remaining > 0) {
        uint32_t lba = cluster_to_lba(cluster);

        for (int sec = 0; sec < g_sectors_per_cluster && remaining > 0; sec++) {
            if (!read_sector(lba + sec, g_sector_buf)) return false;

            uint32_t to_print = remaining;
            if (to_print > KFAT_SECTOR_SIZE)
                to_print = KFAT_SECTOR_SIZE;

            for (uint32_t i = 0; i < to_print; i++) {
                char c = (char)g_sector_buf[i];
                if (c == '\r') continue;
                if (c >= 0x20 || c == '\n' || c == '\t')
                    putc(c);
            }

            remaining -= to_print;
        }

        cluster = next_cluster(cluster);
    }

    putc('\n');
    return true;
}

/* ---- Public: read file into buffer ---- */
int KFAT_ReadFile(const char *path, void *buffer, uint32_t max_size)
{
    if (!g_initialized) return -1;

    uint32_t file_cluster;
    KFAT_DirEntry entry;
    bool is_root;

    if (!resolve_path(path, &file_cluster, &entry, &is_root))
        return -1;

    if (entry.Attributes & KFAT_ATTR_DIRECTORY)
        return -1;

    uint32_t remaining = entry.Size;
    if (remaining > max_size) remaining = max_size;

    uint32_t cluster = file_cluster;
    uint8_t *out = (uint8_t *)buffer;
    uint32_t total = 0;

    while (!is_end_of_chain(cluster) && remaining > 0) {
        uint32_t lba = cluster_to_lba(cluster);

        for (int sec = 0; sec < g_sectors_per_cluster && remaining > 0; sec++) {
            if (!read_sector(lba + sec, g_sector_buf)) return total;

            uint32_t take = remaining;
            if (take > KFAT_SECTOR_SIZE) take = KFAT_SECTOR_SIZE;

            for (uint32_t i = 0; i < take; i++)
                out[total + i] = g_sector_buf[i];

            total += take;
            remaining -= take;
        }

        cluster = next_cluster(cluster);
    }

    return total;
}

bool KFAT_Exists(const char *path)
{
    uint32_t c;
    KFAT_DirEntry e;
    bool is_root;
    return resolve_path(path, &c, &e, &is_root);
}

bool KFAT_IsDir(const char *path)
{
    uint32_t c;
    KFAT_DirEntry e;
    bool is_root;
    if (!resolve_path(path, &c, &e, &is_root)) return false;
    if (is_root) return true;
    return (e.Attributes & KFAT_ATTR_DIRECTORY) != 0;
}