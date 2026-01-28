#include "tests.h"
// #include <Windows.h>
#include "render.h" // Include the student's code
#include "uniform_tests.h"
#include <framework/trackball.h>
#include <framework/window.h>
#include <memory>

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples = 16384; // Number of random samples taken in certain tests

constexpr uint32_t chi2_classes_x = 8; // 8*8=64 classes
constexpr float chi2_limit = 81.381f; // For 64 classes, 62 dof,  chi2 value to conclude
                                      // a non-uniform distribution with p=0.05. Should suffice

TEST_CASE("Multisampling")
{
    // Instantiate reference objects
    ref::Sampler sampler(4);

    // Instantiate a @#&^@*#$^&*@&^&$&* OpenGL context. Because the trackball needed one.
    // Really?! Really really?! Wars have been started over less than this
    std::string window_title = "String of curses";
    auto window_size = glm::ivec2(256);
    auto window_p = std::make_unique<Window>(window_title, window_size, OpenGLVersion::GL45, false);
    auto cam_fovy = 1.f;
    auto cam_lookAt = glm::vec3(0, 0, 1);
    auto cam_distanceFromLookAt = 1.f;
    auto cam_rotationX = 0.f;
    auto cam_rotationY = 0.f;
    auto camera_p = std::make_unique<Trackball>(window_p.get(), cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);

    // Set up relevant objects; scene/bvh **should not** be used, so we don't really init these
    Scene scene;
    ref::FakeBVH bvh(sampler, 6);
    Features features = {
        .enableShading = false,
        .enableReflections = false,
        .enableShadows = false,
        .enableAccelStructure = false,
        .enableTransparency = false
    };
    RenderState state = { .scene = scene, .features = features, .bvh = bvh, .sampler = { 4 } };

    // Helper data for taking a camera ray and recovering a [0, 1] sample representation
    glm::ivec2 px = { 7, 11 }, res = { 16, 16 };
    const auto half_screen_space_height = std::tan(1.f / 2.f);
    const auto half_screen_space_width = window_p->getAspectRatio() * half_screen_space_height;
    const auto f_ray_to_sample = [&](const Ray& r) {
        auto v = glm::vec2(
            r.direction.x / -half_screen_space_width,
            r.direction.y / half_screen_space_height);
        v = v / glm::dot(glm::vec3(0, 0, 1), r.direction);
        v = (v + 1.f) * 0.5f;
        v = (v * glm::vec2(res)) - glm::vec2(px);
        return v;
    };

    // Helper lambda for testing sample boundedness
    constexpr auto f_test_bounded = [](glm::vec2 v) { return glm::clamp(v, glm::vec2(0), glm::vec2(1)) == v; };

    // Chi2 fit tester, with null-hypothesis being that samples are uniformly distributed
    // and helper lambda to push 2d [0, 1] samples into specified bins
    detail::Chi2FitTest chi2_tester(chi2_classes_x * chi2_classes_x, chi2_limit);
    auto f_push_sample = [&chi2_tester](glm::vec2 v) {
        auto c
            = static_cast<uint32_t>(v.x * static_cast<float>(chi2_classes_x)) * chi2_classes_x
            + static_cast<uint32_t>(v.y * static_cast<float>(chi2_classes_x));
        if (c < chi2_tester.n_classes())
            chi2_tester.puti(c);
    };

    {   // generatePixelRaysUniform
        // Take a whopton of samples
        features.numPixelSamples = num_samples;

        // Generate student's ray samples, then recover sample's position inside [0, 1] rectangle
        auto stud_rays = ::generatePixelRaysUniform(state, *camera_p, px, res);
        auto stud_samples = stud_rays | vws::transform(f_ray_to_sample);

        // Verify that no samples step outside the pixel's unit rectangle
        SECTION("generatePixelRaysUniform [No samples lie outside unit rect]")
        {
            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            CHECK(rng::all_of(stud_samples, f_test_bounded));
        }

        // Do a chi2-test, with the null-hypothesis being that the samples are uniformly distributed
        SECTION("generatePixelRaysUniform [Samples follow a uniform distribution]")
        {
            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            rng::for_each(stud_samples, f_push_sample);
            CHECK(chi2_tester.test_chi2());
        }
    }

    {   // generatePixelRaysStratified
        // Take 1 sample per chi2 class;
        // this, by extension, tells us the samples are exactly distributed across the bins
        features.numPixelSamples = chi2_classes_x * chi2_classes_x;

        // Generate student's ray samples, then recover sample's position inside [0, 1] rectangle
        auto stud_rays = ::generatePixelRaysStratified(state, *camera_p, px, res);
        auto stud_samples = stud_rays | vws::transform(f_ray_to_sample);

        // Do a chi2-test;
        // if even one sample steps outside its allotted bin now, this instantly fails the test;
        // as the computed value could not be exactly 0
        SECTION("generatePixelRaysStratified [Samples each are in one stratified bin]")
        {
            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            rng::for_each(stud_samples, f_push_sample);
            CHECK(chi2_tester.chi2() == 0);
        }

        // Verify that students correctly split the ray counts, even though we gave them this
        SECTION("generatePixelRaysStratified [Correct nr. of x*y samples is returned]")
        {

            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            CHECK(features.numPixelSamples == static_cast<uint32_t>(stud_samples.size()));
        }

        // Verify that no samples step outside the pixel's unit rectangle
        SECTION("generatePixelRaysStratified [No samples lie outside unit rect]")
        {

            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            CHECK(rng::all_of(stud_samples, f_test_bounded));
        }

        SECTION("generatePixelRaysStratified [Samples follow a uniform distribution]")
        {
            // Now take many more samples
            features.numPixelSamples = num_samples;
            stud_rays = ::generatePixelRaysStratified(state, *camera_p, px, res);
            stud_samples = stud_rays | vws::transform(f_ray_to_sample);

            // Verify that no samples step outside the pixel's unit rectangle
            CHECK(rng::all_of(stud_samples, f_test_bounded));

            // Run a second chi2-test; as we have many more samples now, we do test for uniformity
            chi2_tester.clear();
            rng::for_each(stud_samples, f_push_sample);

            CAPTURE(stud_rays, features.numPixelSamples, px, res,
                window_size, cam_fovy, cam_lookAt, cam_distanceFromLookAt, cam_rotationX, cam_rotationY);
            CHECK(chi2_tester.test_chi2());
        }
    }

    // Tear down window and dependencies in hardcoded order. GLFW has a tendency to self-destruct
    // if you don't
    camera_p.reset();
    window_p.reset();
}
} // namespace test