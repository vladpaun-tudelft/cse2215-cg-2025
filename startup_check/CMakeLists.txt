cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(ComputerGraphics C CXX)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
	add_subdirectory("../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

add_executable(Bachelor_StartUpCheck "src/main.cpp")
target_compile_features(Bachelor_StartUpCheck PRIVATE cxx_std_20)
target_link_libraries(Bachelor_StartUpCheck PRIVATE CGFramework)
set_project_warnings(Bachelor_StartUpCheck)
enable_sanitizers(Bachelor_StartUpCheck)
