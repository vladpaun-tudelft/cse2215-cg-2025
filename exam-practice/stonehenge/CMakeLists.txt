cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(ComputerGraphics C CXX)


if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
	add_subdirectory("../../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

add_executable(Bachelor_ProgrammingExam_Stonehenge "src/solution.cpp")
target_compile_features(Bachelor_ProgrammingExam_Stonehenge PRIVATE cxx_std_20)
# Link to OpenGL, and Microsoft-GSL and/or make their header files available.
target_link_libraries(Bachelor_ProgrammingExam_Stonehenge PRIVATE CGFramework)
enable_sanitizers(Bachelor_ProgrammingExam_Stonehenge)
set_project_warnings(Bachelor_ProgrammingExam_Stonehenge)

# if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/grading_tests")
# 	add_subdirectory("grading_tests")
# endif()
