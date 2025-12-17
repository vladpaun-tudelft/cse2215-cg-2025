#pragma once
// Suppress warnings in third-party code.
#include "glm/gtc/constants.hpp"
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

glm::ivec2 directionVec(int dir) {
    switch (dir) {
        case 0: return {0,1};
        case 1: return {1,0};
        case 2: return {0,-1};
        case 3: return {-1,0};
        default: return {0,0};
    }
}
inline glm::mat3 computeMazeTransformation(std::span<const MovementCommand> moves)
{
    glm::mat3 out = glm::identity<glm::mat3>();
    out = translationMatrix(glm::vec2{1,1}) * out;
    int direction = 0;
    glm::ivec2 gridPos{1,1};

    for (MovementCommand move : moves) {
        switch (move) {
            case MovementCommand::MoveForward: {
                glm::ivec2 v = directionVec(direction);
                gridPos += v;
                out = translationMatrix(glm::vec2(v)) * out;
                break;
            }

            case MovementCommand::RotateLeft: {
                glm::vec2 pivot = glm::vec2(gridPos) + glm::vec2(0.5f, 0.5f);
                out = translationMatrix(pivot)
                        * rotationMatrix(glm::half_pi<float>())
                        * translationMatrix(-pivot)
                        * out;
                direction = (direction + 3) % 4;
                break;
            }
            case MovementCommand::RotateRight: {
                glm::vec2 pivot = glm::vec2(gridPos) + glm::vec2(0.5f, 0.5f);
                out = translationMatrix(pivot)
                    * rotationMatrix(- glm::half_pi<float>())
                    * translationMatrix(-pivot)
                    * out;
                direction = (direction + 5) % 4;
                break;
            }
            default: break;
        }
    }
    return out;
}
