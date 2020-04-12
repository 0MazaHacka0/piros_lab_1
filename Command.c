//
// Created by atom on 12.04.2020.
//

#include "Command.h"

int64 read_commands(FILE *input, struct Command **commands_list)
{
    int64 commands_count;
    fscanf(input, "%lld", &commands_count);
    (*commands_list) = calloc(sizeof(struct Command), commands_count);

    for (int64 i = 0; i < commands_count; ++i) {
        int64 command, file_id, file_size;
        fscanf(input, "%lld %lld %lld", &command, &file_id, &file_size);

        (*commands_list)[i].command = command;
        (*commands_list)[i].file_id = file_id;
        (*commands_list)[i].file_size = file_size;
    }

    return commands_count;
}

void command_delete(struct Disk *disk, int64 file_id)
{
    struct File *file = NULL;
    if (!disk->files_count) {
        return;
    }

    for (int i = 0; i < disk->files_count; ++i) {
        if (disk->files[i].file_id == file_id) {
            file = &disk->files[i];
            break;
        }
    }

    if (file == NULL) {
        return;
    }

    // Erase block from file
    for (int i = 0; i < file->blocks_count; ++i) {
        disk->blocks[file->blocks_ids[i]].file_id = -1;
    }
    if (file->blocks_count) {
        free(file->blocks_ids);
    }
    file->block_last_id = 0;
    file->blocks_count = 0;

//     defragmentation(disk);
}

void command_append(struct Disk *disk, int64 file_id, int64 size,
                    int64 (*strategy) (int64, struct Block *, int64, int64))
{
    struct File *file = NULL;
    for (int i = 0; i < disk->files_count; ++i) {
        if (disk->files[i].file_id == file_id) {
            file = &disk->files[i];
            break;
        }
    }

    if (file == NULL) {
        return;
    }

    while (size > 0) {
        int64 block_id = strategy(size, disk->blocks, disk->block_last_id, disk->block_last_id);
        disk->blocks[block_id].file_id = file_id;

        if (file->blocks_count) {
            file->blocks_ids = realloc(file->blocks_ids, sizeof(int64) * (file->blocks_count + 1));
        } else {
            file->blocks_ids = calloc(sizeof(int64), 1);
        }
        file->block_last_id++;
        file->blocks_count++;
        file->blocks_ids[file->block_last_id] = block_id;

        if (disk->blocks[block_id].size > size) {
            int64 block_new_size = disk->blocks[block_id].size - size;
            disk->blocks[block_id].size = size;

            // Create new block
            disk->blocks = realloc(disk->blocks, sizeof(struct Block) * (disk->blocks_count + 1));

            disk->blocks[disk->block_last_id].start = disk->blocks[block_id].start + disk->blocks[block_id].size;
            disk->blocks[disk->block_last_id].size = block_new_size;
            disk->blocks[disk->block_last_id].file_id = -1;
            disk->blocks_count++;
            disk->block_last_id++;
        }

        size -= disk->blocks[block_id].size;
    }
}

void command_write(struct Disk *disk, int64 file_id, int64 size,
                   int64 (*strategy) (int64, struct Block *, int64, int64))
{
    command_delete(disk, file_id);

    if (disk->files_count < file_id || disk->files_count == 0) {
        if (disk->files_count) {
            disk->files = realloc(disk->files, sizeof(struct File) * (disk->files_count + 1));
        } else {
            disk->files = calloc(sizeof(struct File), (disk->files_count + 1));
            disk->file_last_id = -1;
        }

        disk->file_last_id++;
        disk->files[disk->file_last_id].file_id = file_id;
        disk->files[disk->file_last_id].blocks_count = 0;
        disk->files[disk->file_last_id].block_last_id = -1;
        disk->files_count++;
    }

    command_append(disk, file_id, size, strategy);
}

void command_execute(struct Disk *disk, enum COMMANDS command, int64 file_id, int64 file_size,
                     int64 (*strategy) (int64, struct Block *, int64, int64))
{
    switch (command) {
        case WRITE:
//            printf("Execute write\n");
            command_write(disk, file_id, file_size, strategy);
            break;
        case APPEND:
//            printf("Execute append\n");
            command_append(disk, file_id, file_size, strategy);
            break;
        case DELETE:
//            printf("Execute delete\n");
            command_delete(disk, file_id);
            break;
    }
}
