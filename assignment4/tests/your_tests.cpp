#include "../src/ray_tracing.h"
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

TEST_CASE("RayTriangleIntersection")
{
    CHECK(true);
    // Add your ray/triangle intersection tests here
}
