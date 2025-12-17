add_executable(AmusementParkAssignment
	"src/main.cpp"
)
target_compile_features(AmusementParkAssignment PRIVATE cxx_std_20)
target_link_libraries(AmusementParkAssignment PRIVATE CGFramework)
enable_sanitizers(AmusementParkAssignment)
set_project_warnings(AmusementParkAssignment)

target_compile_definitions(AmusementParkAssignment PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"")
