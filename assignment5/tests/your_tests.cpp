// Put your includes here
#include "../src/bvh.h"
#include "../src/scene.h"
#include <limits>

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
//

// Add your tests here, if you want :D
TEST_CASE("StudentTest")
{
    // Add your own tests here...
}

// The below tests are not "good" unit tests. They don't actually test correctness.
// They simply exist for demonstrative purposes. As they interact with the interfaces
// (scene, bvh_interface, etc). They should compile without changes.
TEST_CASE("InterfaceTest")
{
    Scene scene = loadScenePrebuilt(SceneType::Cube, DATA_DIR);
    BVH bvh(scene);

    SECTION("BVH generation")
    {
        // There's something in here?
        CHECK(!bvh.primitives().empty());
    }

    SECTION("BVH traversal")
    {
        Ray ray = { .origin = glm::vec3(0), .direction = glm::vec3(1) };
        HitData hitInfo;

        // Hit something?
        CHECK(bvh.intersect(scene, ray, hitInfo));
        CHECK(ray.t != std::numeric_limits<float>::max());
    }
}
