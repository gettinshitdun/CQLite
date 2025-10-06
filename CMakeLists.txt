# Minimum version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(CQlite)

# Tell CMake where to find headers
include_directories(include)

# Collect all .c files from src folder
file(GLOB SOURCES "src/*.c")

include_directories(${PROJECT_SOURCE_DIR}/src/include)
# Build an executable called "bplustree" from these sources
add_executable(cqlite ${SOURCES})
