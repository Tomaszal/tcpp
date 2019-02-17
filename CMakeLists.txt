cmake_minimum_required(VERSION 3.13)
project(tcpp C)

set(CMAKE_C_STANDARD 99)

add_executable(tcpp src/main.c src/args.c src/args.h)
