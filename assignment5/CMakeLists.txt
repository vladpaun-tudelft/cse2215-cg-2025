cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(ComputerGraphics C CXX)
enable_testing()

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
    # Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
    add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
    add_subdirectory("../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

add_subdirectory("src")
add_subdirectory("tests")
