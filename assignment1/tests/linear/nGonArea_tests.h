#include "helpers.h"
#include "linear.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <array>
#include <cmath>
#include <ostream>
#include <random>
#include <tuple>
#include <vector>

TEST_CASE("nGonAreaTest")
{
    SECTION("Triangle Area")
    {
        // Generate some random triangles, check whether student computes their area correctly.
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto v0 = randomVectorGenerator.generate();
            auto v1 = randomVectorGenerator.generate();
            auto v2 = randomVectorGenerator.generate();
            while (v1 == v0) {
                v1 = randomVectorGenerator.generate();
            }
            while ((v2 == v0) || (v2 == v1)) {
                v2 = randomVectorGenerator.generate();
            }

            const std::array<glm::vec3, 3> triangle = { v0, v1, v2 };

            // Use the cross product to compute the area of the triangle
            glm::vec3 edge1 = triangle[1] - triangle[0];
            glm::vec3 edge2 = triangle[2] - triangle[0];
            auto expected = 0.5f * glm::length(glm::cross(edge1, edge2));

            // Get the student answer
            auto output = areaOfTriangle(triangle);

            CAPTURE(
                triangle,
                expected,
                output);
            REQUIRE(
                expected == APPROX_FLOAT(output));
        }
    }

    SECTION("nGon Area Varying Angles")
    {
        // Create some n-Gon with a random number (>2) vertices that that lie at a random distance from the origin and have random angles between them.
        RandomGenerator randomNumberGenerator { +3.f, +10.0f };
        RandomGenerator randomAngleGenerator { +1.f, +10.f };
        RandomGenerator randomRotationGenerator { 0.f, glm::two_pi<float>() };
        for (size_t i = 0; i < testsPerSection; i++) {
            // Pick a random number of Vertices for the n-Gon
            const int numVertices = (int)std::ceil(randomNumberGenerator.generate());
            // Place them at a random distance in the x/y plane
            float distance = randomAngleGenerator.generate();
            // Compute angles for each of them (summing up to 2 PI)
            std::vector<float> vertexAngles;
            float angleSum = 0.f;
            // Each vertex lies at a random Angle
            for (int j = 0; j < numVertices; j++) {
                float randomAngle = randomAngleGenerator.generate();
                vertexAngles.push_back(randomAngle);
                angleSum += randomAngle;
            }
            // The angles have to be normalized to sum up to 2PI.
            for (int j = 0; j < numVertices; j++) {
                vertexAngles[j] = ((vertexAngles[j] / angleSum) * glm::two_pi<float>());
            }

            // Here, we want to provide an ordered nGon, thus sort the angles
            std::sort(std::begin(vertexAngles), std::end(vertexAngles));

            // Create two random rotations around the x and y axis
            const float xRotationAngle = randomRotationGenerator.generate();
            const glm::mat3 xRotation = {
                1,
                0,
                0,
                0,
                std::cos(xRotationAngle),
                -1.f * std::sin(xRotationAngle),
                0,
                std::sin(xRotationAngle),
                std::cos(xRotationAngle)
            };
            const float yRotationAngle = randomRotationGenerator.generate();
            const glm::mat3 yRotation = {
                std::cos(yRotationAngle),
                0,
                std::sin(yRotationAngle),
                0,
                1,
                0,
                -1.f * std::sin(yRotationAngle),
                0,
                std::cos(yRotationAngle)
            };

            // Translate the angles into x/y coordinates and rotate them in 3-space around the x and y axis by some random angle
            std::vector<glm::vec3> nGon;
            for (int j = 0; j < numVertices; j++) {
                glm::vec3 vertex = { std::cos(vertexAngles[j]), std::sin(vertexAngles[j]), 0 };
                vertex = xRotation * yRotation * vertex;
                nGon.push_back(vertex);
            }

            // Compute the correct solution
            std::vector<std::array<glm::vec3, 3>> triangles;
            glm::vec3 base = nGon[0];
            glm::vec3 first = nGon[1];
            glm::vec3 second;
            for (size_t j = 2; j != nGon.size(); j++) {
                second = nGon[j];
                std::array<glm::vec3, 3> triangle { base, first, second };
                triangles.push_back(triangle);
                first = second;
            }
            float expected = 0.;
            for (const auto& triangle : triangles) {
                glm::vec3 edge1 = triangle[1] - triangle[0];
                glm::vec3 edge2 = triangle[2] - triangle[0];
                expected += 0.5f * glm::length(glm::cross(edge1, edge2));
            }

            // Get the student answer
            auto output = areaOfIrregularNGon(nGon);

            CAPTURE(
                nGon,
                expected,
                output);
            REQUIRE(
                expected == APPROX_FLOAT(output));
        }
    }
}
