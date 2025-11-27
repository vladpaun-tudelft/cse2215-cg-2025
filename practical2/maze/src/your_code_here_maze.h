#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
DISABLE_WARNINGS_POP()
#include <span>

static glm::mat3 rotationMatrix(float angle)
{
    return glm::rotate(glm::identity<glm::mat3>(), angle);
}
static glm::mat3 translationMatrix(const glm::vec2& translation)
{
    return glm::translate(glm::identity<glm::mat3>(), translation);
}

enum class MovementCommand : int {
    MoveForward = 0,
    RotateLeft = 1,
    RotateRight = 2
};

// ==========================
// =====    EXERCISE    =====
// ==========================
inline glm::mat3 computeMazeTransformation(std::span<const MovementCommand> moves)
{
    glm::mat3 out = glm::identity<glm::mat3>();
    return out;
}
