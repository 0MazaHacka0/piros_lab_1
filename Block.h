//
// Created by atom on 13.04.2020.
//

#ifndef LABA_SEVR_BLOCK_H
#define LABA_SEVR_BLOCK_H

typedef long long int int64;

struct Block {
    int64 start;
    int64 size;
    int64 file_id;
};

void block_init(struct Block *block, int64 start, int64 size, int64 file_id);

int block_comparator(const void *p, const void *q);

#endif //LABA_SEVR_BLOCK_H
