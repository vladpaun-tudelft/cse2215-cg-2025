#include "tests.h"
#include "shading.h" // Include the student's code
#include "solution.h"

namespace test {

// Test settings
constexpr bool run_ref_tests = false; // Run REQUIRES(...) tests over the reference solution
constexpr uint32_t num_samples_i = 16; // Number of random samples taken in certain tests
constexpr uint32_t num_samples_j = 16; // Number of random samples taken in certain tests


struct TestCase {
    LinearGradient linear_gradient;
    float t;
    glm::vec3 stud_colr;
    glm::vec3 refr_colr;
};

inline std::ostream& operator<<(std::ostream& stream, const TestCase& test)
{
    stream << "TestCase {";
    stream << "linear_gradient: " << test.linear_gradient << ", ";
    stream << "t: " << test.t << ", ";
    stream << "stud_colr: " << test.stud_colr << ", ";
    stream << "refr_colr: " << test.refr_colr;
    stream << "}";
    return stream;
}


TEST_CASE("Shading models")
{
  // Instantiate reference objects
  ref::Sampler sampler(4);

  /* SECTION("computePhongModel")
  {
    // Not tested, allotted 0 points
  } */

  /* SECTION("computeBlinnPhongModel")
  {
    // Not tested, allotted 0 points
  } */

  { // LinearGradient.sample
    
    auto eval = solution::LinearGradientSample;

    { // LinearGradient.sample [Unsorted]
      // Test check values
      bool is_stud_minimum_extrema_correct = true;
      bool is_stud_maximum_extrema_correct = true;
      bool is_stud_interior_sample_correct = true;


      std::vector<TestCase> failed_inner = {};
      std::vector<TestCase> failed_max = {};
      std::vector<TestCase> failed_min = {};

      for (uint32_t i = 0; i < num_samples_i; ++i) {
        // Generate a random gradient of size [1-8]
        LinearGradient g; 
        g.components.resize(1 + static_cast<uint32_t>(sampler.next_1d() * 8));
        rng::generate(g.components, [&]() {
          return LinearGradient::Component { .t = sampler.next_1d(), .color = sampler.next_3d() };
        });
        LinearGradient logging_backup = g;

        glm::vec3 stud_minimum = g.sample(-1.f);
        glm::vec3 stud_maximum = g.sample(1.f);
        glm::vec3 refr_minimum = eval(g, -1.f);
        glm::vec3 refr_maximum = eval(g, 1.f);

        bool min_correct = (epsEqual(refr_minimum, stud_minimum));
        bool max_correct = (epsEqual(refr_maximum, stud_maximum));

        // Verify extrema samples
        is_stud_minimum_extrema_correct &= min_correct;
        is_stud_maximum_extrema_correct &= max_correct;

        // Verify random samples inside gradient
        float ti;
        glm::vec3 stud_inner;
        glm::vec3 refr_inner;
        bool inner_correct;

        for (uint32_t j = 0; j < num_samples_j; ++j) {
          ti = 2.f * sampler.next_1d() - 1.f;
          stud_inner = g.sample(ti);
          refr_inner = eval(g, ti);
          inner_correct = epsEqual(refr_inner, stud_inner);
          if (!inner_correct) {
              is_stud_interior_sample_correct = false;
              break;
          }
          // is_stud_interior_sample_correct &= inner_correct;
        }

        if (!inner_correct) {
            failed_inner.emplace_back(logging_backup, ti, stud_inner, refr_inner);
        }
        if (!min_correct) {
            failed_min.emplace_back(logging_backup, -1.f, stud_minimum, refr_minimum);
        }
        if (!max_correct) {
            failed_max.emplace_back(logging_backup, 1.f, stud_maximum, refr_maximum);
        }
      }

      TestCase failed_minus_1 = failed_min.empty() ? TestCase {} : failed_min.front();
      TestCase failed_positive_1 = failed_max.empty() ? TestCase {} : failed_max.front();
      TestCase failed_inner_ = failed_inner.empty() ? TestCase {} : failed_inner.front();
      // Verify student solution
      SECTION("LinearGradient.sample [Unsorted gradient data, sample is correct at boundaries -1/+1]")
      {
        CAPTURE(failed_minus_1);
        CHECK(is_stud_minimum_extrema_correct); // Is the sample corect at [-1]
        CAPTURE(failed_positive_1);
        CHECK(is_stud_maximum_extrema_correct); // Is the sample correct at [1]
      }
      SECTION("LinearGradient.sample [Unsorted gradient data; sample is correct inside range [-1, 1]]")
      {
        CAPTURE(failed_inner_);
        CHECK(is_stud_interior_sample_correct); // Is the sample correct in [-1, 1]
      }
    }
    
    { // LinearGradient.sample [Sorted]
      // Test check values
      bool is_stud_minimum_extrema_correct = true;
      bool is_stud_maximum_extrema_correct = true;
      bool is_stud_interior_sample_correct = true;

      std::vector<TestCase> failed_inner = {};
      std::vector<TestCase> failed_max = {};
      std::vector<TestCase> failed_min = {};

      for (uint32_t i = 0; i < num_samples_i; ++i) {
        // Generate a random gradient of size [1-8]
        LinearGradient g; 
        g.components.resize(1 + static_cast<uint32_t>(sampler.next_1d() * 8));
        rng::generate(g.components, [&]() {
          return LinearGradient::Component { .t = sampler.next_1d(), .color = sampler.next_3d() };
        });
        rng::sort(g.components, {}, &LinearGradient::Component::t);

        LinearGradient logging_backup = g;

        glm::vec3 stud_minimum = g.sample(-1.f);
        glm::vec3 stud_maximum = g.sample(1.f);
        glm::vec3 refr_minimum = eval(g, -1.f);
        glm::vec3 refr_maximum = eval(g, 1.f);

        bool min_correct = (epsEqual(refr_minimum, stud_minimum));
        bool max_correct = (epsEqual(refr_maximum, stud_maximum));

        // Verify extrema samples
        is_stud_minimum_extrema_correct &= min_correct;
        is_stud_maximum_extrema_correct &= max_correct;

        // Verify random samples inside gradient
        float ti;
        glm::vec3 stud_inner;
        glm::vec3 refr_inner;
        bool inner_correct;

        for (uint32_t j = 0; j < num_samples_j; ++j) {
          ti = 2.f * sampler.next_1d() - 1.f;
          stud_inner = g.sample(ti);
          refr_inner = eval(g, ti);
          inner_correct = epsEqual(refr_inner, stud_inner);
          if (!inner_correct) {
              is_stud_interior_sample_correct = false;
              break;
          }
          // is_stud_interior_sample_correct &= inner_correct;
        }

        if (!inner_correct) {
            failed_inner.emplace_back(logging_backup, ti, stud_inner, refr_inner);
        }
        if (!min_correct) {
            failed_min.emplace_back(logging_backup, -1.f, stud_minimum, refr_minimum);
        }
        if (!max_correct) {
            failed_max.emplace_back(logging_backup, 1.f, stud_maximum, refr_maximum);
        }
        // // Verify extrema samples
        // is_stud_minimum_extrema_correct &= (epsEqual(eval(g, -1.f), g.sample(-1.f)));
        // is_stud_maximum_extrema_correct &= (epsEqual(eval(g, 1.f), g.sample(1.f)));
        //
        // // Verify random samples inside gradient
        // for (uint32_t j = 0; j < num_samples_j; ++j) {
        //   float ti = 2.f * sampler.next_1d() - 1.f;
        //   is_stud_interior_sample_correct &= epsEqual(eval(g, ti), g.sample(ti));
        // }
      }

      TestCase failed_minus_1 = failed_min.empty() ? TestCase {} : failed_min.front();
      TestCase failed_positive_1 = failed_max.empty() ? TestCase {} : failed_max.front();
      TestCase failed_inner_ = failed_inner.empty() ? TestCase {} : failed_inner.front();
      // Verify student solution
      SECTION("LinearGradient.sample [Sorted gradient data, sample is correct at boundaries -1/+1]")
      {
        CAPTURE(failed_minus_1);
        CHECK(is_stud_minimum_extrema_correct); // Is the sample corect at [-1]
        CAPTURE(failed_positive_1);
        CHECK(is_stud_maximum_extrema_correct); // Is the sample correct at [1]
      }
      SECTION("LinearGradient.sample [Sorted gradient data; sample is correct inside range [-1, 1]]")
      {
        CAPTURE(failed_inner_);
        CHECK(is_stud_interior_sample_correct); // Is the sample correct in [-1, 1]
      }
    }
  }

  /* SECTION("computeLinearGradientModel")
  {
    // TODO; uncertain how to test this, as we do not have a properly defined solution r.n.
  } */
}
} // namespace test