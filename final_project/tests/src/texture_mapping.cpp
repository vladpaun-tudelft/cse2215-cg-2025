#include "tests.h"
#include "texture.h" // Include the student's code
#include <catch2/catch_approx.hpp>
#include "solution.h"

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples = 16; // Number of random samples taken in certain tests
std::string image_name = "emoji.png"; // Stupid, stupid, stupid. Who wrote the image class?!

TEST_CASE("Texture mapping")
{
    // Instantiate reference objects
    ref::Sampler sampler(4);

    // Load test image
    // Whoever wrote the Image class, I curse you to stub a toe. Why in the everloving
    // crap does it only have an explicit file constructor
    std::filesystem::path ddir = DATA_DIR;
    Image image(ddir / image_name);
    
    {   // sampleTextureNearest
        // Test check values
        bool is_stud_flipped = true;
        bool is_stud_correct_off_by_0 = true;
        bool is_stud_correct_off_by_1 = true;

        // Override test image data with expected pixel coordinate values
        // That way, we can use difference values to estimate offset err
        #pragma omp parallel for
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                image.set_pixel(y * image.width + x, glm::vec3(x, y, 0));
            }
        }

        // We test if y-coordinates are correctly flipped
        is_stud_flipped 
            // If y-coordinate is not flipped, this should return the first pixel
            &= !(epsEqual(image.get_pixel(0), sampleTextureNearest(image, { 0.0, 0.0 })))
            && !(epsEqual(image.get_pixel(image.width * image.height - 1), sampleTextureNearest(image, { 1.0, 1.0 })))
            // If y-coordinate is flipped, this should return the first pixel of the last column,
            // followed by the last pixel of the first row
            && (epsEqual(image.get_pixel((image.height - 1) * image.width), sampleTextureNearest(image, { 0.0, 0.0 })))
            && (epsEqual(image.get_pixel(image.width - 1), sampleTextureNearest(image, { 1.0, 1.0 })));

        // SECTION("sampleTextureNearest [Y-coordinate is flipped]")
        // {
        //     CHECK(is_stud_flipped); // Is the y-coordinate correctly flipped?
        // }

        // Select evaluate function dependent on flip/noflip
        auto eval = solution::sampleTextureNearest;
        
        // Verify student solution with samples
        glm::vec3 stud_colr_of_failed;
        glm::vec3 refr_colr_of_failed;
        glm::vec2 uv_of_failed;
        int refr_index_of_failed;
        for (uint32_t i = 0; i < num_samples; ++i) {
            auto uv = sampler.next_2d();
            glm::vec3 stud_colr = ::sampleTextureNearest(image, uv);
            auto [refr_index, refr_colr] = eval(image, uv, is_stud_flipped);

            bool correct_off_by_0 = (epsEqual(refr_colr, stud_colr));
            bool correct_off_by_1 = (epsEqual(refr_colr - stud_colr, glm::vec3(0), 1.f + eps_default));

            is_stud_correct_off_by_0 &= correct_off_by_0;
            is_stud_correct_off_by_1 &= correct_off_by_1;

            if (!correct_off_by_0) {
                stud_colr_of_failed = stud_colr;
                refr_colr_of_failed = refr_colr;
                uv_of_failed = uv;
                refr_index_of_failed = refr_index;
                if (!correct_off_by_1) {
                    break;
                }
            }
        }

        SECTION("sampleTextureNearest [Sample coordinate lies at pixel center]")
        {
            INFO("This test allows both, flipped and not flipped y coordinate, as long as one version is correctly implemented it passes.");
            INFO("In case you want to trace your mistake, the texture used for evaluation is defined as follows <<image[y * width + x] = vec3(x, y, 0)>> for unflipped, or <<image[(1-y) * width + x] = vec3(x, y, 0)>> for flipped.");
            INFO("The `refr_index_of_failed` is the expected index used to access the reference texture given `uv_of_failed`.");
            CAPTURE(uv_of_failed, refr_index_of_failed, stud_colr_of_failed, refr_colr_of_failed);
            CHECK(is_stud_correct_off_by_0);
        }

        SECTION("sampleTextureNearest [Sample coordinate lies within 0.5 of pixel center]")
        {
            INFO("This test allows both, flipped and not flipped y coordinate, as long as one version is correctly implemented it passes.");
            INFO("In case you want to trace your mistake, the texture used for evaluation is defined as follows <<image[y * width + x] = vec3(x, y, 0)>> for unflipped, or <<image[(1-y) * width + x] = vec3(x, y, 0)>> for flipped.");
            INFO("The `refr_index_of_failed` is the expected index used to access the reference texture given `uv_of_failed`.");
            CAPTURE(uv_of_failed, refr_index_of_failed, stud_colr_of_failed, refr_colr_of_failed);
            CHECK((is_stud_correct_off_by_0 || is_stud_correct_off_by_1));
        }
    }

    {   // sampleTextureBilinear
        // Test check values
        bool is_stud_flipped = true;
        bool is_stud_correct_off_by_0 = true;
        bool is_stud_correct_off_by_1 = true;

        // Override test image data with expected pixel coordinate values
        // That way, we can use difference values to estimate offset err
        #pragma omp parallel for
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                image.set_pixel(y * image.width + x, glm::vec3(x, y, 0));
            }
        }

        // We test if y-coordinates are correctly flipped
        is_stud_flipped 
            // If y-coordinate is not flipped, this should return the first pixel
            &= !(epsEqual(image.get_pixel(0), sampleTextureBilinear(image, { 0.0, 0.0 })))
            && !(epsEqual(image.get_pixel(image.width * image.height - 1), sampleTextureBilinear(image, { 1.0, 1.0 })))
            // If y-coordinate is flipped, this should return the first pixel of the last column,
            // followed by the last pixel of the first row
            && (epsEqual(image.get_pixel((image.height - 1) * image.width), sampleTextureBilinear(image, { 0.0, 0.0 })))
            && (epsEqual(image.get_pixel(image.width - 1), sampleTextureBilinear(image, { 1.0, 1.0 })));
        
        // SECTION("sampleTextureBilinear [Y-coordinate is flipped]")
        // {
        //     CHECK(is_stud_flipped); // Is the y-coordinate correctly flipped?
        // }

        // Select evaluate function dependent on flip/noflip
        auto eval = solution::sampleTextureBilinear;

        // Verify student solution with samples
        glm::vec3 stud_colr_of_failed;
        glm::vec3 refr_colr_of_failed;
        glm::vec2 uv_of_failed;
        for (uint32_t i = 0; i < num_samples; ++i) {
            auto uv = sampler.next_2d();
            glm::vec3 stud_colr = ::sampleTextureBilinear(image, uv);
            glm::vec3 refr_colr = eval(image, uv, is_stud_flipped);
            bool correct_off_by_0 = (epsEqual(refr_colr, stud_colr, 1e-4f));
            bool correct_off_by_1 = (epsEqual(refr_colr - stud_colr, glm::vec3(0), 1.f + eps_default));

            is_stud_correct_off_by_0 &= correct_off_by_0;
            is_stud_correct_off_by_1 &= correct_off_by_1;

            if (!correct_off_by_0) {
                stud_colr_of_failed = stud_colr;
                refr_colr_of_failed = refr_colr;
                uv_of_failed = uv;
                if (!correct_off_by_1) {
                    break;
                }
            }
        }

        SECTION("sampleTextureBilinear [Sample coordinate lies at pixel center]")
        {
            INFO("This test allows both, flipped and not flipped y coordinate, as long as one version is correctly implemented it passes.");
            INFO("In case you want to trace your mistake, the texture used for evaluation is defined as follows <<image[y * width + x] = vec3(x, y, 0)>> ");
            CAPTURE(uv_of_failed, stud_colr_of_failed, refr_colr_of_failed);
            CHECK(is_stud_correct_off_by_0);
        }

        SECTION("sampleTextureBilinear [Sample coordinate lies within 0.5 of pixel center]")
        {
            INFO("This test allows both, flipped and not flipped y coordinate, as long as one version is correctly implemented it passes.");
            INFO("In case you want to trace your mistake, the texture used for evaluation is defined as follows <<image[y * width + x] = vec3(x, y, 0)>> ");
            CAPTURE(uv_of_failed, stud_colr_of_failed, refr_colr_of_failed);
            CHECK((is_stud_correct_off_by_0 || is_stud_correct_off_by_1));
        }
    }
}
} // namespace test