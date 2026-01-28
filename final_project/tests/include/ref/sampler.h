#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
DISABLE_WARNINGS_POP()
#include <random>
#include <utility>

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
    using result_type = uint32_t;
    
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
};

// Verify concept conformance
static_assert(std::uniform_random_bit_generator<Sampler>);
} // namespace ref