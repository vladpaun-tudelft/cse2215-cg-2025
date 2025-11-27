add_executable(MazeAssignment
	"src/main.cpp"
	"src/maze.h"
	"src/sprite.h" 
	"src/sprite.cpp" 
	"src/maze.cpp" 
)
target_compile_features(MazeAssignment PRIVATE cxx_std_20)
target_link_libraries(MazeAssignment PRIVATE CGFramework)
enable_sanitizers(MazeAssignment)
set_project_warnings(MazeAssignment)

target_compile_definitions(MazeAssignment PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"")
