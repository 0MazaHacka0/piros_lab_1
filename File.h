//
// Created by atom on 13.04.2020.
//

#ifndef LABA_SEVR_FILE_H
#define LABA_SEVR_FILE_H

typedef long long int int64;

struct File {
    int64 file_id;
    int64 block_last_id;
    int64 blocks_count;
    int64 *blocks_ids;
};

#endif //LABA_SEVR_FILE_H
