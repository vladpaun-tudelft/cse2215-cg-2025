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
#include <cmath>
#include <ostream>
#include <random>
#include <tuple>

TEST_CASE("nGonOrderTest")
{
    SECTION("nGon Order Varying Angles")
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
            float* vertexAngles = new float[numVertices];
            float angleSum = 0.f;
            // Each vertex lies at a random Angle
            for (int j = 0; j < numVertices; j++) {
                vertexAngles[j] = randomAngleGenerator.generate();
                angleSum += vertexAngles[j];
            }
            // The angles have to be normalized to sum up to 2PI.
            for (int j = 0; j < numVertices; j++) {
                vertexAngles[j] = ((vertexAngles[j] / angleSum) * glm::two_pi<float>());
            }
            
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
            std::vector<int> correctResult;
            // Get the barycenter of the n-Gon
            glm::vec3 barycenter = { 0.f, 0.f, 0.f };
            for (glm::vec3 vertex : nGon) {
                barycenter += vertex;
            }
            // Get the (normalized) normal of the plane the n-Gon resides in
            glm::vec3 normal = glm::normalize(glm::cross(nGon[0] - barycenter, nGon[1] - barycenter));
            // Iterate through all vertices and compute the angle to the first one, save them as tuples
            std::vector<std::tuple<float, int>> anglesAndIndices;
            glm::vec3 baseLine = nGon[0] - barycenter;
            for (int j = 0; j < nGon.size(); j++) {
                glm::vec3 line = nGon[j] - barycenter;
                float angle = std::atan2(glm::dot(glm::cross(baseLine, line), normal), glm::dot(baseLine, line));
                anglesAndIndices.push_back({ angle, j });
            }
            // Order the tuples, return the order
            std::sort(anglesAndIndices.begin(), anglesAndIndices.end());
            // Fill the result vector
            for (std::tuple<float, int> tuple : anglesAndIndices) {
                correctResult.push_back(std::get<1>(tuple));
            }

            // Get the student answer
            auto output = orderOfnGonVertices(nGon);

            CAPTURE(nGon, correctResult, output);
            REQUIRE(correctResult.size() == output.size());

            // Check whether the two permutations agree
            correctResult.insert(correctResult.end(), correctResult.begin(), correctResult.end());
            auto correctResultReversed = correctResult;
            std::ranges::reverse(correctResultReversed);
            REQUIRE_FALSE((
                std::ranges::search(correctResult, output).empty() &&
                std::ranges::search(correctResultReversed, output).empty()
            ));

        }
    }
}
