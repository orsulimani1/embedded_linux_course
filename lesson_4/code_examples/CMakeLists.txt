

function(setup_compiler target_prefix compiler source_dir)
    set(CMAKE_CXX_COMPILER ${compiler})

    # Find all .c/.cpp files in the source directory
    file(GLOB_RECURSE SOURCES "${source_dir}/*.[ch]pp" "${source_dir}/*.c")

    message(STATUS "            ${SOURCES} ${TARGET_NAME} using ${compiler}")

    list(LENGTH SOURCES SOURCE_COUNT)
    if(SOURCE_COUNT EQUAL 0)
        message(STATUS "No source files found in ${source_dir}")
        return()
    endif()

    foreach(SOURCE ${SOURCES})
        # Get the filename without the extension
        get_filename_component(FILE_NAME ${SOURCE} NAME_WE)
        get_filename_component(DIR_NAME ${SOURCE} DIRECTORY)
        get_filename_component(PARENT_NAME ${DIR_NAME} NAME)

        # Define a unique target name
        set(TARGET_NAME "${FILE_NAME}")
        # Create an executable for each .cpp file
        add_executable(${TARGET_NAME} ${SOURCE})

        set_target_properties(${TARGET_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/lesson_4/${PARENT_NAME}"
        )

        message(STATUS "Created target: ${TARGET_NAME} using ${compiler}")
    endforeach()
endfunction()

# Read ignore directories from ignore_dirs.txt
set(IGNORE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ignore_dirs.txt)
if(EXISTS ${IGNORE_FILE})
    file(READ ${IGNORE_FILE} IGNORE_CONTENTS)
    string(REGEX REPLACE "\n" ";" IGNORE_LIST "${IGNORE_CONTENTS}")
else()
    set(IGNORE_LIST "")
endif()

message(STATUS "    Skipping ignored directory: ${IGNORE_LIST}")


file(GLOB FIRST_LAYER_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

set(GCC_COMPILER g++)

foreach(FIRST_LAYER_DIR ${FIRST_LAYER_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${FIRST_LAYER_DIR})
        # Check if the directory is in the ignore list
        list(FIND IGNORE_LIST ${FIRST_LAYER_DIR} IGNORE_INDEX)
        if(IGNORE_INDEX EQUAL -1)
            set(TARGET_NAME "${FIRST_LAYER_DIR}")
            setup_compiler(${TARGET_NAME} ${GCC_COMPILER} ${FIRST_LAYER_DIR})
            # setup_compiler(${TARGET_NAME} ${CLANG_COMPILER} ${FIRST_LAYER_DIR})
        else()
            message(STATUS "Skipping ignored directory: ${FIRST_LAYER_DIR}")
        endif()
    endif()
endforeach()