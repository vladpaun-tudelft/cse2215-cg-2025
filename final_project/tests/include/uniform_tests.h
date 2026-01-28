#pragma once

#include "tests.h"
#include <ranges>

namespace test::detail {

// Uniform distribution test
// Tests the uniformity of a distribution of 1D samples, given a specified
// nr. of `k` classes across which those samples are distributed. This
// follows as an implemnetation of the chi-square test.
class Chi2FitTest {
    std::vector<uint32_t> m_classes;
    float m_chi2_value;

public:
    Chi2FitTest() = default;

    Chi2FitTest(uint32_t k, float chi2_value)
        : m_classes(static_cast<size_t>(k), 0u)
        , m_chi2_value(chi2_value)
    {
        // ...
    }

    // Reset sample counts in all classes
    void clear()
    {
        rng::fill(m_classes, 0u);
    }

    // Put an observation i into its specified class
    void puti(uint32_t i)
    {
        m_classes[i]++;
    }

    // Put an observation in [0, 1] into its specified class, given normalization to [0, k-1]
    void putf(float f)
    {
      m_classes[static_cast<uint32_t>(f * static_cast<float>(m_classes.size()))]++;
    }

    float chi2() const {
    //   auto n = *rng::fold_left_first(m_classes, std::plus {});
      auto n = std::accumulate(m_classes.begin(), m_classes.end(), 0u);
      auto k = m_classes.size();

      // Expectance for each class is the nr. of samples over the nr. of classes,
      // indicating a uniform distribution
      auto e = static_cast<float>(n) / static_cast<float>(k);

      float chi2 = 0.f;
      for (size_t i = 0; i < k; ++i) {
        auto o = static_cast<float>(m_classes[i]);
        chi2 += ((o - e) * (o - e)) / e;
      }
      return chi2;
    }

    bool test_chi2() const {
      return chi2() < m_chi2_value;
    }

    uint32_t n_classes() {
      return static_cast<uint32_t>(m_classes.size());
    }
};

// Testing helper tool for uniformity testing of 1d sample-use
// Generates evenly spaced samples in [0, 1], s.t. a transformation
// of these samples, if linear/affine, results in a potentially
// shuffled but equally evenly spaced set of other samples.
// If this spacing is not preserved, this indicates some non-uniformity,
// or at best extremely weird code.
class Uniform1dGridTest {
    std::vector<float> m_samples;

public:
    Uniform1dGridTest() = default;

    Uniform1dGridTest(uint32_t n_bins)
        : m_samples(static_cast<size_t>(n_bins), 0.f)
    {
        // Distribution necessitates 2 or more samples
        assert(n_bins > 1);

        // Generate a distribution s.t. values are uniformly spaced in [0, 1]
        rng::transform(m_samples.begin(), m_samples.end() - 1, m_samples.begin() + 1,
            [s = 1.f / static_cast<float>(n_bins)](float x) { return x + s; });
    }

    [[nodiscard]] std::span<const float> samples() const { return m_samples; }

    // Compare another set of samples in some independent order to the sorted samples
    bool eval(std::span<const float> X_)
    {
        assert(X_.size() == m_samples.size());

        // Generate a 0-1 sorted set of samples
        std::vector<float> X(X_.begin(), X_.end());
        rng::sort(X);

        // Verify that distances between all samples equal the uniformly spaced input set,
        // and that all distances are positive
        float s = 1.f / static_cast<float>(m_samples.size());
        bool is_eps = true;
        for (size_t i = 1; i < X.size(); ++i)
            is_eps &= epsEqual(s, X[i] - X[i - 1]);

        return is_eps;
    }
};

// Testing helper tool for uniformity testing of 2d sample-use:
// Generate evenly spaced samples in [0, 1], s.t. a transformation
// of these samples, if linear/affine, results in a potentially
// shuffled but equally evenly spaced set of other samples.
// If this spacing is not preserved, this indicates some non-uniformity,
// or at best extremely weird code.
class Uniform2dGridTest {
    float m_dist;
    std::vector<glm::vec2> m_samples;

public:
    Uniform2dGridTest() = default;

    Uniform2dGridTest(uint32_t n_bins_xy)
        : m_samples(static_cast<size_t>(n_bins_xy * n_bins_xy), glm::vec2(0.f))
        , m_dist(1.f / static_cast<float>(n_bins_xy))
    {
        // Distribution necessitates 2 or more samples
        assert(n_bins_xy > 1);

        // Generate a distribution s.t. values are uniformly spaced in [0, 1]
        rng::transform(m_samples.begin(), m_samples.end() - 1, m_samples.begin() + 1,
            [s = m_dist](glm::vec2 xy) {
                if (epsEqual(xy.x + s, 1.f))
                    return glm::vec2(0, xy.y + s); // Return to start, one line up
                else
                    return glm::vec2(xy.x + s, xy.y); // One line right
            });
    }

    [[nodiscard]] std::span<const glm::vec2> samples() const { return m_samples; }

    // Compare another set of samples in some independent order to the sorted samples
    bool eval(std::span<const glm::vec2> X_)
    {
        assert(X_.size() == m_samples.size());

        // Generate a 0-1 sorted set of samples
        std::vector<glm::vec2> X(X_.begin(), X_.end());
        rng::sort(X, [](const glm::vec2& a, const glm::vec2& b) {
            return epsEqual(a.y, b.y) ? a.x < b.x : a.y < b.y;
        });

        // Verify that distances between all samples equal the uniformly spaced input set,
        // and that all distances are positive
        bool is_eps = true;
        for (size_t i = 1; i < X.size(); ++i)
            is_eps &= (epsEqual(m_dist, X[i].x - X[i - 1].x) || epsEqual(m_dist, X[i].y - X[i - 1].y));

        return is_eps;
    }
};
} // namespace test::detail