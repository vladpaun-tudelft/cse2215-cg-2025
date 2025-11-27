# FetchContent = CMake 3.11
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

add_executable(Bachelor_Practical1_1 "src/cpp/main.cpp")
target_compile_features(Bachelor_Practical1_1 PRIVATE cxx_std_20)
target_link_libraries(Bachelor_Practical1_1 PRIVATE CGFramework Catch2WithMain)
set_project_warnings(Bachelor_Practical1_1)
enable_sanitizers(Bachelor_Practical1_1)


add_executable(Bachelor_Practical1_2
	"src/linear/main.cpp"
	"src/linear/your_code_here.cpp"
	"src/linear/draw.cpp" 
)
target_compile_features(Bachelor_Practical1_2 PRIVATE cxx_std_20)
target_link_libraries(Bachelor_Practical1_2 PRIVATE CGFramework OpenGL::GLU)
enable_sanitizers(Bachelor_Practical1_2)
set_project_warnings(Bachelor_Practical1_2)
target_compile_definitions(Bachelor_Practical1_2 PRIVATE "DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"")

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/tests/")
	add_subdirectory("tests")
endif()	
