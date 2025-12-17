#ifndef SHADING_ASSIGNMENT_HELPERS
#define SHADING_ASSIGNMENT_HELPERS

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <cassert>
#include <cmath>
#include <random>

class RandomHemisphereGenerator {
public:
    inline glm::vec3 generate()
    {
        const float r = 1.0f; // Radius.
        const float phi = m_phiDist(m_randomEngine);
        const float theta = 0.01f + 0.98f * m_thetaDist(m_randomEngine);

        const float x = r * std::sin(theta) * std::cos(phi);
        const float y = r * std::sin(theta) * std::sin(phi);
        const float z = r * std::cos(theta);
        assert(std::abs(glm::length(glm::vec3(x, y, z)) - 1.0f) < 1e-5f);
        return glm::vec3(x, z, y);
    }

private:
    // Known engine + fixed seed = reproducable across machines.
    std::mt19937 m_randomEngine { 12345 };
    std::uniform_real_distribution<float> m_phiDist { 0.0f, glm::two_pi<float>() };
    std::uniform_real_distribution<float> m_thetaDist { 0.0f, glm::half_pi<float>() };
};

class RandomGenerator {
public:
    inline float generate()
    {
        return m_dist(m_randomEngine);
    }

private:
    // Known engine + fixed seed = reproducable across machines.
    std::mt19937 m_randomEngine { 54321 };
    std::uniform_real_distribution<float> m_dist { 0.0f, 1.0f };
};

class RandomVec3Generator {
public:
    inline glm::vec3 generate()
    {
        return glm::vec3(m_dist(m_randomEngine), m_dist(m_randomEngine), m_dist(m_randomEngine));
    }

    inline glm::vec3 generate(const glm::vec3& scale)
    {
        return scale * generate();
    }

    inline glm::vec3 generate(const glm::vec3& lower, const glm::vec3 upper)
    {
        return lower + (upper - lower) * generate();
    }

private:
    // Known engine + fixed seed = reproducable across machines.
    std::mt19937 m_randomEngine { 54321 };
    std::uniform_real_distribution<float> m_dist { 0.0f, 1.0f };
};

// Deterministic way of converting a hemisphere vector with the y-axis as up to a vector with
//  the given normal as up vector.
inline glm::vec3 normalCoordinateSystem(const glm::vec3& normal, const glm::vec3& vector)
{
    const glm::vec3 tangent = glm::normalize(glm::dot(normal, glm::vec3(0, 1, 0)) < 0.5f ? glm::cross(normal, glm::vec3(0, 1, 0)) : glm::cross(normal, glm::vec3(1, 0, 0)));
    const glm::vec3 bitangent = glm::normalize(glm::cross(tangent, normal));
    return vector.x * tangent + vector.y * normal + vector.z * bitangent;
}

constexpr float errorMargin = 1e-3f;
#define ApproxZero Catch::Approx(0.0f).margin(errorMargin)

static constexpr size_t testsPerSection = 100;


/*
 *
 * Googch helpers
 */

// Prevents the case of k_cool > 1 or k_warm > 1 from ever arising.
static void ensureSumLowerThan1(float& a, float& b)
{
    if (a + b > 1.0f) {
        a = 1.0f - b;
    }
}

Color runStudentGooch(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& lightPos, const Color& lightColor, const int n)
{
    try {
        return gooch(materialInformation, vertexPos, normal, lightPos, lightColor, n);
    } catch (const std::exception&) {
        return glm::vec3(0, 0, 1);
    }
}

#endif SHADING_ASSIGNMENT_HELPERS
