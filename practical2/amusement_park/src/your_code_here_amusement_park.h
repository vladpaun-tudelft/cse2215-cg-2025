#pragma once
// Suppress warnings in third-party code.
#include "glm/ext/matrix_transform.hpp"
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
    for (int i = 0; i < 2 * seatsPerRow; ++i) {
        matrices.harnesses[i] = glm::identity<glm::mat4>();
    }

    glm::mat4 armRotation = rotationMatrix(armAngleInRadians, glm::vec3(1,0,0));
    glm::mat4 armTranslation = translationMatrix(glm::vec3{0,11.0f,0});
    matrices.arms = armTranslation * armRotation;

    glm::mat4 seatsRotation = rotationMatrix(seatsAngleInRadians, glm::vec3{1,0,0});
    glm::mat4 seatsTranslation = translationMatrix(glm::vec3{0,-(7.8f + 3.0f),0});
    glm::mat4 seatsSmallTranslation = translationMatrix(glm::vec3{0,3,0});
    glm::mat4 seatsSmallTranslationInverse = translationMatrix(glm::vec3{0,-3,0});

    matrices.seats = matrices.arms * seatsTranslation * translationMatrix(glm::vec3 { 0, 3.0f, 0 }) * seatsRotation * translationMatrix(glm::vec3 { 0, -3.0f, 0 });

    for (size_t i = 0; i <= seatsPerRow; i++) {
        float bullshit = 16.0f / seatsPerRow;
        float xPos = (float)(i+1) * bullshit - 8.0f - bullshit / 2;
        glm::mat4 harnessRotation = rotationMatrix(harnessAngleInRadians, glm::vec3{1,0,0});
        glm::mat4 putPivotBack = translationMatrix(glm::vec3{xPos, 1.52f, 0.95f});

        matrices.harnesses[i] = matrices.seats * putPivotBack * harnessRotation;
    }

    for (size_t i = seatsPerRow; i <2 * seatsPerRow; i++) {
        float bullshit = 16.0f / seatsPerRow;
        float xPos = (float)(i+1 - seatsPerRow) * bullshit - 8.0f - bullshit / 2z;
        glm::mat4 harnessRotation = rotationMatrix(harnessAngleInRadians, glm::vec3 { 1, 0, 0 });
        glm::mat4 putPivotBack = translationMatrix(glm::vec3 { xPos, 1.52f, -0.35f });

        matrices.harnesses[i] = matrices.seats * putPivotBack * harnessRotation;
    }
    return matrices;
}