cmake_minimum_required(VERSION 3.13)
project(protectLib)

set(CMAKE_CXX_STANDARD 14)

add_library(protectLib library.cpp library.h)

set_target_properties(protectLib PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")