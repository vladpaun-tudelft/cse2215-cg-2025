#pragma once
#include "your_code_here.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_approx.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <random>
#include <tuple>

static constexpr size_t randomSeed = 12345;

class RandomGenerator {
public:
    inline RandomGenerator(float lowest = 0.0f, float highest = 1.0f)
        : m_dist(lowest, highest) {};

    inline float generate()
    {
        return m_dist(m_randomEngine);
    }

private:
    // Known engine + fixed seed = reproducible across machines.
    std::mt19937 m_randomEngine { randomSeed };
    std::uniform_real_distribution<float> m_dist;
};

#define APPROX_FLOAT(x) Catch::Approx(x).margin(1e-5f)

// ApproxZero by default uses an epsilon of 0.0f and for some reason it makes one of the tests fail ( 0.0f == Approx(0.0) fails... ).
// https://github.com/catchorg/Catch2/issues/1444
//
// https://stackoverflow.com/questions/56466022/what-is-the-canonical-way-to-check-for-approximate-zeros-in-catch2
#define ApproxZero Catch::Approx(0.0f).margin(1e-5f)

static constexpr size_t testsPerSection = 60;
