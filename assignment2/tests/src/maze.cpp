#include "../../maze/src/your_code_here_maze.h"
// This comment acts as a barrier for clang-format such that your_code_here.h is always included first
#include "reference.h"

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/matrix_decompose.hpp>
// #include <glm/gtx/string_cast.hpp>
DISABLE_WARNINGS_POP()
#include <framework/glm_ostream.h>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <vector>

using namespace Catch;

constexpr float epsilon = 0.0001f;

static std::ostream& operator<<(std::ostream& stream, MovementCommand command)
{
    if (command == MovementCommand::MoveForward) {
        stream << "MoveForward";
    } else if (command == MovementCommand::RotateLeft) {
        stream << "RotateLeft";
    } else if (command == MovementCommand::RotateRight) {
        stream << "RotateRight";
    } else {
        stream << "INVALID";
    }
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, const std::span<const MovementCommand> &commands)
{
    stream << "[";
    for (const auto command : commands) {
        stream << command << ", ";
    }
    stream << "]";
    return stream;
}

static std::vector<MovementCommand> generateRandomMovementCommands(std::mt19937& rng, size_t numCommands)
{
    std::vector<MovementCommand> commands;
    while (commands.size() < numCommands) {
        const MovementCommand command = (MovementCommand)(rng() % 3);
        commands.push_back(command);

        // Remove command if it results in the crab going into x<1 or y<1 like in the real maze (there will always be 1 student that does something weird).
        const auto crabPos = glm::ivec2(referenceComputeMazeTransformation(commands) * glm::vec3(0.5f, 0.5f, 1));
        if (glm::any(glm::lessThan(crabPos, glm::ivec2(1))))
            commands.pop_back();
    }
    return commands;
}

// Use a macro of a lambda so the error messages match up with the line where the check failed. If we would define this as a regular
// function than the REQUIRE that failed will always report the same line (instead of the line of the caller).
static float matrixDistance(const glm::mat4& matrix1, const glm::mat4& matrix2)
{
    auto diff = matrix1 - matrix2;
    glm::vec4 diffSum { 0.0f };
    diffSum += glm::abs(diff[0]); // glm::abs() not defined for matrices :(
    diffSum += glm::abs(diff[1]); // glm::abs() not defined for matrices :(
    diffSum += glm::abs(diff[2]); // glm::abs() not defined for matrices :(
    diffSum += glm::abs(diff[3]); // glm::abs() not defined for matrices :(
    return glm::compAdd(diffSum);
}

static float normalizeAngleDiff(float angleDiff)
{
    float absAngle = std::abs(angleDiff);
    if (absAngle > glm::two_pi<float>())
        absAngle = std::remainder(absAngle, glm::two_pi<float>());
    absAngle = std::min(absAngle, glm::two_pi<float>() - absAngle);
    return absAngle;
}

TEST_CASE("[DIAGNOSTIC] Maze")
{
    static constexpr size_t numTests = 100;

    SECTION("The initial matrix when given no commands")
    {
        // Generate a random movements through maze.
        std::mt19937 rng { 91492584 };

        std::vector<MovementCommand> input;
        const auto outputMatrix = computeMazeTransformation(input);
        const auto expectedMatrix = glm::translate(glm::identity<glm::mat3>(), glm::vec2(1, 1));

        CAPTURE(input, outputMatrix, expectedMatrix);
        REQUIRE(matrixDistance(outputMatrix, expectedMatrix) == Approx(0.0f).margin(::epsilon));
    }

    SECTION("The transform matrix should preserve distance")
    {
        // Generate a random movements through maze.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomMovementCommands(rng, 5);
            const auto outputMatrix = computeMazeTransformation(input);

            const glm::vec2 point0 = { 0, 0 };
            const glm::vec2 point1 = { 1, 0 };
            const float originalDistance = glm::length(point0 - point1);
            const float transformedDistance = glm::length(outputMatrix * glm::vec3(point0, 1) - outputMatrix * glm::vec3(point1, 1));

            CAPTURE(input, outputMatrix, point0, point1, originalDistance, transformedDistance);
            REQUIRE(transformedDistance / originalDistance == Approx(1.0f).margin(::epsilon));
        }
    }

    SECTION("The transform matrix should preserve angle")
    {
        // Generate a random movements through maze.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomMovementCommands(rng, 5);
            const auto outputMatrix = computeMazeTransformation(input);

            const glm::vec2 point0 = { 0, 0 };
            const glm::vec2 point1 = { 1, 0 };
            const float orignalAngle = float(rng() % 180);
            const glm::vec2 point2 = { std::cos(glm::radians(orignalAngle)), std::sin(glm::radians(orignalAngle)) };
            const glm::vec3 tpoint0 = outputMatrix * glm::vec3(point0, 1);
            const glm::vec3 tpoint1 = outputMatrix * glm::vec3(point1, 1);
            const glm::vec3 tpoint2 = outputMatrix * glm::vec3(point2, 1);
            const float transformedCosine = glm::dot(tpoint2 - tpoint0, tpoint1 - tpoint0);
            const float transformedAngle = glm::degrees(std::acos(transformedCosine));

            CAPTURE(input, outputMatrix, point0, point1, orignalAngle, transformedAngle);
            REQUIRE(transformedAngle - orignalAngle == Approx(0.0f).margin(1e-3));
        }
    }

    SECTION("Forward command")
    {
        std::vector<MovementCommand> input;
        for (size_t testId = 0; testId < numTests; testId++) {
            input.push_back(MovementCommand::MoveForward);
            const auto outputMatrix = computeMazeTransformation(input);
            const auto expectedMatrix = referenceComputeMazeTransformation(input);

            CAPTURE(input, outputMatrix, expectedMatrix);
            REQUIRE(matrixDistance(outputMatrix, expectedMatrix) == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Rotation command")
    {
        // Generate a random movements through maze.
        std::mt19937 rng { 91492584 };

        std::vector<MovementCommand> input;
        for (size_t testId = 0; testId < numTests; testId++) {
            input.push_back(rng() % 2 == 0 ? MovementCommand::RotateLeft : MovementCommand::RotateRight);
            const auto outputMatrix = computeMazeTransformation(input);
            const auto expectedMatrix = referenceComputeMazeTransformation(input);

            CAPTURE(input, outputMatrix, expectedMatrix);
            REQUIRE(matrixDistance(outputMatrix, expectedMatrix) == Approx(0).margin(::epsilon));
        }
    }
}

TEST_CASE("Maze Random Tests")
{
    static constexpr size_t numTests = 100;
    // Generate a random movements through maze.
    std::mt19937 rng { 91492584 };

    for (size_t testId = 0; testId < numTests; testId++) {
        const auto input = generateRandomMovementCommands(rng, 10);
        const auto outputMatrix = computeMazeTransformation(input);
        const auto expectedMatrix = referenceComputeMazeTransformation(input);

        CAPTURE(input, outputMatrix, expectedMatrix);
        REQUIRE(matrixDistance(outputMatrix, expectedMatrix) == Approx(0).margin(::epsilon));
    }
}
