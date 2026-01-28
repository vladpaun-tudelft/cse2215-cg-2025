#include "tests.h"
#include "uniform_tests.h"
#include "light.h" // Include the student's code
#include "shading.h" // Include the student's code
#include "bvh.h"
#include "solution.h"

#include <fmt/core.h>
#include <glm/gtx/string_cast.hpp>

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples = 2 * 65536; // Number of random samples taken in certain tests

TEST_CASE("Lights and shadows")
{
    // Instantiate reference objects
    ref::Sampler sampler(4);

    {   // sampleSegmentLight
        // Generate an arbitrary segment light
        SegmentLight light = {
            .endpoint0 = sampler.next_3d(),
            .endpoint1 = sampler.next_3d(),
            .color0 = sampler.next_3d(),
            .color1 = sampler.next_3d()
        };

        // Generate a set of 1d statistical bins
        detail::Uniform1dGridTest bins(2u + static_cast<uint32_t>(16 * sampler.next_1d()));

        // Recover 3d/1d samples from the student's implementation using inversion
        std::vector<float> samples = bins.samples() | std::ranges::to<std::vector>();

        std::vector<glm::vec3> position_stud {};
        std::vector<glm::vec3> color_stud {};
        for (float x : samples) {
            glm::vec3 p, c;
            ::sampleSegmentLight(x, light, p, c);
            position_stud.emplace_back(p);
            color_stud.emplace_back(c);
        }

        auto vec3_inv = position_stud | vws::transform([&](glm::vec3 p) {
            return (p - light.endpoint0) / (light.endpoint1 - light.endpoint0);
        }) | rng::to<std::vector>();
        auto vec1_inv = vec3_inv
            | vws::transform([](auto v) { return v.x; }) | rng::to<std::vector>();

        // Confirm the samples remain uniformly distributed according to the input sample distr
        SECTION("sampleSegmentLight [Samples remain uniformly distributed]")
        {
            CAPTURE(light.endpoint0, light.endpoint1, samples, position_stud);
            CHECK(bins.eval(vec1_inv));
        }

        // Confirm x/y/z components of samples are identical;
        // failure indicates the point does not lie on the line
        SECTION("sampleSegmentLight [Sampled point lies on segment line]")
        {
            CAPTURE(light.endpoint0, light.endpoint1, samples, position_stud);
            bool is_on_line = rng::all_of(vec3_inv,
                [](auto v) { return epsEqual(v.x, v.y) && epsEqual(v.y, v.z); });
            CHECK(is_on_line);
        }

        // Confirm inverted samples remain between [0, 1];
        // failure indicates the returned point does not lie on the segment
        SECTION("sampleSegmentLight [Sampled point lies inside segment line]")
        {
            CAPTURE(light.endpoint0, light.endpoint1, samples, position_stud);
            bool is_in_line = rng::all_of(vec1_inv,
                [](auto v) { return epsEqual(v, glm::clamp(v, 0.f, 1.f)); });
            CHECK(is_in_line);
        }

        // Confirm the sampled positions match the sampled colors exactly
        // failure indicates color mixing is incorrectly implemented
        SECTION("sampleSegmentLight [Sampled color is correctly mixed]")
        {
            bool is_color_correct = true;
            std::vector<glm::vec3> reference_color_for_student_position {};
            for (int i=0; i < samples.size(); i++) {
                auto c = solution::mixSegmentLightColor(light, position_stud[i]);
                reference_color_for_student_position.emplace_back(c);
                is_color_correct &= epsEqual(c, color_stud[i]);
            }

            CAPTURE(light.endpoint0, light.endpoint1, samples, position_stud, color_stud, reference_color_for_student_position);
            CHECK(is_color_correct);
        }
    }

    {   // sampleParallelogramLight
        // Generate an arbitrary parallelogram light
        ParallelogramLight light = {
            .v0 = sampler.next_3d(),
            .edge01 = detail::pointOnSphere(sampler),
            .edge02 = detail::pointOnSphere(sampler),
            .color0 = sampler.next_3d(),
            .color1 = sampler.next_3d(),
            .color2 = sampler.next_3d(),
            .color3 = sampler.next_3d()
        };

        // Generate a set of 2d statistical bin samples
        detail::Uniform2dGridTest bins(2u + static_cast<uint32_t>(16 * sampler.next_1d()));

        // Generate an inverse transformation for the parallelogram's points/colors
        glm::mat3 parallel_inv = glm::inverse(glm::mat3 {
            light.edge01,
            light.edge02,
            glm::cross(light.edge01, light.edge02) });

        auto samples = bins.samples() | rng::to<std::vector>();

        std::vector<glm::vec3> position_stud {};
        std::vector<glm::vec3> color_stud {};
        for (glm::vec2 x : samples) {
            glm::vec3 p, c;
            ::sampleParallelogramLight(x, light, p, c);
            position_stud.emplace_back(p);
            color_stud.emplace_back(c);
        }

        // Recover 2d/3d samples from the inverse of the student's implementation
        auto vec3_inv = position_stud | vws::transform([&](glm::vec3 p) {
            return parallel_inv * (p - light.v0);
        }) | rng::to<std::vector>();

        auto vec2_inv = vec3_inv
            | vws::transform([](auto v) { return glm::vec2(v); }) | rng::to<std::vector>();

        // Confirm the samples remain uniformly distributed according to the input sample distr
        SECTION("sampleParallelogramLight [Samples remain uniformly distributed]")
        {
            CAPTURE(light.v0, light.edge01, light.edge02, samples, position_stud);
            CHECK(bins.eval(vec2_inv));
        }

        // Confirm the z-value remains zero;
        // failure indicates the samples do not lie on the parallelogram's plane, but above it
        SECTION("sampleParallelogramLight [Sampled point lies on parallelogram plane]")
        {
            CAPTURE(light.v0, light.edge01, light.edge02, samples, position_stud);
            bool is_on_plane = rng::all_of(vec3_inv,
                [](auto v) { return epsEqual(v.z, 0.f); });
            CHECK(is_on_plane);
        }

        // Confirm inverted samples remain between [0, 1];
        // failure indicates the returned point does not lie on the parallelogram, but outside it
        SECTION("sampleParallelogramLight [Sampled point lies inside parallelogram plane]")
        {
            CAPTURE(light.v0, light.edge01, light.edge02, samples, position_stud);
            bool is_in_plane = rng::all_of(vec2_inv,
                [](auto v) { return epsEqual(v, glm::clamp(v, glm::vec2(0), glm::vec2(1))); });
            CHECK(is_in_plane);
        }

        // Confirm the sampled positions match the sampled colors exactly;
        // failure indicates color mixing is incorrectly implemented
        SECTION("sampleParallelogramLight [Sampled color is correctly mixed]")
        {
            bool is_color_correct = true;
            std::vector<glm::vec3> reference_color_for_student_position {};
            for (int i = 0; i < samples.size(); i++) {
                auto c_ = solution::mixParallelogramLightColor(light, position_stud[i], parallel_inv);
                reference_color_for_student_position.emplace_back(c_);
                is_color_correct &= epsEqual(c_, color_stud[i]);
            }

            CAPTURE(light.v0, light.edge01, light.edge02, samples, position_stud, color_stud, reference_color_for_student_position);
            CHECK(is_color_correct);
        }
    }

    {   // testVisibilityLightSampleBinary
        // Construct a test scene;
        // Several triangles in parallel, with two triangles extremely close
        Scene scene;
        for (auto offs : { 0.f, eps_default, 1.f }) {
            Mesh m = { .vertices = {
                           { .position = { 1.f, 1.f, offs }, .normal = {}, .texCoord = {} },
                           { .position = { 1.f, -1.f, offs }, .normal = {}, .texCoord = {} },
                           { .position = { -1.f, 1.f, offs }, .normal = {}, .texCoord = {} } },
                .triangles = { { 0, 1, 2 } },
                .material = {
                    .kd = sampler.next_3d(),
                    .ks = sampler.next_3d(),
                    .shininess = sampler.next_1d(),
                    .transparency = sampler.next_1d(),
                } };
            scene.meshes.push_back(m);
        }

        // Add a fourth, random triangle, that no test should interact with;
        // if students somehow interact with this triangle during the test, they're doing
        // something really weird
        scene.meshes.push_back({ .vertices = {
                                     { .position = { 1.f, 1.f, 100.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 1.f, -1.f, 100.f }, .normal = {}, .texCoord = {} },
                                     { .position = { -1.f, 1.f, 100.f }, .normal = {}, .texCoord = {} } },
            .triangles = { { 0, 1, 2 } },
            .material = {
                .kd = sampler.next_3d(),
                .ks = sampler.next_3d(),
                .shininess = sampler.next_1d(),
                .transparency = sampler.next_1d(),
            } });

        // Set up necessary objects for coming tests
        Features features = {
            .enableShading = false,
            .enableReflections = true,
            .enableShadows = true,
            .enableAccelStructure = false,
            .enableTransparency = false
        };
        BVH bvh = { scene, features };
        RenderState state = {
            .scene = scene,
            .features = features,
            .bvh = bvh,
            .sampler = { 4 }
        };

        glm::vec3 light_pos1 {0, 0, 0.9f};
        glm::vec3 light_pos2 { 0, 0, 1.1f };
        glm::vec3 light_pos3 { 0, 0, 0.5f };
        glm::vec3 light_color (1);

        SECTION("testVisibilityLightSampleBinary [Visibility test from front of a triangle stack]")
        {
            // Generate valid intersection data somewhere in space
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, 1 } };


            bool is_hit_pos1 = ::visibilityOfLightSampleBinary(state, light_pos1, light_color, ray, hit);
            bool is_hit_pos2 = ::visibilityOfLightSampleBinary(state, light_pos2, light_color, ray, hit);

            CAPTURE(scene, light_pos1, light_pos2, ray.origin, ray.direction, ray.t, hit.normal);

            // Confirm that yes, the light is visible
            CHECK(is_hit_pos1);

            // Confirm that no, the light is not visible
            CHECK_FALSE(is_hit_pos2);
        }

        SECTION("testVisibilityLightSampleBinary [Visibility test from back of a triangle stack]")
        {
            // Generate valid intersection data behind a triangle
            Ray ray = { .origin = { 0, 0, 1.5f }, .direction = { 0, 0, -1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, -1 } };

            bool is_hit_pos1 = ::visibilityOfLightSampleBinary(state, light_pos1, light_color, ray, hit);
            bool is_hit_pos2 = ::visibilityOfLightSampleBinary(state, light_pos2, light_color, ray, hit);

            CAPTURE(scene, light_pos1, light_pos2, ray.origin, ray.direction, ray.t, hit.normal);

            // Confirm that no, the light is not visible
            CHECK_FALSE(is_hit_pos1);

            // Confirm that yes, the light is visible
            CHECK(is_hit_pos2);
        }

        SECTION("testVisibilityLightSampleBinary [Self-intersection is handled from on a triangle]")
        {
            // Generate valid intersection data exactly on the first triangle
            Ray ray = { .origin = { 0, 0, 0 }, .direction = { 0, 0, 1 }, .t = 0.f };
            HitInfo hit = { .normal = { 0, 0, 1 } };

            CAPTURE(scene, light_pos3, ray.origin, ray.direction, ray.t, hit.normal);
            // Confirm that no, the light is not visible
            bool is_visible = ::visibilityOfLightSampleBinary(state, light_pos3, light_color, ray, hit);
            CHECK_FALSE(is_visible);

            // Offset by epsilon to the second triangle, which is rather close
            ray.origin = { 0, 0, eps_default };

            CAPTURE(scene, light_pos3, ray.origin, ray.direction, ray.t, hit.normal);
            // Confirm that yes, the light is visible
            is_visible = ::visibilityOfLightSampleBinary(state, light_pos3, light_color, ray, hit);
            CHECK(is_visible);
        }
    }

    {   // testVisibilityLightSampleTransparency
        // Construct a test scene;
        // Three triangles in order, each transparent with a color
        Scene scene;
        for (auto offs : { 1.f, 5.f, 10.f }) {
            Mesh m = { .vertices = {
                           { .position = { 1.f, 1.f, offs }, .normal = {}, .texCoord = {} },
                           { .position = { 1.f, -1.f, offs }, .normal = {}, .texCoord = {} },
                           { .position = { -1.f, 1.f, offs }, .normal = {}, .texCoord = {} } },
                .triangles = { { 0, 1, 2 } },
                .material = {
                    .kd = sampler.next_3d(),
                    .ks = sampler.next_3d(),
                    .shininess = sampler.next_1d(),
                    .transparency = sampler.next_1d(),
                } };
            scene.meshes.push_back(m);
        }

        // Add a fourth, random triangle, that is perfectly opaque, and always behind 
        // the light source.
        // If students somehow interact with this triangle during the test, they're doing
        // something really weird 
        scene.meshes.push_back({ .vertices = {
                                     { .position = { 1.f, 1.f, 20.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 1.f, -1.f, 20.f }, .normal = {}, .texCoord = {} },
                                     { .position = { -1.f, 1.f, 20.f }, .normal = {}, .texCoord = {} } },
            .triangles = { { 0, 1, 2 } },
            .material = {
                .kd = sampler.next_3d(),
                .ks = sampler.next_3d(),
                .shininess = sampler.next_1d(),
                .transparency = 1.0,
            } });

        // Add a fifth, random triangle, that no test should interact with;
        // if students somehow interact with this triangle during the test, they're doing
        // something really weird
        scene.meshes.push_back({ .vertices = {
                                     { .position = { 1.f, 1.f, 100.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 1.f, -1.f, 100.f }, .normal = {}, .texCoord = {} },
                                     { .position = { -1.f, 1.f, 100.f }, .normal = {}, .texCoord = {} } },
            .triangles = { { 0, 1, 2 } },
            .material = {
                .kd = sampler.next_3d(),
                .ks = sampler.next_3d(),
                .shininess = sampler.next_1d(),
                .transparency = sampler.next_1d(),
            } });

        // Set up necessary objects for coming tests
        Features features = {
            .enableShading = false,
            .enableReflections = false,
            .enableShadows = true,
            .enableAccelStructure = false, // No BVH! don't use the student solution here
            .enableTransparency = true
        };
        BVH bvh = { scene, features };
        RenderState state = {
            .scene = scene,
            .features = features,
            .bvh = bvh,
            .sampler = { 4 }
        };

        // View over throughput of first three triangles
        auto throughput = scene.meshes | vws::take(3) | vws::transform([](const auto& mesh) {
            return mesh.material.kd * (1.f - mesh.material.transparency);
        });

        SECTION("testVisibilityLightSampleTransparency [Visibility test through a triangle stack]")
        {
            // Generate valid intersection data somewhere in space
            // Put ray on one side, light on other
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, 1 } };
            glm::vec3 light_colr = sampler.next_3d();
            glm::vec3 light_pos = { 0.f, 0.f, 15.f };

            // Generate student solution
            auto stud_colr
                = ::visibilityOfLightSampleTransparency(state, light_pos, light_colr, ray, hit);

            // Generate reference solution, following specified blending equation:
            // Li = kd * (1 - alpha) * (kd * (1 - alpha) * (... * (Lo)))
            glm::vec3 refr_colr = rng::fold_left(
                throughput, light_colr, std::multiplies<>());

            CAPTURE(scene, light_pos, light_colr, ray.origin, ray.direction, ray.t, hit.normal, stud_colr, refr_colr);
            // Verify solution's blending correctness
            CHECK(epsEqual(stud_colr, refr_colr));
        }

        SECTION("testVisibilityLightSampleTransparency [Visibility test halfway through a triangle stack]")
        {
            // Generate valid intersection data somewhere in space
            // Put ray on one side, light on other
            Ray ray = { .origin = { 0, 0, 15.f }, .direction = { 0, 0, -1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, 1 } };
            glm::vec3 light_colr = sampler.next_3d();
            glm::vec3 light_pos = { 0.f, 0.f, 2.5f };

            // Generate student solution
            auto stud_colr
                = ::visibilityOfLightSampleTransparency(state, light_pos, light_colr, ray, hit);

            // Generate reference solution, following specified blending equation,
            // while dropping the very first/last triangles (which is behind the light)
            // Li = kd * (1 - alpha) * (kd * (1 - alpha) * (... * (Lo)))
            glm::vec3 refr_colr = rng::fold_left(
                throughput | vws::drop(1), light_colr, std::multiplies<>());

            CAPTURE(scene, light_pos, light_colr, ray.origin, ray.direction, ray.t, hit.normal, stud_colr, refr_colr);
            // Verify solution's blending correctness;
            // if this test fails, the student's ray did not stop at the light source!
            CHECK(epsEqual(stud_colr, refr_colr));
        }

        SECTION("testVisibilityLightSampleTransparency [Visibility test with a blocker, no blending]")
        {
            // Third triangle is now a blocker
            scene.meshes.at(2).material.transparency = 1.f;

            // Generate valid intersection data somewhere in space
            // Put ray on one side, light behind blocker
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, 1 } };
            glm::vec3 light_colr = sampler.next_3d();
            glm::vec3 light_pos = { 0.f, 0.f, 15.f };

            // Generate student solution
            auto stud_colr
                = ::visibilityOfLightSampleTransparency(state, light_pos, light_colr, ray, hit);

            auto refr_colr = glm::vec3(0);
            CAPTURE(scene, light_pos, light_colr, ray.origin, ray.direction, ray.t, hit.normal, stud_colr, refr_colr);
            // Verify solution's correctness; light should not be visible
            CHECK(epsEqual(stud_colr, refr_colr));
        }
    }

    {   // Contribution tests are nested to reuse this scene
        // Construct a test scene;
        // Only one quad of size 1 at distance 1, just to keep the math simple
        Scene scene;
        scene.meshes.push_back({ .vertices = {
                                     { .position = { 0.f, 0.f, 1.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 0.f, 1.f, 1.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 1.f, 0.f, 1.f }, .normal = {}, .texCoord = {} },
                                     { .position = { 1.f, 1.f, 1.f }, .normal = {}, .texCoord = {} } },
            .triangles = { { 0, 1, 2 }, { 1, 2, 3 } },
            .material = {
                .kd = glm::vec3(1),
                .ks = glm::vec3(0),
                .shininess = 0.f,
                .transparency = 1.f,
            } });

        // Set up necessary objects for coming tests
        Features features = {
            .enableShading = false,
            .enableReflections = false,
            .enableShadows = true,
            .enableAccelStructure = false, // NO BVH! Don't trust the student solution here
            .enableTransparency = false
        };
        BVH bvh = { scene, features };
        RenderState state = {
            .scene = scene,
            .features = features,
            .bvh = bvh,
            .sampler = { 4 }
        };

        // Offset of lights behind objects
        constexpr float eps_offset = 1e-5f;

        SECTION("computeContributionPointLight")
        {
            auto kd_colr = sampler.next_3d();
            auto li_colr = sampler.next_3d();

            // Generate valid intersection data somewhere in space
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = {
                .normal = { 0, 0, 1 },
                .material = { .kd = kd_colr }
            };

            SECTION("[Visible]")
            {
                glm::vec3 light_pos = { 0.5f, 0.5f, .5f };
                // Confirm li * kd is returned for a visible light
                // (this is the default for non-shaded solutions)
                auto stud_colr_visible = ::computeContributionPointLight(
                    state, PointLight { .position = light_pos, .color = li_colr }, ray, hit);

                auto refr_colr = li_colr * ::sampleMaterialKd(state, hit);
                CAPTURE(scene, light_pos, li_colr, ray.origin, ray.direction, ray.t, hit.normal, stud_colr_visible, refr_colr);
                CHECK(epsEqual(refr_colr, stud_colr_visible));
            }

            SECTION("[Invisible]")
            {
                glm::vec3 light_pos = { 0.5f, 0.5f, 1.f + eps_default };
                // Confirm 0, 0, 0 is returned for an invisible light
                auto stud_colr_invisible = ::computeContributionPointLight(
                    state, PointLight { .position = light_pos, .color = li_colr }, ray, hit);

                auto refr_colr = glm::vec3(0);
                CAPTURE(scene, light_pos, li_colr, ray.origin, ray.direction, ray.t, hit.normal, stud_colr_invisible, refr_colr);
                CHECK(epsEqual(refr_colr, stud_colr_invisible));
            }
        }

        SECTION("computeContributionSegmentLight")
        {
            auto kd_colr = sampler.next_3d();
            auto li_colr = sampler.next_3d();

            // Generate valid intersection data somewhere in space
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = { .normal = { 0, 0, 1 }, .material = { .kd = kd_colr } };

            SECTION("[Fully visible]")
            {
                // Confirm a fully visible segment light results in li * kd
                SegmentLight light_visible = {
                    .endpoint0 = { -.5f, 0, .5f },
                    .endpoint1 = { 1.5f, 0, .5f },
                    .color0 = li_colr,
                    .color1 = li_colr
                };
                auto stud_colr_full = ::computeContributionSegmentLight(state, light_visible, ray, hit, num_samples);
                auto refr_colr_full = li_colr * ::sampleMaterialKd(state, hit);

                CAPTURE(scene, light_visible.endpoint0, light_visible.endpoint1, light_visible.color0, light_visible.color1,
                    ray.origin, ray.direction, ray.t, hit.normal,
                    stud_colr_full, refr_colr_full);
                CHECK(epsEqual(refr_colr_full, stud_colr_full));
            }

            SECTION("[Half visible]")
            {
                // Confirm a 50% visible segment light results in 0.5 * li * kd
                SegmentLight light_half = {
                    .endpoint0 = { -.5f, 0, 1.f + eps_offset },
                    .endpoint1 = { 1.5f, 0, 1.f + eps_offset },
                    .color0 = li_colr,
                    .color1 = li_colr
                };
                auto stud_colr_half = ::computeContributionSegmentLight(state, light_half, ray, hit, num_samples);
                auto refr_colr_half = 0.5f * li_colr * ::sampleMaterialKd(state, hit);

                CAPTURE(scene, light_half.endpoint0, light_half.endpoint1, light_half.color0, light_half.color1,
                    ray.origin, ray.direction, ray.t, hit.normal,
                    stud_colr_half, refr_colr_half);

                CHECK(epsEqual(refr_colr_half, stud_colr_half));
            }
        }

        SECTION("computeContributionParallelogramLight")
        {
            auto kd_colr = sampler.next_3d();
            auto li_colr = sampler.next_3d();

            // Generate valid intersection data somewhere in space
            Ray ray = { .origin = glm::vec3(0.f), .direction = { 0, 0, 1 }, .t = .1f };
            HitInfo hit = {
                .normal = { 0, 0, 1 },
                .material = { .kd = kd_colr }
            };

            SECTION("[Fully visible]")
            {
                // Confirm a fully visible segment light results in (li * kd)
                ParallelogramLight light_visible = {
                    .v0 = { 0, 0, .5f },
                    .edge01 = { 2, 0, 0 },
                    .edge02 = { 0, 2, 0 },
                    .color0 = li_colr,
                    .color1 = li_colr,
                    .color2 = li_colr,
                    .color3 = li_colr
                };
                auto stud_colr_full = ::computeContributionParallelogramLight(state, light_visible, ray, hit, num_samples);
                auto refr_colr_full = li_colr * ::sampleMaterialKd(state, hit);

                CAPTURE(scene, light_visible.v0, light_visible.edge01, light_visible.edge02, light_visible.color0, light_visible.color1, light_visible.color2, light_visible.color3,
                    ray.origin, ray.direction, ray.t, hit.normal,
                    stud_colr_full, refr_colr_full);
                CHECK(epsEqual(refr_colr_full, stud_colr_full));
            }

            SECTION("[75% visible]")
            {
                // A correct solution converges, for this 75% visible segment light, to (0.75 * li * kd)
                ParallelogramLight light_half = {
                    .v0 = { 0, 0, 1.f + eps_offset },
                    .edge01 = { 2, 0, 0 },
                    .edge02 = { 0, 2, 0 },
                    .color0 = li_colr,
                    .color1 = li_colr,
                    .color2 = li_colr,
                    .color3 = li_colr
                };
                auto refr_colr_half = 0.75f * li_colr * ::sampleMaterialKd(state, hit);
                auto stud_colr_half = ::computeContributionParallelogramLight(state, light_half, ray, hit, num_samples);

                CAPTURE(scene, light_half.v0, light_half.edge01, light_half.edge02, light_half.color0, light_half.color1, light_half.color2, light_half.color3,
                    ray.origin, ray.direction, ray.t, hit.normal,
                    stud_colr_half, refr_colr_half);
                CHECK(epsEqual(refr_colr_half, stud_colr_half));
            }
        }
    }
}
} // namespace test