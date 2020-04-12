//
// Created by atom on 13.04.2020.
//
#include <stdlib.h>
#include "File.h"
#include "Block.h"
#include <limits.h>


#ifndef LABA_SEVR_DISK_H
#define LABA_SEVR_DISK_H

typedef long long int int64;

struct Disk {
    int64 size;

    int64 file_last_id;
    int64 files_count;
    struct File *files;

    int64 block_last_id;
    int64 blocks_count;
    struct Block *blocks;
};

void disk_init(struct Disk *disk, int64 size);

void disk_erase(struct Disk *disk);

void fragmentation(struct Disk *disk);

void defragmentation(struct Disk *disk);

int64 first_fit(int64 size, struct Block *blocks, int64 block_count, int64 block_last_id);

int64 next_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id);

int64 best_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id);

int64 worst_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id);

#endif //LABA_SEVR_DISK_H
