cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(ComputerGraphics C CXX)
enable_testing()

option(USE_PREBUILT_REFERENCE_P3 "Use the prebuilt libraries for the reference solutions" ON)
option(MAKE_PREBUILT_REFERENCE_P3 "Generate prebuilt libraries for the reference solutions (students please don't turn on this flag)" OFF)

if (MAKE_PREBUILT_REFERENCE_P3)
    if (WIN32)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64")
    elseif (APPLE)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64")
    else()
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64")
    endif()

    add_library(Bachelor_Practical3_Prebuilt STATIC "reference.cpp")
    target_link_libraries(Bachelor_Practical3_Prebuilt PRIVATE CGFramework)
    target_compile_features(Bachelor_Practical3_Prebuilt PRIVATE cxx_std_20)
endif()

if (USE_PREBUILT_REFERENCE_P3)
    add_executable(Bachelor_Practical3Tests tests.cpp)
    add_library(Bachelor_Practical3_Import STATIC IMPORTED)
    if (WIN32)
        set_target_properties(Bachelor_Practical3_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64/Bachelor_Practical3_Prebuilt.lib")
    elseif (APPLE)
        set_target_properties(Bachelor_Practical3_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64/libBachelor_Practical3_Prebuilt.a")
    else()
        set_target_properties(Bachelor_Practical3_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64/libBachelor_Practical3_Prebuilt.a")
    endif()

    target_link_libraries(Bachelor_Practical3Tests PUBLIC Bachelor_Practical3_Import)
else()
    add_executable(Bachelor_Practical3Tests tests.cpp reference.cpp)
endif()

add_test(NAME Bachelor_Practical3Tests COMMAND Bachelor_Practical3Tests)
target_compile_features(Bachelor_Practical3Tests PRIVATE cxx_std_20)
set_project_warnings(Bachelor_Practical3Tests)
target_link_libraries(Bachelor_Practical3Tests PRIVATE CGFramework Catch2WithMain)

add_executable(Bachelor_Practical3YourTests your_tests.cpp)
add_test(NAME Bachelor_Practical3YourTests COMMAND Bachelor_Practical3_YourTests)
target_compile_features(Bachelor_Practical3YourTests PRIVATE cxx_std_20)
set_project_warnings(Bachelor_Practical3YourTests)
target_link_libraries(Bachelor_Practical3YourTests PRIVATE CGFramework Catch2WithMain)


