cmake_minimum_required(VERSION 3.1...3.27)

project(embedded-linux-class)

set(CMAKE_BUILD_TYPE Debug)

# Set the general output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Set up compiler options for g++ 
set(GCC_COMPILER gcc)

find_package(Threads REQUIRED)

add_subdirectory(lesson_0/assignment_solution)
add_subdirectory(lesson_1/code_examples)
add_subdirectory(lesson_2/code_examples)
add_subdirectory(lesson_3/code_examples)
add_subdirectory(lesson_4/code_examples)
add_subdirectory(lesson_5/code_examples)
add_subdirectory(lesson_6/code_examples)
add_subdirectory(lesson_7/code_examples)
add_subdirectory(lesson_8/code_examples)
