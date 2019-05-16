cmake_minimum_required(VERSION 3.13)
project(protectLibC C)

set(CMAKE_C_STANDARD 99)

add_library(protectLibC library.c library.h)