#include "tests.h"
#include "recursive.h" // Include the student's code
#include "solution.h"

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples = 16; // Number of random samples taken in certain tests
constexpr float override_eps = 0.02f;


struct test_info {
    Ray incident, exitant_stud, exitant_ref;
    HitInfo hit;
};

inline std::ostream& operator<<(std::ostream& stream, const test_info& ti)
{
    stream << "TestCase {";
    stream << "incident: " << ti.incident;
    stream << ", HitInfo.normal: " << ti.hit.normal;
    stream << ", exitant_stud: " << ti.exitant_stud;
    stream << ", exitant_ref: " << ti.exitant_ref;
    stream << "}";
    return stream;
}

struct PredeterminedBVHIntersectionTrans {
    glm::vec3 material_kd;
    float material_transparency;
    float ray_t;
};

inline std::ostream& operator<<(std::ostream& stream, const PredeterminedBVHIntersectionTrans& x)
{
    stream << "PredeterminedBVHIntersectionTrans {";
    stream << "material_kd: " << x.material_kd;
    stream << ", material_transparency: " << x.material_transparency;
    stream << ", ray_t: " << x.ray_t;
    stream << "}";
    return stream;
}


TEST_CASE("Recursive ray transparency")
{
    // Instantiate reference objects
    ref::Sampler sampler(4);

    SECTION("generatePassthroughRay")
    {
        // Test check values
        bool is_stud_correct_origin = true;
        bool is_stud_correct_direction = true;
        bool is_stud_correct_distance = true;

        std::vector<test_info> test_data_wrong_orig {};
        std::vector<test_info> test_data_wrong_dir {};
        std::vector<test_info> test_data_wrong_t {};

        for (uint32_t i = 0; i < num_samples; ++i) {
            test_info info{};
            // Generate random hit position with incident ray; note that we
            // do not fill in most of HitInfo. If students are using barycentricCoords for this
            // for some reason, the test will fail (as it should)
            Ray incident = {
                .origin = sampler.next_3d(),
                .direction = glm::normalize(sampler.next_3d() * 2.f - 1.f),
                .t = sampler.next_1d() * 32.f
            };
            info.incident = incident;
            HitInfo hit = {
                .normal = glm::normalize(sampler.next_3d() * 2.f - 1.f)
            };
            info.hit = hit;

            // Generate student and reference solution
            Ray stud_exitant = ::generatePassthroughRay(incident, hit);
            info.exitant_stud = stud_exitant;
            Ray refr_exitant = solution::generatePassthroughRay(incident, hit);
            info.exitant_ref = refr_exitant;

            // Test that exitant ray origin lies somewhere around the hit point, with sufficient eps
            // Something tricky: students probably add an offset whose size or type we can't predict
            // accurately, so we relax the test slightly
            bool stud_corr_orig = (epsEqual(refr_exitant.origin - stud_exitant.origin, glm::vec3(0), override_eps));
            is_stud_correct_origin &= stud_corr_orig;

            // Test that exitant ray direction is mostly correct
            // This should catch students offsetting the exitant ray by epsilon instead
            bool stud_corr_dir = (epsEqual(refr_exitant.direction, stud_exitant.direction, override_eps));
            is_stud_correct_direction &= stud_corr_dir;

            // Test that exitant ray still has t at infinity, as otherwise intersection tests
            // with this ray will break
            bool stud_corr_dist = (stud_exitant.t == refr_exitant.t);
            is_stud_correct_distance &= stud_corr_dist;

            CAPTURE(info);
            REQUIRE(stud_corr_orig); // Is the student's ray origin correct?
            REQUIRE(stud_corr_dir);  // Is the student's ray direction correct?
            REQUIRE(stud_corr_dist); // Is the student's ray distance at infinity?
        }
    }

    SECTION("renderRayTransparentComponent")
    {
        for (uint32_t i = 0; i < num_samples; ++i) {
            // Pick an expected recursion depth of 1-6
            uint32_t max_depth = 1 + static_cast<uint32_t>(sampler.next_1d() * 6);

            // Generate a fake BVH that feeds random data into intersection points along a fake path
            ref::FakeBVH bvh(sampler, max_depth);

            // Dummy scene/state/ray/hit to ensure renderRay() is configured properly
            Scene scene;
            scene.lights = { PointLight { .position = sampler.next_3d(), .color = sampler.next_3d() }};
            RenderState state = {
                .scene = scene,
                .features = {
                    .enableShading = false,
                    .enableReflections = false,
                    .enableShadows = false,
                    .enableAccelStructure = true,
                    .enableTransparency = true },
                .bvh = bvh
            };
            Ray ray;
            HitInfo hit;

            std::vector<PredeterminedBVHIntersectionTrans> deterministicBVH_HitPointSequence {};
            auto lightColor = std::get<PointLight>(scene.lights[0]).color;

            glm::vec3 refr_colr(0);
            for (const auto& v : bvh.hits() | vws::reverse) {
                deterministicBVH_HitPointSequence.emplace_back(v.hit.material.kd, v.hit.material.transparency, v.t);
                if (v.t == std::numeric_limits<float>::max())
                    continue;
                solution::accumulateTransparentColor(v.hit.material, lightColor, refr_colr);
            }

            // Next, verify the student solution
            // Note that generation of a reflection ray is independent of whether the
            // specular color computation is correct.. it only depends on where the
            // ray hits... so this dumb check is a nice way to simply verify ks * (...)

            glm::vec3 stud_colr = ::renderRay(state, ray);
            bool stud_equal =  (epsEqual(stud_colr, refr_colr));

            INFO("renderRay is tested using a deterministic \"BVH\" returning predetermined hit points to make the test independent of your implementation of other parts. Therefore, in case of test failure, we provide you with the hit point sequence(s) which failed so you can track your mistake down if desired.");
            CAPTURE(stud_colr, refr_colr, deterministicBVH_HitPointSequence, lightColor);
            REQUIRE(stud_equal);
        }
    }
}
} // namespace test