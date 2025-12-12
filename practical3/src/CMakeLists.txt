add_executable(Bachelor_Practical3
        "main.cpp"
        "draw.cpp")
target_compile_features(Bachelor_Practical3 PRIVATE cxx_std_20)
target_link_libraries(Bachelor_Practical3 PRIVATE CGFramework)
enable_sanitizers(Bachelor_Practical3)
set_project_warnings(Bachelor_Practical3)
