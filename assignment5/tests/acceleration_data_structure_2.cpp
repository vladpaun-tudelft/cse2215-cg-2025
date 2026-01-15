#include <algorithm>
#include <list>
#include <vector>
#include <array>
#include <span>
#include <random>

#include "../src/bvh.h" // Include the student's BVH
#include "util.h"
#include "bvh_ref.h"
#include "../src/intersect.h"
#include <numeric>
#include <iostream>
#include "framework/glm_ostream.h"

std::ostream &operator<<(std::ostream &stream, const AxisAlignedBox &aabb) {
    stream << "AxisAlignedBox {";
    stream << "lower: " << aabb.lower;
    stream << ", upper: " << aabb.upper;
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const Vertex &vertex) {
    stream << "Vertex {";
    stream << "position: " << vertex.position;
    stream << " [...] }";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface::Primitive &prim) {
    stream << "Primitive {";
    stream << "meshID: " << prim.meshID;
    stream << ", v1: " << prim.v1;
    stream << ", v2: " << prim.v1;
    stream << ", v3: " << prim.v1;
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface::Node &node) {
    stream << "Node {";
    stream << "leftChild: " << node.leftChild();
    stream << ", rightChild: " << node.rightChild();
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface &bvh) {
    stream << "BVH {";
    stream << "Nodes: {";
    for (const auto &node : bvh.nodes()) {
        stream << node;
        stream << ",";
    }
    stream << "} [...] }";
    return stream;
}

namespace test {
    // Test settings
    constexpr uint32_t num_samples = 16; // Number of random samples taken in certain tests
    constexpr uint32_t allowed_time_multiplier = 10; // >= 10x reference intersect() time throws a warning for now

    std::tuple<ref::Sampler, Scene, ref::BVH, BVHInterface::Primitive, BVHInterface::Primitive>
    getPreambel() {
        // Instantiate reference objects
        static ref::Sampler sampler(4);

        // Load test scene
        static Scene scene = loadScenePrebuilt(SceneType::Teapot, DATA_DIR);

        // Instantiate reference bvh, not student's bvh
        static ref::BVH refr_bvh(scene);

        // Instantiate collapsed primitive, should be handled correctly either way
        static BVHInterface::Primitive good_prim = refr_bvh.primitives()[0];
        static BVHInterface::Primitive empty_prim = {
            .meshID = 16,
            .v0 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)},
            .v1 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)},
            .v2 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)}
        };

        return {sampler, scene, refr_bvh, good_prim, empty_prim};
    }

TEST_CASE("BVH Recursive Construction: Build Recursive") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();
    // The following gets a bit tricky, as it is essentially an "end-to-end" test
    // of the BVH constructor.
    // We will simply confirm several necessary properties of a good BVH, s.a.
    // - approximately O(log N) in the nr. of ray-triangle intersections
    // - an expected node-leaf structure with the right nr. of levels
    // - given a reference traversal, correct ray hits

    // bvh.buildRecursive
    // Generate the student's BVH implementation
    ::BVH stud_bvh(scene);

    // We will do a count of the nr. of BVH levels, to confirm the tree is deep, and not wide
    SECTION("bvh.buildRecursive [A full tree is built; no dangling nodes or large leaves]")
    {
        // Set a limit to the level size to prevent infinite recursion
        uint32_t max_allowed_levels = static_cast<uint32_t>(refr_bvh.numLevels()) + 2;

        // Check that max level size was not grossly exceeded; potentially infinite recursion
        bool is_exceeded = false;

        // Check that no exception was caught, indicating wonky bvh data
        bool is_caught = false;

        // Count the nr. of levels
        uint32_t numLevels = 0;
        try {
            // Often used accessors
            std::span<const BVHInterface::Node> nodes = stud_bvh.nodes();
            std::span<const BVHInterface::Primitive> primitives = stud_bvh.primitives();

            // We use a function to do a recursive count
            std::function<void(uint32_t, int)> countLevels =
                [&](uint32_t currentNodeIdx, uint32_t currentLevel) {
                    if (currentLevel > max_allowed_levels) {
                        is_exceeded = true;
                        return;
                    }

                    numLevels = std::max(numLevels, currentLevel);
                    if (currentNodeIdx >= nodes.size()) {
                        is_exceeded = true;
                        return;
                    };
                    const auto& bvhNode = nodes[currentNodeIdx];
                    if (!bvhNode.isLeaf()) {
                        for (const auto childNodeIdx : bvhNode.data) {
                            countLevels(childNodeIdx, currentLevel + 1);
                        }
                    }
                };

            // Start recursive count
            countLevels(0, 1);
        } catch (const std::exception&) {
            is_caught = true;
            numLevels = 0;
        }

        // Check that difference between both level counts is at most 2, and
        // that there are actually multiple levels
        auto refr_lvls = static_cast<int>(refr_bvh.numLevels());
        auto stud_lvls = static_cast<int>(numLevels);
        // Note: Feel free to print the entire bvh here instead or set a break point
        // CAPTURE(stud_bvh, refr_bvh);
        CAPTURE(refr_lvls, stud_lvls);
        REQUIRE_FALSE(is_caught); // No exceptions were caught
        REQUIRE_FALSE(is_exceeded); // No signs of potential infinite recursion
        REQUIRE_FALSE(stud_lvls <= 1); // There actually are multiple levels
        REQUIRE(std::abs(refr_lvls - stud_lvls) <= 2); // Level count is within reasonable deviation of reference implementation
    }

    // We use traversal to find an arbitrary primitive along a ray, and confirm we did
    // not deal with O(N) comparisons.
    SECTION("bvh.buildRecursive [Traversal to find a primitive is logarithmic]")
    {
        // We limit the nr. of allowed ray-primitive tests;
        // if this is exceeded, the bvh is no better than iterating over all primitives
        int max_allowed_prims = static_cast<int>(refr_bvh.primitives().size());

        // We limit the nr. of allowed steps in the while-loop;
        // if this is exceeded, the test fails as something went wrong in the student's bvh
        int max_allowed_steps = static_cast<int>(refr_bvh.primitives().size());

        // This boolean needs to be set to true, as we know the ray intersects a primitive
        bool is_hit = false;

        // This boolean needs to remain false, as otherwise some exception was thrown
        bool is_caught = false;

        // Often used accessors
        std::span<const BVHInterface::Node> nodes = stud_bvh.nodes();
        std::span<const BVHInterface::Primitive> primitives = stud_bvh.primitives();

        // Define a ray which we know intersects a primitive
        Ray ray = { .origin = glm::vec3(0), .direction = glm::vec3(1) };

        try {
            // Initialize traversal stack with root node pushed on it
            detail::FixedStack<uint32_t, 24> stack;
            stack.push(0);

            // Stack-based traversal, or until a maximum was hit
            while (!stack.empty()
                && max_allowed_steps > 0
                && max_allowed_prims > 0) {

                const auto& node = nodes[stack.pop()];

                if (node.isLeaf()) {
                    // Leaf node, iterate over all primitives within and decrease primitive counter
                    for (const auto& prim : primitives.subspan(node.primitiveOffset(), node.primitiveCount())) {
                        const auto& [v0, v1, v2] = std::tie(prim.v0, prim.v1, prim.v2);
                        if (intersectRayWithTriangle(v0.position, v1.position, v2.position, ray)) {
                            is_hit = true;
                        }
                        max_allowed_prims--;
                    }
                } else {
                    // Inner node; handle left/right cases in general
                    float tl = std::numeric_limits<float>::max(), tr = std::numeric_limits<float>::max();
                    bool hitl = detail::intersectRayWithAABBVolume(nodes[node.leftChild()].aabb, ray, tl),
                         hitr = detail::intersectRayWithAABBVolume(nodes[node.rightChild()].aabb, ray, tr);
                    if (hitl && hitr) {
                        if (tl < tr) {
                            stack.push(node.rightChild());
                            stack.push(node.leftChild());
                        } else {
                            stack.push(node.leftChild());
                            stack.push(node.rightChild());
                        }
                    } else if (hitl) {
                        stack.push(node.leftChild());
                    } else if (hitr) {
                        stack.push(node.rightChild());
                    }
                }

                max_allowed_steps--;
            } // while (!stack.empty())
        } catch (const std::exception &e) {
            CAPTURE(e);
            is_caught = true;
            REQUIRE(is_caught);
        }

        // Test that no safety counter was hit during traversal, and that
        // we did in fact intersect a primitive, and that this is the same
        // primitive we _expected_ to intersect, and that nothing crashed
        // Note: Feel free to capture the entire board here or just set a break point
        // CAPTURE(stud_bvh, refr_bvh);
        REQUIRE_FALSE(is_caught); // An exception was thrown, instant fail
        REQUIRE(is_hit); // No primitive was hit, instant fail
        REQUIRE(max_allowed_steps > 0); // Maximum allowed nr. of steps equals or exceeds nr. of primitives
        REQUIRE(max_allowed_prims > 0); // maximum allowed nr. of primitive tests equals or exceeds nr. of primitives
    }

    // We run several hit tests on the BVH to ensure the same primitives
    // are always returned for the reference and the student's BVH using the reference intersection method
    SECTION("bvh.buildRecursive [The correct primitives are encountered on intersection with reference method]")
    {
        // Sample rays and compare results
        bool is_stud_eql = true;
        bool is_stud_hit = true;
        bool throws_exception = false;
        try {
            for (uint32_t i = 0; i < num_samples; ++i) {
                // Sample a ray by picking points on a sphere far away from the scene,
                // s.t. the ray targets the center fo the scene
                glm::vec3 d = detail::pointOnSphere(sampler);
                glm::vec3 o = -16.f * d;

                // Generate identical placeholder ray/hit objects
                HitData refr_hit, stud_hit;
                Ray refr_ray = {.origin = o, .direction = d}, stud_ray = {.origin = o, .direction = d};

                // Run both intersection tests
                // CHANGED: TRAVERSE BOTH WITH OUR INTERSECT METHOD
                auto refr_did_hit = ref::intersectRayWithBVH(scene, refr_bvh, refr_ray, refr_hit);
//                auto stud_did_hit = ::intersectRayWithBVH(scene, stud_bvh, stud_ray, stud_hit);
                auto stud_did_hit = ref::intersectRayWithBVH(scene, stud_bvh, stud_ray, stud_hit);

                // Update student test return
                is_stud_hit &= (refr_did_hit == stud_did_hit);
                is_stud_eql &= epsEqual(refr_ray.t, stud_ray.t);

                CAPTURE(stud_ray, refr_ray);
                REQUIRE(stud_did_hit == refr_did_hit);
                REQUIRE(epsEqual(refr_ray.t, stud_ray.t));
            }
        }
        catch (std::exception &e) {
            CAPTURE(e);
            throws_exception = true;
            REQUIRE(throws_exception);
        }

        // Verify student solution
        CAPTURE(stud_bvh, refr_bvh);
        if (!throws_exception) {
            REQUIRE(is_stud_hit); // Function always reported hit/no-hit equal to reference
            REQUIRE(is_stud_eql); // Function always updated hit distance equal to reference
        }
        else {
            REQUIRE_FALSE(throws_exception);
        }
    }
}

TEST_CASE("BVH Traversal: Correct Intersections on Reference BVH") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    SECTION("bvh.intersectRayWithBVH [Correct Intersections, Note: Cannot test asymptotic runtime complexity]") {
        // Note: We only test if the intersections returned are still the same as with naive traversal.
        // We cannot trivially test if the student actually traverses the BVH for better runtime complexity.
        // Sample rays and compare results
        bool is_stud_eql = true;
        bool is_stud_hit = true;
        bool throws_exception = false;
        try {
            for (uint32_t i = 0; i < num_samples; ++i) {
                // Sample a ray by picking points on a sphere far away from the scene,
                // s.t. the ray targets the center fo the scene
                glm::vec3 d = detail::pointOnSphere(sampler);
                glm::vec3 o = -16.f * d;

                // Generate identical placeholder ray/hit objects
                HitData refr_hit, stud_hit;
                Ray refr_ray = {.origin = o, .direction = d}, stud_ray = {.origin = o, .direction = d};

                // Run both intersection tests on our BVH
                auto refr_did_hit = ref::intersectRayWithBVH(scene, refr_bvh, refr_ray, refr_hit);
                auto stud_did_hit = ::intersectRayWithBVH(scene, refr_bvh, stud_ray, stud_hit);

                // Update student test return
                is_stud_hit &= (refr_did_hit == stud_did_hit);
                is_stud_eql &= epsEqual(refr_ray.t, stud_ray.t);

                CAPTURE(stud_ray, refr_ray);
                REQUIRE(stud_did_hit == refr_did_hit);
                REQUIRE(epsEqual(refr_ray.t, stud_ray.t));

                CAPTURE(stud_ray, refr_ray, stud_hit, refr_did_hit);
                REQUIRE(epsEqual(refr_hit.normal, stud_hit.normal));
            }
        }
        catch (std::exception &e) {
            CAPTURE(e);
            throws_exception = true;
            REQUIRE(throws_exception);
        }

        // Verify student solution
        CAPTURE(refr_bvh);
        if (!throws_exception) {
            REQUIRE(is_stud_hit); // Function always reported hit/no-hit equal to reference
            REQUIRE(is_stud_eql); // Function always updated hit distance equal to reference
        }
        else {
            REQUIRE_FALSE(throws_exception);
            REQUIRE_FALSE(throws_exception);
        }
    }
}
}