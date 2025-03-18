#!/usr/bin/env bash

# Check if the user provided a lesson number
if [ -z "$1" ]; then
  echo "Usage: $0 <lesson_number>"
  exit 1
fi

# Read the lesson number from the first argument
LESSON_NUM=$1

# Define the new lesson directory name
LESSON_DIR="lesson_${LESSON_NUM}"

# Create the lesson directory and subdirectories
mkdir -p "${LESSON_DIR}/assignments"
mkdir -p "${LESSON_DIR}/code_examples"

# Create (or overwrite) the assignment file
touch "${LESSON_DIR}/assignments/assignment.txt"

# Copy the CMakeLists.txt from lesson_5/codeexamples to the new directory
cp lesson_5/code_examples/CMakeLists.txt "${LESSON_DIR}/code_examples"

sed -i "s/lesson_5/${LESSON_DIR}/g" "${LESSON_DIR}/code_examples/CMakeLists.txt"
echo "add_subdirectory(${LESSON_DIR}/code_examples)" >> CMakeLists.txt


echo "Created ${LESSON_DIR}/ with assignments/assignment.txt and code_examples/CMakeLists.txt"
echo "Appended add_subdirectory(${LESSON_DIR}/code_examples) to the master CMakeLists.txt"