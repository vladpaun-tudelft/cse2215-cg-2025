if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/intersect.cpp")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_BACKUP ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    if (WIN32)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64")
    elseif (APPLE)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64")
    else()
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64")
    endif()
    add_library(IntersectLib STATIC intersect.cpp)
    target_link_libraries(IntersectLib PUBLIC CGFramework)
    target_compile_features(IntersectLib PUBLIC cxx_std_20)
    set_project_warnings(IntersectLib)
    enable_sanitizers(IntersectLib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_BACKUP})
else()
    add_library(IntersectLib STATIC IMPORTED)
    if (WIN32)
        set_target_properties(IntersectLib PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win_x64/IntersectLib.lib")
    elseif (APPLE)
        set_target_properties(IntersectLib PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/macos_arm64/libIntersectLib.a")
    else()
        set_target_properties(IntersectLib PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/linux_x64/libIntersectLib.a")
    endif()
endif()


add_library(Bachelor_Assignment5_Lib
        "bvh.cpp"
        "scene.cpp"
        "draw.cpp"
)

target_link_libraries(Bachelor_Assignment5_Lib PUBLIC CGFramework OpenGL::GLU OpenMP::OpenMP_CXX IntersectLib)
target_compile_features(Bachelor_Assignment5_Lib PUBLIC cxx_std_20)
enable_sanitizers(Bachelor_Assignment5_Lib)
set_project_warnings(Bachelor_Assignment5_Lib)

target_compile_definitions(Bachelor_Assignment5_Lib PUBLIC "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/../data/\"")

add_executable(Bachelor_Assignment5 "main.cpp")
target_link_libraries(Bachelor_Assignment5 PUBLIC Bachelor_Assignment5_Lib)
target_compile_features(Bachelor_Assignment5 PUBLIC cxx_std_23)
enable_sanitizers(Bachelor_Assignment5)
set_project_warnings(Bachelor_Assignment5)
