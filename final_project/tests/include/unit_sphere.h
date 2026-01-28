#pragma once

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
DISABLE_WARNINGS_POP()
#include "ref/sampler.h"
#include <numbers>

namespace test::detail {
// Given a random vector in R3 bounded to [-1, 1], return a vector
// distributed over a gaussian distribution
inline auto inv_gaussian_cdf(const glm::vec3& x)
{
    auto y = glm::log(glm::max(-(x * x) + 1.f, 0.0001f));
    auto z = (0.5f * y + (2.f / std::numbers::pi_v<float>));
    return (glm::sqrt(glm::sqrt(z * z - y) - z) * glm::sign(x));
}

// Given a random vector in R3 bounded to [-1, 1], return a uniformly
// distributed point on the unit sphere
inline auto inv_unit_sphere_cdf(const auto& x)
{
    return glm::normalize(inv_gaussian_cdf(x));
}

// Given a sampler in [0, 1], return a uniformly distributed point on the unit sphere
inline glm::vec3 pointOnSphere(ref::Sampler& sampler)
{
    return inv_unit_sphere_cdf(sampler.next_3d() * 2.f - 1.f);
}
} // namespace test::detail