option(USE_PREBUILT "Use the prebuilt libraries for the reference solutions" ON)
option(MAKE_PREBUILT "Generate prebuilt libraries for the reference solutions (students please don't turn on this flag)" OFF)

if (MAKE_PREBUILT)
	add_library(Bachelor_Practical2_Prebuilt STATIC "src/reference.cpp")

    target_link_libraries(Bachelor_Practical2_Prebuilt PRIVATE CGFramework Catch2WithMain)
    target_compile_features(Bachelor_Practical2_Prebuilt PRIVATE cxx_std_20)

    # Copy target to prebuilt/
    if (WIN32)
        set(prebuilt_output "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/Provided_x64.lib")
    elseif (APPLE)
        set(prebuilt_output "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libProvided_macos_arm64.a")
    else()
        set(prebuilt_output "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libProvided_linux_x64.a")
    endif()

    # Copy rule that produces the prebuilt output
    add_custom_command(
        OUTPUT "${prebuilt_output}"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Bachelor_Practical2_Prebuilt> "${prebuilt_output}"
        DEPENDS Bachelor_Practical2_Prebuilt
        COMMENT "Copying prebuilt library to ${prebuilt_output}"
    )

    # Phony target that always checks for (and regenerates) the file
    add_custom_target(copy_prebuilt ALL
        DEPENDS "${prebuilt_output}"
    )
endif()

if (USE_PREBUILT)
    add_executable(Bachelor_Practical2_MazeTests "src/maze.cpp")
    add_executable(Bachelor_Practical2_AmusementParkTests "src/amusement_park.cpp")

    # Import from prebuilt/
    add_library(Bachelor_Practical2_Import STATIC IMPORTED)
	if (WIN32)
		set_target_properties(Bachelor_Practical2_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/Provided_x64.lib")
	elseif (APPLE)
		set_target_properties(Bachelor_Practical2_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libProvided_macos_arm64.a")
	else()
		set_target_properties(Bachelor_Practical2_Import PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libProvided_linux_x64.a")
	endif()

    target_link_libraries(Bachelor_Practical2_MazeTests PUBLIC Bachelor_Practical2_Import)
    target_link_libraries(Bachelor_Practical2_AmusementParkTests PUBLIC Bachelor_Practical2_Import)
else ()
    add_executable(Bachelor_Practical2_MazeTests "src/maze.cpp" "src/reference.cpp")
    add_executable(Bachelor_Practical2_AmusementParkTests "src/amusement_park.cpp" "src/reference.cpp")
endif ()

target_link_libraries(Bachelor_Practical2_MazeTests PRIVATE CGFramework Catch2WithMain)
target_link_libraries(Bachelor_Practical2_AmusementParkTests PRIVATE CGFramework Catch2WithMain)

target_compile_features(Bachelor_Practical2_MazeTests PRIVATE cxx_std_20)
target_compile_features(Bachelor_Practical2_AmusementParkTests PRIVATE cxx_std_20)

set_project_warnings(Bachelor_Practical2_MazeTests)
set_project_warnings(Bachelor_Practical2_AmusementParkTests)

add_test(NAME Bachelor_Practical2_MazeTests COMMAND Bachelor_Practical2_MazeTests)
add_test(NAME Bachelor_Practical2_AmusementParkTests COMMAND Bachelor_Practical2_AmusementParkTests)
