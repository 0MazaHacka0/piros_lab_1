//
// Created by atom on 13.04.2020.
//

#include "Block.h"

void block_init(struct Block *block, int64 start, int64 size, int64 file_id)
{
    block->start = start;
    block->size = size;
    block->file_id = file_id;
}

int block_comparator(const void *p, const void *q)
{
    int p_start = ((struct Block *) p)->start;
    int q_start = ((struct Block *) q)->start;
    return p_start - q_start;
}
