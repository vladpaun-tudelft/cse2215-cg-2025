// Suppress warnings in third-party code.
#include "tests.h"
#include "interpolate.h" // Include the student's code
#include "bvh.h"
#include "solution.h"

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples = 16; // Number of random samples taken in certain tests

TEST_CASE("Interpolation")
{
    // Load test scene
    Scene scene = loadScenePrebuilt(SceneType::CornellBox, DATA_DIR);

    // Instantiate reference objects
    BVH bvh(scene, { .enableAccelStructure = true });
    ref::Sampler sampler(4);

    SECTION("computeBarycentricCoord")
    {
        // Generate a random subset 'prims' of all primitives
        std::vector<BVHInterface::Primitive> prims(bvh.primitives().begin(), bvh.primitives().end());
        rng::shuffle(prims, sampler);
        prims.resize(std::min(num_samples, static_cast<uint32_t>(prims.size())));

        // Test check values
        bool is_stud_correct_inside = true;
        bool is_stud_correct_edge = true;
        bool is_stud_correct_vert = true;

        std::vector<glm::vec3> inside_ps {};
        std::vector<glm::vec3> inside_bary_ref {};
        std::vector<glm::vec3> inside_bary_stud {};

        std::vector<glm::vec3> onvertex_ps {};
        std::vector<glm::vec3> onvertex_bary_ref {};
        std::vector<glm::vec3> onvertex_bary_stud {};

        std::vector<glm::vec3> onedge_ps {};
        std::vector<glm::vec3> onedge_bary_ref {};
        std::vector<glm::vec3> onedge_bary_stud {};

        // Iterate over this subset of primitives
        for (const auto& prim : prims) {
            // Extract primitives positions
            const auto& [p0, p1, p2] = std::tie(prim.v0.position, prim.v1.position, prim.v2.position);

            // Reused variables
            glm::vec3 bary, p;

            // Generate valid barycentric coordinates inside the primitive,
            // and a matching point at these coordinates
            bary = sampler.next_3d();
            bary /= (bary.x + bary.y + bary.z);
            p = p0 * bary.x + p1 * bary.y + p2 * bary.z;
            inside_ps.push_back(p);
            inside_bary_ref.push_back(bary);

            // First, verify the reference recovers the coordinates
            // Next, verify the solution recovers the coordinates
            if constexpr (run_ref_tests)
              REQUIRE(epsEqual(solution::computeBarycentricCoord(p0, p1, p2, p), bary));
            glm::vec3 bary_stud = ::computeBarycentricCoord(p0, p1, p2, p);
            inside_bary_stud.push_back(bary_stud);
            is_stud_correct_inside &= epsEqual(bary_stud, bary);

            // Generate a point exactly on one of the primitive's vertices
            bary = { 0, 1, 0 };
            p = p0 * bary.x + p1 * bary.y + p2 * bary.z;
            onvertex_ps.push_back(p);
            onvertex_bary_ref.push_back(bary);

            // First, verify the reference recovers the coordinates
            // Next, verify the solution recovers the coordinates
            if constexpr (run_ref_tests)
              REQUIRE(epsEqual(solution::computeBarycentricCoord(p0, p1, p2, p), bary));
            bary_stud = ::computeBarycentricCoord(p0, p1, p2, p);
            onvertex_bary_stud.push_back(bary_stud);
            is_stud_correct_edge &= epsEqual(bary_stud, bary);

            // Generate a point exactly on one of the primitive's edges
            bary = { 0, 0.35, 0.65 };
            p = p0 * bary.x + p1 * bary.y + p2 * bary.z;
            onedge_ps.push_back(p);
            onedge_bary_ref.push_back(bary);

            // First, verify the reference recovers the coordinates
            // Next, verify the solution recovers the coordinates
            if constexpr (run_ref_tests)
              REQUIRE(epsEqual(solution::computeBarycentricCoord(p0, p1, p2, p), bary));
            bary_stud = ::computeBarycentricCoord(p0, p1, p2, p);
            onedge_bary_stud.push_back(bary_stud);
            is_stud_correct_vert &= epsEqual(bary_stud, bary);

            // We do not test for points outside the primitive, as the assignment
            // clarifies the point lies **on** the triangle and even the reference
            // does not support this
        }

        // Verify student solution
        CAPTURE(prims, inside_ps, inside_bary_ref, inside_bary_stud);
        CHECK(is_stud_correct_inside); // Is the solution correct inside the triangle
        CAPTURE(prims, onedge_ps, onedge_bary_ref, onedge_bary_stud);
        CHECK(is_stud_correct_edge); // Is the solution correct on triangle edge
        CAPTURE(prims, onvertex_ps, onvertex_bary_ref, onvertex_bary_stud);
        CHECK(is_stud_correct_vert); // Is the solution correct on triangle vert
    }

    SECTION("interpolateNormal/interpolateTexCoord")
    {
        // Generate a random subset 'prims' of all primitives
        std::vector<BVHInterface::Primitive> prims(bvh.primitives().begin(), bvh.primitives().end());
        rng::shuffle(prims, sampler);
        prims.resize(std::min(num_samples, static_cast<uint32_t>(prims.size())));
        
        // Test check values
        bool is_stud_correct_inside = true;
        bool is_stud_correct_outside = true;

        std::vector<glm::vec3> all_inside_stud_n {};
        std::vector<glm::vec2> all_inside_stud_t {};
        std::vector<glm::vec3> all_inside_refr_n {};
        std::vector<glm::vec2> all_inside_refr_t {};

        std::vector<glm::vec3> all_outside_stud_n {};
        std::vector<glm::vec2> all_outside_stud_t {};
        std::vector<glm::vec3> all_outside_refr_n {};
        std::vector<glm::vec2> all_outside_refr_t {};

        // Iterate over this subset of primitives
        for (const auto& prim : prims) {
            // Extract primitive normals/texcoords
            const auto& [n0, n1, n2] = std::tie(prim.v0.normal, prim.v1.normal, prim.v2.normal);
            const auto& [t0, t1, t2] = std::tie(prim.v0.texCoord, prim.v1.texCoord, prim.v2.texCoord);

            // Reused variables
            glm::vec3 bary, n;
            glm::vec2 t;

            // Generate valid barycentric coordinates inside the primitive
            // and matching normals/texcoords at these coordinates
            bary = sampler.next_3d();
            bary /= (bary.x + bary.y + bary.z);
            n = n0 * bary.x + n1 * bary.y + n2 * bary.z;
            t = t0 * bary.x + t1 * bary.y + t2 * bary.z;

            all_inside_refr_n.push_back(n);
            all_inside_refr_t.push_back(t);

            // First, verify the reference recovers the values
            // Next, verify the solution recovers the values
            if constexpr (run_ref_tests) {
              REQUIRE(epsEqual(solution::interpolateNormal(n0, n1, n2, bary), n));
              REQUIRE(epsEqual(solution::interpolateTexCoord(t0, t1, t2, bary), t));
            }

            auto inside_stud_n = ::interpolateNormal(n0, n1, n2, bary);
            auto inside_stud_t = ::interpolateTexCoord(t0, t1, t2, bary);
            all_inside_stud_n.push_back(inside_stud_n);
            all_inside_stud_t.push_back(inside_stud_t);

            is_stud_correct_inside 
              &= (epsEqual(inside_stud_n, n))
              && (epsEqual(inside_stud_t, t));

            // Generate arbitrary coordinates outside the primitive;
            // the assignment never specifies the solution should clamp!
            bary = sampler.next_3d() * 2.f - 1.f;
            n = glm::normalize(n0 * bary.x + n1 * bary.y + n2 * bary.z);
            t = t0 * bary.x + t1 * bary.y + t2 * bary.z;
            all_outside_refr_n.push_back(n);
            all_outside_refr_t.push_back(t);

            // First, verify the reference recovers the values
            // Next, verify the solution recovers the values
            if constexpr (run_ref_tests) {
              REQUIRE(epsEqual(glm::normalize(solution::interpolateNormal(n0, n1, n2, bary)), n));
              REQUIRE(epsEqual(solution::interpolateTexCoord(t0, t1, t2, bary), t));
            }
            auto outside_stud_n = glm::normalize(::interpolateNormal(n0, n1, n2, bary));
            auto outside_stud_t = ::interpolateTexCoord(t0, t1, t2, bary);
            all_outside_stud_n.push_back(outside_stud_n);
            all_outside_stud_t.push_back(outside_stud_t);

            is_stud_correct_outside
              &= (epsEqual(outside_stud_n, n))
              && (epsEqual(outside_stud_t, t));
        }

        // Verify student solution
        CAPTURE(prims, all_inside_stud_n, all_inside_refr_n, all_inside_stud_t, all_inside_refr_t);
        CHECK(is_stud_correct_inside); // Is the solution correct for [0, 1] values
        CAPTURE(prims, all_outside_stud_n, all_outside_refr_n, all_outside_stud_t, all_outside_refr_t);
        CHECK(is_stud_correct_outside); // Is the solution correct for unbounded values
    }
}
} // namespace test