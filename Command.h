//
// Created by atom on 12.04.2020.
//
#include <stdlib.h>
#include <stdio.h>

#include "Disk.h"

#ifndef LABA_SEVR_COMMAND_H
#define LABA_SEVR_COMMAND_H

typedef long long int int64;

enum COMMANDS {
    WRITE  = 1,
    APPEND = 2,
    DELETE = 3,
};

struct Command {
    enum COMMANDS command;
    int64 file_id;
    int64 file_size;
};

int64 read_commands(FILE *input, struct Command **commands_list);

void command_delete(struct Disk *disk, int64 file_id);

void command_append(struct Disk *disk, int64 file_id, int64 size,
                    int64 (*strategy) (int64, struct Block *, int64, int64));

void command_write(struct Disk *disk, int64 file_id, int64 size,
                   int64 (*strategy) (int64, struct Block *, int64, int64));

void command_execute(struct Disk *disk, enum COMMANDS command, int64 file_id, int64 file_size,
                     int64 (*strategy) (int64, struct Block *, int64, int64));

#endif //LABA_SEVR_COMMAND_H
