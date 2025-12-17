add_library(Bachelor_Practical4_Assignment4Lib STATIC "ray_tracing.cpp" "pacman.cpp")
target_link_libraries(Bachelor_Practical4_Assignment4Lib PUBLIC CGFramework)
target_compile_features(Bachelor_Practical4_Assignment4Lib PUBLIC cxx_std_20)
set_project_warnings(Bachelor_Practical4_Assignment4Lib)
enable_sanitizers(Bachelor_Practical4_Assignment4Lib)

find_package(OpenGL REQUIRED)

add_executable(Bachelor_Practical4 "main.cpp" "draw.cpp")
target_link_libraries(Bachelor_Practical4 PRIVATE Bachelor_Practical4_Assignment4Lib CGFramework OpenGL::GLU)
target_compile_features(Bachelor_Practical4 PRIVATE cxx_std_20)
enable_sanitizers(Bachelor_Practical4)
set_project_warnings(Bachelor_Practical4)

target_compile_definitions(Bachelor_Practical4 PRIVATE "-DGL_SILENCE_DEPRECATION=1")
target_compile_definitions(Bachelor_Practical4 PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"" "-DOUTPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/\"")

