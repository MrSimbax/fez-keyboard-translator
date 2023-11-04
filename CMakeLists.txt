cmake_minimum_required(VERSION 3.27)

project("fez-keyboard-translator" VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Embedded dependencies
add_subdirectory("libs")

# Project files
add_subdirectory("src")
