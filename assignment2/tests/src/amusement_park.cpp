#include "../../amusement_park/src/your_code_here_amusement_park.h"
// This comment acts as a barrier for clang-format such that your_code_here.h is always included first
#include "reference.h"

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
DISABLE_WARNINGS_POP()
#include <algorithm>
#include <array>
#include <cmath>
#include <framework/glm_ostream.h>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace Catch;
constexpr float epsilon = 0.001f;

static std::ostream& operator<<(std::ostream& stream, const TopSpinState& topSpinState)
{
    stream << "TopSpinState {" << std::endl;
    stream << "\tarmAngle = " << topSpinState.armAngle << std::endl;
    stream << "\tseatsAngle = " << topSpinState.seatsAngle << std::endl;
    stream << "\tharnessAngle = " << topSpinState.harnessAngle << std::endl;
    stream << "}";
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, const TopSpinMatrices& matrices)
{
    stream << "TopSpinState {" << std::endl;
    stream << "\tsupports= " << matrices.supports << std::endl;
    stream << "\tarms= " << matrices.arms << std::endl;
    stream << "\tseats= " << matrices.seats << std::endl;
    stream << "\tharnesses = [" << std::endl;
    for (const glm::mat4& m : matrices.harnesses) {
        stream << m << ", " << std::endl;
    }
    stream << "\t]" << std::endl;
    stream << "}";
    return stream;
}

static float normalizeAngleDiff(float angleDiff)
{
    float absAngle = std::abs(angleDiff);
    if (absAngle > glm::two_pi<float>())
        absAngle = std::remainder(absAngle, glm::two_pi<float>());
    absAngle = std::min(absAngle, glm::two_pi<float>() - absAngle);
    return absAngle;
}

static float quatDistance(const glm::quat& q1, const glm::quat& q2)
{
    // https://stackoverflow.com/questions/23260939/distance-or-angular-magnitude-between-two-quaternions/23263233#23263233
    const glm::quat qd = glm::inverse(q1) * q2;
    const float tmp = glm::length(glm::vec3(qd.x, qd.y, qd.z));
    float absAngle = std::abs(2.0f * std::atan2(tmp, qd.w));
    return normalizeAngleDiff(absAngle);
}

static TopSpinState generateRandomState(std::mt19937& rng)
{
    std::uniform_real_distribution<float> anglesDistribution { 0.0f, 720.0f };
    std::uniform_real_distribution<float> harnessAnglesDistribution { 0.0f, 90.0f };
    return TopSpinState {
        .armAngle = anglesDistribution(rng),
        .seatsAngle = anglesDistribution(rng),
        .harnessAngle = harnessAnglesDistribution(rng)
    };
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
static float matrixTranslationDistance(const glm::mat4& matrix1, const glm::mat4& matrix2)
{
    const glm::vec3 p1 = matrix1 * glm::vec4(0, 0, 0, 1);
    const glm::vec3 p2 = matrix2 * glm::vec4(0, 0, 0, 1);
    return glm::distance(p1, p2);
}

constexpr size_t numSeats = 44;

TEST_CASE("TopSpin Random Tests")
{
    static constexpr size_t numPositionTests = 176;
    static constexpr size_t numRotationTests = 88;
    static constexpr size_t numCombinedTests = 88;
    // Each individual harness counts as a separate test.
    static_assert(numPositionTests % numSeats == 0);
    static_assert(numRotationTests % numSeats == 0);
    static_assert(numCombinedTests % numSeats == 0);

    SECTION("Support transform matrix")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        const auto input = generateRandomState(rng);
        const auto outputMatrices = computeTopSpinTransformations(input);
        const auto referenceMatrices = referenceComputeTopSpinTransformations(input);

        CAPTURE(input, outputMatrices.supports, referenceMatrices.supports);
        REQUIRE(matrixDistance(outputMatrices.supports, referenceMatrices.supports) == Approx(0.0f).margin(::epsilon));
    }

    SECTION("Arm transform should have the correct translation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numPositionTests / 2; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.arms, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            const glm::vec3 desiredTranslation = glm::vec3(referenceMatrices.arms * glm::vec4(0, 0, 0, 1));
            CAPTURE(input, outputMatrices.arms, outputTranslation, desiredTranslation);
            REQUIRE(glm::distance(outputTranslation, desiredTranslation) == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Arm transform should have the correct rotation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numRotationTests / 2; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.arms, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            const glm::quat desiredRotationQuat = glm::angleAxis(glm::radians(input.armAngle), glm::vec3(1, 0, 0));
            CAPTURE(input, outputMatrices.arms, outputRotationQuat, desiredRotationQuat);
            REQUIRE(quatDistance(outputRotationQuat, desiredRotationQuat) == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Arm transform matrix")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numCombinedTests / 2; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);
            // std::shuffle(std::begin(outputMatrices.harnesses), std::end(outputMatrices.harnesses), rng);

            CAPTURE(input, outputMatrices.arms, referenceMatrices.arms);
            REQUIRE(matrixDistance(outputMatrices.arms, referenceMatrices.arms) == Approx(0.0f).margin(::epsilon));
        }
    }

    SECTION("Seats transform should have the correct rotation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numRotationTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.seats, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            const glm::quat desiredRotationQuat = glm::angleAxis(glm::radians(input.armAngle + input.seatsAngle), glm::vec3(1, 0, 0));
            CAPTURE(input, outputMatrices.seats, outputRotationQuat, desiredRotationQuat);
            REQUIRE(quatDistance(outputRotationQuat, desiredRotationQuat) == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Seats transform should have the correct translation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numPositionTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);
            CAPTURE(input, outputMatrices.seats, referenceMatrices.seats);

            const auto outputInvArmsMatrix = glm::inverse(outputMatrices.arms);
            const auto referenceInvArmsMatrix = glm::inverse(referenceMatrices.arms);

            const auto errorWithRelativeMatrices = glm::distance(
                glm::vec3((outputInvArmsMatrix * outputMatrices.seats) * glm::vec4(0, 0, 0, 1)),
                glm::vec3((referenceInvArmsMatrix * referenceMatrices.seats) * glm::vec4(0, 0, 0, 1)));
            const auto errorWithAbsoluteMatrices = glm::distance(
                glm::vec3(outputMatrices.seats * glm::vec4(0, 0, 0, 1)),
                glm::vec3(referenceMatrices.seats * glm::vec4(0, 0, 0, 1)));

            if (errorWithRelativeMatrices < errorWithAbsoluteMatrices)
                REQUIRE(errorWithRelativeMatrices == Approx(0).margin(::epsilon));
            else
                REQUIRE(errorWithAbsoluteMatrices == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Seats transform matrix")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numCombinedTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);
            // std::shuffle(std::begin(outputMatrices.harnesses), std::end(outputMatrices.harnesses), rng);

            CAPTURE(input, outputMatrices.seats, referenceMatrices.seats);

            // To prevent a mistake in computing "outputMatrices.arms" from impacting this test, we multiply both
            // the output & reference solutions by their respective arms matrices.
            const auto outputInvArmsMatrix = glm::inverse(outputMatrices.arms);
            const auto referenceInvArmsMatrix = glm::inverse(referenceMatrices.arms);
            const auto errorWithRelativeMatrices = matrixDistance(outputInvArmsMatrix * outputMatrices.seats, referenceInvArmsMatrix * referenceMatrices.seats);
            const auto errorWithAbsoluteMatrices = matrixDistance(outputMatrices.seats, referenceMatrices.seats);
            if (errorWithRelativeMatrices <= errorWithAbsoluteMatrices)
                REQUIRE(matrixDistance(outputInvArmsMatrix * outputMatrices.seats, referenceInvArmsMatrix * referenceMatrices.seats) == Approx(0).margin(::epsilon));
            else
                REQUIRE(matrixDistance(outputMatrices.seats, referenceMatrices.seats) == Approx(0).margin(::epsilon));
        }
    }

    SECTION("Harnesses transform should have the correct rotation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numRotationTests / numSeats; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const glm::quat desiredRotationQuat = glm::angleAxis(glm::radians(input.armAngle + input.seatsAngle + input.harnessAngle), glm::vec3(1, 0, 0));

            for (size_t seatId = 0; seatId < numSeats; ++seatId) {
                glm::vec3 outputScale, outputTranslation, outputSkew;
                glm::quat outputRotationQuat;
                glm::vec4 perspective;
                glm::decompose(outputMatrices.harnesses[seatId], outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

                CAPTURE(input, seatId, outputMatrices.harnesses[seatId], outputRotationQuat, desiredRotationQuat);
                REQUIRE(quatDistance(outputRotationQuat, desiredRotationQuat) == Approx(0).margin(::epsilon));
            }
        }
    }
    SECTION("Harnesses transform should have the correct translation component")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numPositionTests / numSeats; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);

            // Compare whether we can find an equivalent "harness" matrix in the reference solution.
            // To prevent a mistake in computing "outputMatrices.seats" from impacting this test, we multiply both
            // the output & reference solutions by their respective seats matrices.
            size_t numCorrectHarnessPositions = 0;
            const auto testHarnesses = [&](const glm::mat4& referenceInvSeatsMatrix, const glm::mat4& outputInvSeatsMatrix) {
                std::array<bool, numSeats> matchingHarnesses;
                std::fill(std::begin(matchingHarnesses), std::end(matchingHarnesses), false);
                for (const auto& outputMatrix : outputMatrices.harnesses) {
                    auto iter = std::find_if(std::begin(referenceMatrices.harnesses), std::end(referenceMatrices.harnesses),
                        [&](const glm::mat4& m) -> bool {
                            return matrixTranslationDistance(referenceInvSeatsMatrix * m, outputInvSeatsMatrix * outputMatrix) < ::epsilon;
                        });
                    if (iter != std::end(referenceMatrices.harnesses)) {
                        matchingHarnesses[(size_t)std::distance(std::begin(referenceMatrices.harnesses), iter)] = true;
                    }
                }
                numCorrectHarnessPositions = std::max((size_t)std::count(std::begin(matchingHarnesses), std::end(matchingHarnesses), true), numCorrectHarnessPositions);
            };
            testHarnesses(glm::identity<glm::mat4>(), glm::identity<glm::mat4>());
            testHarnesses(glm::inverse(referenceMatrices.seats), glm::inverse(outputMatrices.seats));

            CAPTURE(input, outputMatrices.harnesses, referenceMatrices.harnesses, numCorrectHarnessPositions);
            for (size_t minNumCorrectToGetPoints = 1; minNumCorrectToGetPoints <= numSeats; ++minNumCorrectToGetPoints) {
                REQUIRE(numCorrectHarnessPositions >= minNumCorrectToGetPoints);
            }
        }
    }
    SECTION("Harnesses matrix")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numCombinedTests / numSeats; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);
            const auto referenceMatrices = referenceComputeTopSpinTransformations(input);
            // std::shuffle(std::begin(outputMatrices.harnesses), std::end(outputMatrices.harnesses), rng);
            CAPTURE(input, outputMatrices.harnesses, referenceMatrices.harnesses);

            // Compare whether we can find an equivalent "harness" matrix in the reference solution.
            // To prevent a mistake in computing "outputMatrices.seats" from impacting this test, we multiply both
            // the output & reference solutions by their respective seats matrices.
            size_t numCorrectHarnesses = 0;
            const auto testHarnesses = [&](const glm::mat4& referenceInvSeatsMatrix, const glm::mat4& outputInvSeatsMatrix) {
                std::array<bool, numSeats> matchingHarnesses;
                std::fill(std::begin(matchingHarnesses), std::end(matchingHarnesses), false);
                for (const auto& outputMatrix : outputMatrices.harnesses) {
                    auto iter = std::find_if(std::begin(referenceMatrices.harnesses), std::end(referenceMatrices.harnesses),
                        [&](const glm::mat4& m) -> bool { return matrixDistance(referenceInvSeatsMatrix * m, outputInvSeatsMatrix * outputMatrix) < ::epsilon; });
                    if (iter != std::end(referenceMatrices.harnesses)) {
                        matchingHarnesses[(size_t)std::distance(std::begin(referenceMatrices.harnesses), iter)] = true;
                    }
                }
                numCorrectHarnesses = std::max((size_t)std::count(std::begin(matchingHarnesses), std::end(matchingHarnesses), true), numCorrectHarnesses);
            };
            testHarnesses(glm::identity<glm::mat4>(), glm::identity<glm::mat4>());
            testHarnesses(glm::inverse(referenceMatrices.seats), glm::inverse(outputMatrices.seats));

            for (size_t minNumCorrectToGetPoints = 1; minNumCorrectToGetPoints <= numSeats; ++minNumCorrectToGetPoints) {
                REQUIRE(numCorrectHarnesses >= minNumCorrectToGetPoints);
            }
        }
    }
}

TEST_CASE("[DIAGNOSTIC] TopSpin - Arm")
{
    static constexpr size_t numTests = 100;

    SECTION("Arm are at y=11 units above the ground in the rest pose")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.arms, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.arms, outputTranslation);
            REQUIRE(outputTranslation.y == Approx(11.0f).margin(::epsilon));
        }
    }

    SECTION("Scale = 1.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.arms, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.arms, outputScale);
            REQUIRE(outputScale.x == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.y == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.z == Approx(1.0f).margin(::epsilon));
        }
    }

    SECTION("Skew = 0.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.arms, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.arms, outputSkew);
            REQUIRE(outputSkew.x == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.y == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.z == Approx(0.0f).margin(::epsilon));
        }
    }
}

TEST_CASE("[DIAGNOSTIC] TopSpin - Seats")
{
    static constexpr size_t numTests = 100;

    SECTION("Seats are at y=0.2 units (11 - 10.8) above the ground in the rest pose")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        const TopSpinState input { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f };
        const auto outputMatrices = computeTopSpinTransformations(input);

        glm::vec3 outputScale, outputTranslation, outputSkew;
        glm::quat outputRotationQuat;
        glm::vec4 perspective;
        glm::decompose(outputMatrices.seats, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

        CAPTURE(input, outputMatrices.seats, outputTranslation);
        REQUIRE(outputTranslation.y == Approx(0.2f).margin(::epsilon));
    }

    SECTION("Scale = 1.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.seats, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.seats, outputScale);
            REQUIRE(outputScale.x == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.y == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.z == Approx(1.0f).margin(::epsilon));
        }
    }

    SECTION("Skew = 0.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.seats, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.seats, outputSkew);
            REQUIRE(outputSkew.x == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.y == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.z == Approx(0.0f).margin(::epsilon));
        }
    }
}

TEST_CASE("[DIAGNOSTIC] TopSpin - Harnesses")
{
    static constexpr size_t numTests = 100;

    SECTION("Harness[0] at y=1.7 units (11 - 10.8 + 1.5) above the ground in the rest pose")
    {
        const TopSpinState input { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f };
        const auto outputMatrices = computeTopSpinTransformations(input);

        glm::vec3 outputScale, outputTranslation, outputSkew;
        glm::quat outputRotationQuat;
        glm::vec4 perspective;
        glm::decompose(outputMatrices.harnesses[0], outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

        CAPTURE(input, outputMatrices.harnesses[0], outputTranslation);
        REQUIRE(outputTranslation.y == Approx(1.7f).margin(::epsilon));
    }

    SECTION("Harness centers in range of x=[-7.65, +7.65] in the rest pose ([-8.0, +8.0] accounting for the 0.7 seat width)")
    {
        const TopSpinState input { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f };
        const auto outputMatrices = computeTopSpinTransformations(input);

        glm::vec3 boundsMin { std::numeric_limits<float>::max() };
        glm::vec3 boundsMax { std::numeric_limits<float>::lowest() };
        for (const auto& harness : outputMatrices.harnesses) {
            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(harness, outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);
            boundsMin = glm::min(boundsMin, outputTranslation);
            boundsMax = glm::max(boundsMax, outputTranslation);
        }

        CAPTURE(input, outputMatrices.harnesses, boundsMin, boundsMax);
        REQUIRE(boundsMin.x == Approx(-7.65f).margin(::epsilon));
        REQUIRE(boundsMax.x == Approx(+7.65f).margin(::epsilon));
    }

    SECTION("Harnesses at either z=-0.35 or z=0.95 in the rest pose")
    {
        const TopSpinState input { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f };
        const auto outputMatrices = computeTopSpinTransformations(input);

        for (size_t seatId = 0; seatId < numSeats; ++seatId) {
            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.harnesses[seatId], outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, seatId, outputMatrices.harnesses[seatId], outputTranslation);
            const float distance035 = glm::abs(outputTranslation.z - (-0.35f));
            const float distance095 = glm::abs(outputTranslation.z - (+0.95f));
            const float distanceFrom035or095 = std::min(distance035, distance095);
            REQUIRE(distanceFrom035or095 == Approx(0.0f).margin(::epsilon));
        }
    }

    SECTION("Scale = 1.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.harnesses[0], outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.harnesses[0], outputScale);
            REQUIRE(outputScale.x == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.y == Approx(1.0f).margin(::epsilon));
            REQUIRE(outputScale.z == Approx(1.0f).margin(::epsilon));
        }
    }

    SECTION("Skew = 0.0")
    {
        // Generate a random TopSpinState.
        std::mt19937 rng { 91492584 };

        for (size_t testId = 0; testId < numTests; testId++) {
            const auto input = generateRandomState(rng);
            const auto outputMatrices = computeTopSpinTransformations(input);

            glm::vec3 outputScale, outputTranslation, outputSkew;
            glm::quat outputRotationQuat;
            glm::vec4 perspective;
            glm::decompose(outputMatrices.harnesses[0], outputScale, outputRotationQuat, outputTranslation, outputSkew, perspective);

            CAPTURE(input, outputMatrices.harnesses[0], outputSkew);
            REQUIRE(outputSkew.x == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.y == Approx(0.0f).margin(::epsilon));
            REQUIRE(outputSkew.z == Approx(0.0f).margin(::epsilon));
        }
    }
}
