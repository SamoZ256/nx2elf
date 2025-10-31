cmake_minimum_required(VERSION 3.10)
project(nx2elf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nx2elf
    src/elf.h
    src/elf_eh.cpp
    src/elf_eh.h
    src/lz4.c
    src/lz4.h
    src/nx2elf.cpp
    src/nx2elf.h
    src/types.h
)
