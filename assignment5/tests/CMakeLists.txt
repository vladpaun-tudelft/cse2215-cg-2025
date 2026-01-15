if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bvh_ref.cpp")
    if (WIN32)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64")
    elseif (APPLE)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64")
    else()
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64")
    endif()
    add_library(BVHReference STATIC bvh_ref.cpp)
    target_link_libraries(BVHReference PUBLIC CGFramework)
    target_compile_features(BVHReference PUBLIC cxx_std_20)
    set_project_warnings(BVHReference)
    enable_sanitizers(BVHReference)
else()
    add_library(BVHReference STATIC IMPORTED)
    if (WIN32)
        set_target_properties(BVHReference PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64/BVHReference.lib")
    elseif (APPLE)
        set_target_properties(BVHReference PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64/libBVHReference.a")
    else()
        set_target_properties(BVHReference PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64/libBVHReference.a")
    endif()
endif()

add_executable(Bachelor_Assignment5_Tests_Part1
    acceleration_data_structure_1.cpp
)

# A reference implementation is provided in a set of compact header files
target_include_directories(Bachelor_Assignment5_Tests_Part1 PRIVATE ../src)
target_link_libraries(Bachelor_Assignment5_Tests_Part1 PRIVATE CGFramework Bachelor_Assignment5_Lib BVHReference Catch2WithMain)
target_compile_features(Bachelor_Assignment5_Tests_Part1 PRIVATE cxx_std_23)
set_project_warnings(Bachelor_Assignment5_Tests_Part1)
enable_sanitizers(Bachelor_Assignment5_Tests_Part1)
add_test(NAME Bachelor_Assignment5_Tests_Part1 COMMAND Bachelor_Assignment5_Tests_Part1)

add_executable(Bachelor_Assignment5_Tests_Part2
        acceleration_data_structure_2.cpp
)

target_include_directories(Bachelor_Assignment5_Tests_Part2 PRIVATE ../src)
target_link_libraries(Bachelor_Assignment5_Tests_Part2 PRIVATE CGFramework Bachelor_Assignment5_Lib BVHReference Catch2WithMain)
target_compile_features(Bachelor_Assignment5_Tests_Part2 PRIVATE cxx_std_23)
set_project_warnings(Bachelor_Assignment5_Tests_Part2)
enable_sanitizers(Bachelor_Assignment5_Tests_Part2)
add_test(NAME Bachelor_Assignment5_Tests_Part2 COMMAND Bachelor_Assignment5_Tests_Part2)

add_executable(Bachelor_Assignment5_YourTests
        "your_tests.cpp"
)

target_compile_features(Bachelor_Assignment5_YourTests PRIVATE cxx_std_20)
set_project_warnings(Bachelor_Assignment5_YourTests)
target_link_libraries(Bachelor_Assignment5_YourTests PRIVATE CGFramework Bachelor_Assignment5_Lib Catch2WithMain)
