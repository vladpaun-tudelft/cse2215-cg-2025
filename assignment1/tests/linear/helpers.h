#pragma once
#include "linear.h"
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

class RandomVectorGenerator {
public:
    inline RandomVectorGenerator(float lowest = 0.0f, float highest = 1.0f)
        : m_floatDist(lowest, highest) {};

    inline glm::vec3 generate()
    {
        const float x = m_floatDist(m_randomEngine);
        const float y = m_floatDist(m_randomEngine);
        const float z = m_floatDist(m_randomEngine);

        return glm::vec3(x, y, z) ;
    }

private:
    // Known engine + fixed seed = reproducible tests.
    std::mt19937 m_randomEngine { randomSeed };
    std::uniform_real_distribution<float> m_floatDist;
};

class RandomMatrixGenerator {
public:
    inline RandomMatrixGenerator(float lowest = 0.0f, float highest = 1.0f)
        : m_floatDist(lowest, highest) {};

    inline std::pair<Matrix3, glm::mat3x3> generate()
    {
        // Col 1
        const float m00 = m_floatDist(m_randomEngine);
        const float m10 = m_floatDist(m_randomEngine);
        const float m20 = m_floatDist(m_randomEngine);

        // Col 2
        const float m01 = m_floatDist(m_randomEngine);
        const float m11 = m_floatDist(m_randomEngine);
        const float m21 = m_floatDist(m_randomEngine);

        // Col 3
        const float m02 = m_floatDist(m_randomEngine);
        const float m12 = m_floatDist(m_randomEngine);
        const float m22 = m_floatDist(m_randomEngine);

        return {
            Matrix3 { glm::vec3 { m00, m10, m20 }, glm::vec3 { m01, m11, m21 }, glm::vec3 { m02, m12, m22 } },
            glm::mat3x3(m00, m10, m20, m01, m11, m21, m02, m12, m22)
        };
    }

private:
    // Known engine + fixed seed = reproducible tests.
    std::mt19937 m_randomEngine { randomSeed };
    std::uniform_real_distribution<float> m_floatDist;
};

#define APPROX_FLOAT(x) Catch::Approx(x).margin(1e-5f)

// ApproxZero by default uses an epsilon of 0.0f and for some reason it makes one of the tests fail ( 0.0f == Approx(0.0) fails... ).
// https://github.com/catchorg/Catch2/issues/1444
//
// https://stackoverflow.com/questions/56466022/what-is-the-canonical-way-to-check-for-approximate-zeros-in-catch2
#define ApproxZero Catch::Approx(0.0f).margin(1e-5f)

static constexpr size_t testsPerSection = 60;

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec)
{
    stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Matrix3& matrix)
{
    stream << "[ " << matrix.col1 << " , " << matrix.col2 << " , " << matrix.col3 << " ]";
    return stream;
}

namespace glm {
std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec)
{
    stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat3& matrix)
{
    stream << "[ " << matrix[0] << " , " << matrix[1] << " , " << matrix[2] << " ]";
    return stream;
}
}

bool operator==(const glm::vec3& lhs, const glm::vec3& rhs)
{
    return lhs.x == APPROX_FLOAT(rhs.x) && lhs.y == APPROX_FLOAT(rhs.y) && lhs.z == APPROX_FLOAT(rhs.z);
}

bool operator==(const Matrix3& lhs, const glm::mat3& rhs)
{
    // clang-format off
    return
        lhs.col1.x == APPROX_FLOAT(rhs[0][0]) &&
        lhs.col1.y == APPROX_FLOAT(rhs[0][1]) &&
        lhs.col1.z == APPROX_FLOAT(rhs[0][2]) &&
        lhs.col2.x == APPROX_FLOAT(rhs[1][0]) &&
        lhs.col2.y == APPROX_FLOAT(rhs[1][1]) &&
        lhs.col2.z == APPROX_FLOAT(rhs[1][2]) &&
        lhs.col3.x == APPROX_FLOAT(rhs[2][0]) &&
        lhs.col3.y == APPROX_FLOAT(rhs[2][1]) &&
        lhs.col3.z == APPROX_FLOAT(rhs[2][2]);
    // clang-format on
}
