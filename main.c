#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include <stdio.h>

typedef long long int int64;

enum COMMANDS {
    WRITE  = 1,
    APPEND = 2,
    DELETE = 3,
};

struct Block {
    int64 start;
    int64 size;
    int64 file_id;
};

struct File {
    int64 file_id;
    int64 blocks_count;
    int64 *blocks_ids;
};

struct Disk {
    int64 size;

    int64 file_last_id;
    int64 files_count;
    struct File *files;

    int64 block_last_id;
    int64 blocks_count;
    struct Block *blocks;
};

struct Command {
    enum COMMANDS command;
    int64 file_id;
    int64 file_size;
};

void block_init(struct Block *block, int64 start, int64 size, int64 file_id)
{
    block->start = start;
    block->size = size;
    block->file_id = file_id;
}

void disk_init(struct Disk *disk, int64 size)
{
    disk->size = size;

    // Block init
    disk->block_last_id = 0;
    disk->blocks_count = 1;
    disk->blocks = (struct Block *) malloc(sizeof(struct Block));
    block_init(&disk->blocks[0], 0, size, -1);

    // Files init
    disk->file_last_id = 0;
    disk->files_count = 0;
}

void disk_erase(struct Disk *disk) {
    // Erase files
    disk->file_last_id = 0;
    disk->files_count = 0;
    free(disk->files);

    // Erase blocks
    disk->block_last_id = 0;
    disk->blocks_count = 1;
    free(disk->blocks);
    disk->blocks = (struct Block *) malloc(sizeof(struct Block));
    block_init(&disk->blocks[0], 0, disk->size, -1);
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

int block_comparator(const void *p, const void *q)
{
    int p_start = ((struct Block *) p)->start;
    int q_start = ((struct Block *) q)->start;
    return p_start - q_start;
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

    struct Block *new_blocks = (struct Block *) malloc(sizeof(struct Block) * (disk->blocks_count));
    for (int64 i = 0; i < blocks_count_old; ++i) {
        if (disk->blocks[i].size) {
            new_blocks[i] = disk->blocks[i];
        }
    }
    free(disk->blocks);
    disk->blocks = new_blocks;
}

void command_delete(struct Disk *disk, int64 file_id)
{
    struct File *file = NULL;
    if (!disk->files_count) {
        return;
    }

    for (int i = 0; i < disk->file_last_id; ++i) {
        if (disk->files[i].file_id == file_id) {
            file = disk->files;
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
    free(file->blocks_ids);
    file->blocks_count = 0;

    defragmentation(disk);
}

void command_append(struct Disk *disk, int64 file_id, int64 size,
                    int64 (*strategy) (int64, struct Block *, int64, int64))
{
    struct File *file = NULL;
    for (int i = 0; i < disk->file_last_id; ++i) {
        if (disk->files[i].file_id == file_id) {
            file = disk->files;
            break;
        }
    }

    if (file == NULL) {
        return;
    }

    while (size) {
        int64 block_id = strategy(size, disk->blocks, disk->block_last_id, disk->block_last_id);
        disk->blocks[block_id].file_id = file_id;

        if (disk->blocks[block_id].size == size) {
            disk->blocks[block_id].size = size;

            int64 *new_blocks = (int64 *) malloc(sizeof(int64) * (file->blocks_count + 1));
            for (int64 i = 0; i < file->blocks_count; ++i) {
                new_blocks[i] = file->blocks_ids[i];
            }
            file->blocks_count++;
            new_blocks[file->blocks_count] = block_id;
            file->blocks_ids = new_blocks;
        } else if (disk->blocks[block_id].size >= size) {
            int64 block_new_size = disk->blocks[block_id].size - size;

            disk->blocks[block_id].size = size;

            int64 *file_new_blocks = (int64 *) malloc(sizeof(int64) * (file->blocks_count + 1));
            for (int64 i = 0; i < file->blocks_count; ++i) {
                file_new_blocks[i] = file->blocks_ids[i];
            }
            if (file->blocks_count) {
                free(file->blocks_ids);
            }
            file->blocks_count++;
            file_new_blocks[file->blocks_count] = block_id;
            file->blocks_ids = file_new_blocks;

            // Create new block
            printf("Try to allocate %lld memory", sizeof(struct Block) * (disk->blocks_count + 1));
            struct Block *new_blocks = (struct Block *) malloc(sizeof(struct Block) * (disk->blocks_count + 1));
            for (int64 i = 0; i < disk->blocks_count; ++i) {
                if (disk->blocks[i].size) {
                    new_blocks[i] = disk->blocks[i];
                }
            }
            free(disk->blocks);
            disk->blocks = new_blocks;
            disk->block_last_id++;
            disk->blocks[disk->block_last_id].start = disk->blocks[block_id].start + disk->blocks[block_id].size;
            disk->blocks[disk->block_last_id].size = block_new_size;
            disk->blocks[disk->block_last_id].file_id = -1;
        }

        size -= disk->blocks[block_id].size;
    }
}

void command_write(struct Disk *disk, int64 file_id, int64 size,
        int64 (*strategy) (int64, struct Block *, int64, int64))
{
    command_delete(disk, file_id);

    if (disk->files_count < file_id) {
        struct File *new_files = (struct File *) malloc(sizeof(struct File) * (disk->files_count + 1));
        for (int64 i = 0; i < disk->files_count; ++i) {
            new_files[i] = disk->files[i];
        }
        if (disk->files_count) {
            free(disk->files);
        }
        disk->files = new_files;
        disk->files[disk->file_last_id].file_id = file_id;
        disk->files[disk->file_last_id].blocks_count = 0;
        disk->file_last_id++;
        disk->files_count++;
    }

    command_append(disk, file_id, size, strategy);
}

void command_execute(struct Disk *disk, enum COMMANDS command, int64 file_id, int64 file_size,
        int64 (*strategy) (int64, struct Block *, int64, int64))
{
    switch (command) {
        case WRITE:
            command_write(disk, file_id, file_size, strategy);
            printf("Executed write\n");
            break;
        case APPEND:
            command_append(disk, file_id, file_size, strategy);
            printf("Executed append\n");
            break;
        case DELETE:
            command_delete(disk, file_id);
            printf("Executed delete\n");
            break;
    }
}

int main() {

    struct Disk disk;
    int64 disk_size = 1000000;

    struct Command *commands;
    int64 commands_count;

    FILE *input = fopen("commands", "r");
    if (input == NULL) {
        printf("Error while opening commands file. Abort\n");
        return 1;
    }

    disk_init(&disk, disk_size);
    printf("Disk initialized with %lld size\n", disk_size);

    // Commands
    fscanf(input, "%lld", &commands_count);
    commands = malloc(sizeof(struct Command) * commands_count);
    printf("Initialized with %lld commands\n", commands_count);

    for (int64 i = 0; i < commands_count; ++i) {
        int64 command, file_id, file_size;
        fscanf(input, "%lld %lld %lld", &command, &file_id, &file_size);

        commands[i].command = command;
        commands[i].file_id = file_id;
        commands[i].file_size = file_size;
    }

    printf("Using next fit\n");
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, next_fit);
    }

    printf("Erasing disk\n");
    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, best_fit);
    }

    printf("Erasing disk\n");
    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, worst_fit);
    }

    printf("Erasing disk\n");
    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, first_fit);
    }

    return 0;
}
