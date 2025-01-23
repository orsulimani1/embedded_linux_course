cmake_minimum_required(VERSION 3.1...3.27)

project(embedded-linux-class)

set(CMAKE_BUILD_TYPE Debug)

# Set the general output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Set up compiler options for g++ 
set(GCC_COMPILER gcc)

find_package(Threads REQUIRED)
