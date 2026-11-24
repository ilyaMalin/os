cmake_minimum_required(VERSION 3.2.0)

project(main LANGUAGES C)

add_executable(main
    src/block.c
    src/file.c
    src/main.c
    src/fileSystem.c
)

target_include_directories(main PRIVATE include)
