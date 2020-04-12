//
// Created by atom on 13.04.2020.
//

#include "Disk.h"

void disk_init(struct Disk *disk, int64 size)
{
    disk->size = size;

    // Block init
    disk->block_last_id = 0;
    disk->blocks_count = 1;
    disk->blocks = calloc(sizeof(struct Block), 1);
    block_init(&disk->blocks[disk->block_last_id], 0, size, -1);

    // Files init
    disk->file_last_id = -1;
    disk->files_count = 0;
}

void disk_erase(struct Disk *disk) {
    // Erase files
    if (disk->files_count) {
        free(disk->files);
    }

    // Erase blocks
    if (disk->blocks_count) {
        free(disk->blocks);
    }

    disk_init(disk, disk->size);
}

void fragmentation(struct Disk *disk)
{

}

void defragmentation(struct Disk *disk)
{
    qsort(disk->blocks, sizeof(struct Block) * disk->blocks_count, sizeof(struct Block), block_comparator);

    int64 count_zero_blocks = 0;
    for (int64 i = 0; i < disk->blocks_count - 1; ++i) {
        struct Block current_block = disk->blocks[i];
        struct Block next_block = disk->blocks[i + 1];

        if ((current_block.start + current_block.size) == next_block.start
            && current_block.file_id == -1 && next_block.file_id == -1)
        {
            disk->blocks[i].size += disk->blocks[i + 1].size;
            disk->blocks[i + 1].size = 0;
            ++count_zero_blocks;
        }
    }

    int64 blocks_count_old = disk->blocks_count;
    disk->block_last_id -= count_zero_blocks;
    disk->blocks_count -= count_zero_blocks;

    struct Block *new_blocks = calloc(sizeof(struct Block), (disk->blocks_count));
    int64 j = 0;
    for (int64 i = 0; i < blocks_count_old; ++i) {
        if (disk->blocks[i].size) {
            new_blocks[j] = disk->blocks[i];
            ++j;
        }
    }
    if (disk->blocks_count) {
        free(disk->blocks);
    }
    disk->blocks = new_blocks;
}

int64 first_fit(int64 size, struct Block *blocks, int64 block_count, int64 block_last_id)
{
    for (int64 i = 0; i < block_count; ++i) {
        if (blocks[i].size >= size) {
            return i;
        }
    }
    return 0;
}

int64 next_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id)
{
    if (blocks_count == 0) {
        return 0;
    }

    for (int64 i = 0; i < blocks_count; ++i) {
        int64 block_id = (i + block_last_id) % blocks_count;
        if (blocks[block_id].size >= size) {
            return block_id;
        }
    }
    return (block_last_id * 2) % blocks_count;
}

int64 best_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id)
{
    int64 best_block_id = -1;
    int64 best_block_size = LLONG_MAX;

    int64 max_block_id = 0;
    int64 max_block_size = blocks[0].size;

    for (int64 i = 0; i < blocks_count; ++i) {
        if (blocks[i].size > max_block_size) {
            max_block_size = blocks[i].size;
            max_block_id = i;
        }
        if (blocks[i].size >= size && blocks[i].size < best_block_size) {
            best_block_id = i;
            best_block_size = blocks[i].size;
        }
    }

    if (best_block_id != -1) {
        return best_block_id;
    }
    return max_block_id;
}

int64 worst_fit(int64 size, struct Block *blocks, int64 blocks_count, int64 block_last_id)
{
    int64 max_block_id = 0;
    int64 max_block_size = blocks[0].size;

    for (int64 i = 0; i < blocks_count; ++i) {
        if (blocks[i].size > max_block_size) {
            max_block_size = blocks[i].size;
            max_block_id = i;
        }
    }

    return max_block_id;
}
