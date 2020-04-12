#include <string.h>

#include <stdio.h>

#include "Command.h"
#include "Block.h"
#include "File.h"
#include "Disk.h"

typedef long long int int64;


int main() {

    struct Disk disk;
    int64 disk_size = 1000000;

    struct Command *commands = NULL;
    int64 commands_count;

    FILE *input = fopen("commands", "r");
    if (input == NULL) {
        printf("Error while opening commands file. Abort\n");
        return 1;
    }

    disk_init(&disk, disk_size);

    // Read Commands
    commands_count = read_commands(input, &commands);

    // Calculate
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, next_fit);
    }

    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, best_fit);
    }

    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, worst_fit);
    }

    disk_erase(&disk);
    for (int64 i = 0; i < commands_count; ++i) {
        command_execute(&disk, commands[i].command, commands[i].file_id, commands[i].file_size, first_fit);
    }

    return 0;
}
