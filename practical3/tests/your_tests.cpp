#include "../src/your_code_here.h"
// This comment acts as a barrier for clang-format such that your_code_here.h is always included first

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()

// In this file you can add your own unit tests using the Catch2 library.
// You can find the documentation of Catch2 at the following link:
// https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md
//
// These tests are only to help you verify that your code is correct.

TEST_CASE("Student Tests")
{
    SECTION("Diffuse") {
        const MaterialInformation materialInformation{
                .Kd = glm::vec3(1, 1, 1)
        };
        const glm::vec3 normal = glm::vec3(0, 1, 0);
        const glm::vec3 vertexPos = glm::vec3(0, 0, 0);
        const glm::vec3 lightPos = glm::vec3(1, 1, 0);
        const glm::vec3 lightColor = glm::vec3(1, 1, 1);

        // Compute expected values and check if your code matches.
        const glm::vec3 yourResult = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
        std::cout << yourResult.r << std::endl;
        // REQUIRE(yourResult.r == Approx(123.0f));
        //REQUIRE(yourResult.g == Approx(456.0f));
        //REQUIRE(yourResult.b == Approx(789.0f));
    }
}
