cmake_minimum_required(VERSION 3.1...3.27)

set(CMAKE_BUILD_TYPE Debug)

# Set the project name to match the subdirectory
project(${CMAKE_CURRENT_SOURCE_DIR})

# # Add all cpp files in the current subdirectory to a list
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if(SOURCES)
    add_executable(hello_world ${SOURCES})
else()
    message(FATAL_ERROR "No source files found.")
endif()