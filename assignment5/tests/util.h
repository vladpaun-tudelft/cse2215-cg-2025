#ifndef COMPUTERGRAPHICS_UTIL_H
#define COMPUTERGRAPHICS_UTIL_H


#pragma once

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
#include <glm/gtx/component_wise.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
DISABLE_WARNINGS_POP()

// Shared includes
#include <ranges>
#include <numbers>

namespace test {
// Namespace shorthands used across tests
namespace rng = std::ranges;
namespace vws = std::views;

// Default (rather lenient) fuzziness used for floating point error
constexpr float eps_default = 1e-2f;

inline
bool epsEqual(const float& a, const float& b, float eps = eps_default)
{
    return glm::epsilonEqual(a, b, eps);
}

inline
bool epsEqual(const glm::vec2& a, const glm::vec2& b, float eps = eps_default)
{
    return glm::all(glm::epsilonEqual(a, b, eps));
}

inline
bool epsEqual(const glm::vec3& a, const glm::vec3& b, float eps = eps_default)
{
    return glm::all(glm::epsilonEqual(a, b, eps));
}

inline
bool epsEqual(const glm::vec4& a, const glm::vec4& b, float eps = eps_default)
{
    return glm::all(glm::epsilonEqual(a, b, eps));
}

inline
bool epsEqual(const AxisAlignedBox& a, const AxisAlignedBox& b, float eps = eps_default)
{
    return epsEqual(a.lower, b.lower, eps) && epsEqual(a.upper, b.upper, eps);
}

inline
bool epsEqual(const BVHInterface::Node& a, const BVHInterface::Node& b, float eps = eps_default)
{
    return epsEqual(a.aabb, b.aabb, eps)
        && rng::equal(a.data, b.data);
}

inline
bool epsEqual(const Vertex& a, const Vertex& b, float eps = eps_default)
{
    return epsEqual(a.position, b.position, eps)
        && epsEqual(a.normal, b.normal, eps)
        && epsEqual(a.texCoord, b.texCoord, eps);
}

inline
bool epsEqual(const BVHInterface::Primitive& a, const BVHInterface::Primitive& b, float eps = eps_default)
{
    return a.meshID == b.meshID
        && epsEqual(a.v0, b.v0, eps)
        && epsEqual(a.v1, b.v1, eps)
        && epsEqual(a.v2, b.v2, eps);
}

namespace detail {
    // Compile-time sized stack for help during BVH traversal
    template <typename T, size_t size>
    class FixedStack {
        static_assert(std::is_trivially_constructible_v<T>);
        std::array<T, size> m_stack;
        size_t m_stackIdx = 0;

    public:
        inline void push(T v)
        {
            if (m_stackIdx + 1 == size) {
                throw std::runtime_error("You exceeded the maximum stack size");
            }
            m_stack[m_stackIdx++] = v;
        }
        inline T pop()
        {
            return m_stack[--m_stackIdx];
        }

        inline bool empty()
        {
            return m_stackIdx == 0;
        }
    };

    // Helper method for intersecting a ray with a BVH 's AABB
    inline
    bool intersectRayWithAABBVolume(const AxisAlignedBox& box, const Ray& ray, float& intersectionDistance)
    {
        // Division by zero in C++ is undefined. However if we know that the platform uses IEE754 (IEC559) then it is guaranteed
        // that division by zero will result in -infinity / +infinity (and not NaN).
        static_assert(std::numeric_limits<float>::is_iec559);

        // Placed inside function despite the significant overhead, to prevent passing student-unnecessary parameters
        glm::vec3 invRayDirection = 1.f / ray.direction;

        // Much faster than glm::min(t1, t2) / glm::max(t1, t2) because we don't need to explicitely handle the edge case where direction == 0.
        // Implementing the swap with glm (glm::lessThan() + glm::mix()) was significantly slower then the current method.
        glm::vec3 t1 = (box.lower - ray.origin) * invRayDirection;
        glm::vec3 t2 = (box.upper - ray.origin) * invRayDirection;
        if (invRayDirection.x < 0)
            std::swap(t1.x, t2.x);
        if (invRayDirection.y < 0)
            std::swap(t1.y, t2.y);
        if (invRayDirection.z < 0)
            std::swap(t1.z, t2.z);

        const float tmin = glm::compMax(t1);
        const float tmax = glm::compMin(t2);
        intersectionDistance = tmin;
        return !(tmin > tmax || tmax < 0.0f || tmin > ray.t);
    }
} // namespace detail
} // namespace test


namespace ref {
    class Sampler {
        uint32_t m_state;

        uint32_t pcg_hash(uint32_t& state)
        {
            state = state * 747796405u + 2891336453u;
            uint32_t v = state;
            v ^= v >> ((v >> 28u) + 4u);
            v *= 277803737u;
            v ^= v >> 22u;
            return v;
        }

    public:
        // Seeded constructor, by default draws from std::random_device
        Sampler(uint32_t seed = std::random_device()())
            : m_state(seed)
        {
            // ...
        }

        // Draw a 1d sample in [a, b]
        float next_1d()
        {
            return static_cast<float>(pcg_hash(m_state)) / 4294967295.f;
        }

        // Draw a 2d sample in [a, b]
        glm::vec2 next_2d()
        {
            return { next_1d(), next_1d() };
        }

        // Draw a 3d sample in [a, b]
        glm::vec3 next_3d()
        {
            return { next_1d(), next_1d(), next_1d() };
        }

    public:
        // Implement conformance to std::uniform_random_bit_generator<Sampler>
        constexpr static uint32_t min() { return 0; }
        constexpr static uint32_t max() { return 4294967295; }
        uint32_t g() { return pcg_hash(m_state); }
        uint32_t operator()() { return pcg_hash(m_state); }

        using result_type = uint32_t;
    };

    // Verify concept conformance
    static_assert(std::uniform_random_bit_generator<Sampler>);
} // namespace ref

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

#endif //COMPUTERGRAPHICS_UTIL_H