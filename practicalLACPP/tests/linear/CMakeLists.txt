add_executable(Bachelor_Practical1_2_Tests
	"main.cpp"
	"../../src/linear/your_code_here.cpp")
target_include_directories(Bachelor_Practical1_2_Tests PRIVATE "../../src/linear") # Get access to "your_code_here.h"
target_compile_features(Bachelor_Practical1_2_Tests PRIVATE cxx_std_20)
set_project_warnings(Bachelor_Practical1_2_Tests)
enable_sanitizers(Bachelor_Practical1_2_Tests)
add_test(NAME Bachelor_Practical1_2_Tests COMMAND Bachelor_Practical1_2_Tests)

# Disable Catch2 from catching Windows SEH exceptions; we will do that manually such that the tests can continue.
# A typical SEH exception that occurs is dividing by zero in the first exercise when an empty input is provided.
if (WIN32)
	target_compile_definitions(Catch2 PUBLIC "CATCH_CONFIG_NO_WINDOWS_SEH=1")
	set_target_properties(Bachelor_Practical1_2_Tests PROPERTIES COMPILE_FLAGS "/EHa")
endif ()

target_link_libraries(Bachelor_Practical1_2_Tests PRIVATE CGFramework Catch2WithMain glm::glm)
