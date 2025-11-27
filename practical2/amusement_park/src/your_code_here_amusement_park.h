#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
DISABLE_WARNINGS_POP()
#include <array>
#include <span>
#include <vector>

static glm::mat4 rotationMatrix(float angle, const glm::vec3& axis)
{
    return glm::rotate(glm::identity<glm::mat4>(), angle, axis);
}
static glm::mat4 translationMatrix(const glm::vec3& translation)
{
    return glm::translate(glm::identity<glm::mat4>(), translation);
}

// !!! DO NOT MODIFY !!!
struct TopSpinState {
    float armAngle { 0 };
    float seatsAngle { 0 };
    float harnessAngle { 0 };
};
// !!! DO NOT MODIFY !!!
struct TopSpinMatrices {
    glm::mat4 supports;
    glm::mat4 arms;
    glm::mat4 seats;
    std::array<glm::mat4, 44> harnesses;
};

// ==========================
// =====    EXERCISE    =====
// ==========================
inline TopSpinMatrices computeTopSpinTransformations(const TopSpinState& topSpinState)
{
    static constexpr int seatsPerRow = 22;
    const float armAngleInRadians = glm::radians(topSpinState.armAngle);
    const float seatsAngleInRadians = glm::radians(topSpinState.seatsAngle);
    const float harnessAngleInRadians = glm::radians(topSpinState.harnessAngle);

    TopSpinMatrices matrices;
    // Place the supports at the origin.
    matrices.supports = glm::identity<glm::mat4>(); // DO NOT CHANGE THIS LINE
    matrices.arms = glm::identity<glm::mat4>();
    matrices.seats = glm::identity<glm::mat4>();
    for (int i = 0; i < 2 * seatsPerRow; ++i) {
        matrices.harnesses[i] = glm::identity<glm::mat4>();
    }
    return matrices;
}