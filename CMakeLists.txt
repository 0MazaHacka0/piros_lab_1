cmake_minimum_required(VERSION 3.15)
project(Laba_Sevr C)

set(CMAKE_C_STANDARD 11)

add_executable(Laba_Sevr main.c Command.c Command.h Block.c Block.h File.c File.h Disk.c Disk.h)