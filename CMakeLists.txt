#=============================================================================#
# Author: Konstantin Gredeskoul https://github.com/kigster                    #
# © 2017 All rights reserved, MIT License                                     #
#                                                                             #
# Description: Arduino CMake Top Level file that uses Arli Library Manager    #
# https://github.com/kigster/arli                                             #
#                                                                             #
# Depends on Arduino-CMake project found at                                   #
# https://github.com/arduino-cmake/arduino-cmake                              #
#                                                                             #
#=============================================================================#

set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
cmake_minimum_required(VERSION 2.8)

project(WeatherPod C CXX ASM)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_LOCAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)

# Build an example by uncommenting this line, and commenting out previous line
# add_subdirectory(example)
