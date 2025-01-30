cmake_minimum_required(VERSION 3.27)
project(faloSteg)

set(CMAKE_CXX_STANDARD 20)

add_executable(faloSteg main.cpp
        image.h
        image.cpp
)
