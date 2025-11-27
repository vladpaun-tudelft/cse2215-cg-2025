#pragma once
// Suppress warnings in third-party code.
#include "glm/matrix.hpp"
#include <framework/disable_all_warnings.h>
#include <vector>
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
    int currDirection = 0;
    out = translationMatrix(glm::vec2{1,1}) * out;
    std::vector<int> currPos = {1,1};

    for (MovementCommand move : moves) {
        if (move == MovementCommand::MoveForward) {
            glm::vec2 direction;
            if (currDirection == 0) {
                direction = {0,1};
            } else if (currDirection == 1) {
                direction = {1,0};
            } else if (currDirection == 2) {
                direction = {0,-1};
            } else {
                direction = {-1, 0};
            }
            currPos = { currPos[0] + (int)direction.x, currPos[1] + (int)direction.y };
            out = translationMatrix(direction) * out;
        } else if (move == MovementCommand::RotateLeft) {
            out = translationMatrix({ currPos[0], currPos[1] }) * rotationMatrix(glm::half_pi<float>()) * translationMatrix({ -currPos[0], -currPos[1] }) * out;
            currDirection--;
            if (currDirection < 0) currDirection = 3;
        }else if (move == MovementCommand::RotateRight) {
            out = translationMatrix({ currPos[0], currPos[1] }) * rotationMatrix(-glm::half_pi<float>()) * translationMatrix({ -currPos[0], -currPos[1] }) * out;
            currDirection++;
            if (currDirection > 3) currDirection = 0;

        } else return out;
    }
    return out;
}
