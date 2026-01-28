cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/solution.cpp")
    # Make prebuilt
    if (WIN32)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64")
    elseif (APPLE)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64")
    else()
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64")
    endif()
    add_library(Tests_Solution solution.cpp)
    target_include_directories(Tests_Solution PRIVATE include)
    target_link_libraries(Tests_Solution PRIVATE CGFramework Bachelor_FinalProjectLib)
else()
    # Use prebuilt solution
    add_library(Tests_Solution STATIC IMPORTED)
    if (WIN32)
        set_target_properties(Tests_Solution PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64/Tests_Solution.lib")
    elseif (APPLE)
        set_target_properties(Tests_Solution PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64/libTests_Solution.a")
    else()
        set_target_properties(Tests_Solution PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64/libTests_Solution.a")
    endif()
endif()

# Source files correspond to a single standard feature
# and all its relevant tests
add_executable(Bachelor_FinalProjectTests 
  src/interpolation.cpp
  src/lights_and_shadows.cpp
  src/multisampling.cpp
  src/recursive_ray_reflections.cpp
  src/recursive_ray_transparency.cpp
  src/shading_models.cpp
  src/texture_mapping.cpp
        include/ostream_custom.h
        include/InvisibleWindow.cpp
        include/InvisibleWindow.h
 )

# A reference implementation is provided in a set of compact
# header files
target_include_directories(Bachelor_FinalProjectTests PRIVATE include)
target_link_libraries(Bachelor_FinalProjectTests PRIVATE CGFramework Bachelor_FinalProjectLib Catch2WithMain Tests_Solution)
target_compile_features(Bachelor_FinalProjectTests PRIVATE cxx_std_23)
set_project_warnings(Bachelor_FinalProjectTests)
enable_sanitizers(Bachelor_FinalProjectTests)
add_test(NAME Bachelor_FinalProjectTests COMMAND Bachelor_FinalProjectTests)

# Disable Catch2 from catching Windows SEH exceptions; we will do that manually such that the tests can continue.
# A typical SEH exception that occurs is dividing by zero in the first exercise when an empty input is provided.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_definitions(Catch2 PUBLIC "CATCH_CONFIG_NO_WINDOWS_SEH=1")
  set_target_properties(Bachelor_FinalProjectTests PROPERTIES COMPILE_FLAGS "/EHa")
endif ()

