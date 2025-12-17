#ifndef COMPUTERGRAPHICS_GAME_H
#define COMPUTERGRAPHICS_GAME_H

#include "glm/glm.hpp"
#include <array>

/*
 * Accessibility:
 * In case there are colors you can not distinguish, feel free to alter the color values here.
 * Please do not alter any other color values without contacting us first, since we might unit test them.
 * There shouldn't be any color values hindering you from completing the assignment.
 * DO NOT ALTER THE NAMES OR THE STRUCTURE OR ANYTHING OTHER THAN THE FLOATING POINT LITERALS
 */
struct ColorPalette {
    // The default is the allegedly accessible IBM Palette
    // Ghost 1 Color
    static constexpr glm::vec3 COLOR1 = {100.0f / 255.0f, 143.0f / 255.0f, 255.0f / 255.0f};

    // Ghost 2 Color
    static constexpr glm::vec3 COLOR2 = {120.0f / 255.0f, 94.0f / 255.0f, 240.0f / 255.0f};

    // Pacman Color
    static constexpr glm::vec3 COLOR4 = {254.0f / 255.0f, 97.0f/255.0f, 0.0f/255.0f};

    // Color of the LookAt Sphere
    static constexpr glm::vec3 COLOR5 = {0.2f, 0.2f, 1.0f};

    // Color of coordinate system axes x, y, z
    static constexpr glm::vec3 COLOR6 = {1.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 COLOR7 = {0.0f, 1.0f, 0.0f};
    static constexpr glm::vec3 COLOR8 = {0.0f, 0.0f, 1.0f};

    // Ray colors for close hit, normal hit, and miss
    static constexpr glm::vec3 COLOR9 = {0.8f, 0.2f, 0.2f};
    static constexpr glm::vec3 COLOR10 = {0.8f, 0.8f, 0.2f};
    static constexpr glm::vec3 COLOR11 = {0.2f, 0.8f, 0.2f};
};

// Don't change these values or you might brake the unit tests
struct Game {
    // shift_direction can be one of the following normalized vectors:
    // additionally has to be scaled by velocity!
    static constexpr glm::vec2 LEFT{-1.0f, 0.0f};
    static constexpr glm::vec2 RIGHT{1.0f, 0.0f};
    static constexpr glm::vec2 UP{0.0f, 1.0f};
    static constexpr glm::vec2 DOWN{0.0f, -1.0f};

    static constexpr uint32_t sequenceLength = 8;
    static constexpr size_t nGhosts = 2;

    static constexpr glm::ivec2 mazeCenter {6, 3};
    static constexpr glm::ivec2 mazeSize {11, 10};
    using Maze = std::array<std::array<bool, mazeSize.x>, mazeSize.y>;
    static constexpr Maze maze {{
        {false, false, false, false, false, false, true , false, false, false, false},
        {false, false, false, false, false, false, true , false, false, false, false},
        {false, false, false, false, false, false, true , false, false, false, false},
        {true , true , true , true , true , true , true , true , true , true , true },
        {false, false, true , false, false, false, true , false, false, true , false},
        {true , true , true , false, false, false, true , false, false, true , false},
        {false, false, true , false, false, false, true , false, false, true , false},
        {false, false, true , false, false, false, true , true , true , true , false},
        {false, false, true , true , true , true , true , false, false, false, false},
        {false, false, false, false, false, false, true , false, false, false, false},
    }};

    static constexpr glm::vec3 pacmanColor = ColorPalette::COLOR4;
    static constexpr glm::vec2 pacmanInitialPosition {2.0f, 0.0f};
    static constexpr std::array<glm::vec2, sequenceLength> pacmanMotion {LEFT, LEFT, UP, UP, UP, UP, UP, UP};

    static constexpr std::array<glm::vec3, nGhosts> ghostColors {{ColorPalette::COLOR1, ColorPalette::COLOR2}};

    static constexpr std::array<glm::vec2, nGhosts> ghostInitialPositions {{{-4.0f, 0.0f}, {3.0f,  1.0f}}};
    static constexpr std::array<std::array<glm::vec2, sequenceLength>, nGhosts> ghostMotions {{
        {RIGHT, RIGHT, RIGHT, RIGHT, UP,   UP,   UP, UP},
        {UP,    UP,    UP,    LEFT,  LEFT, LEFT, UP, UP}
    }};

    static constexpr uint32_t pacmanShapeN = 8;
    static constexpr uint32_t ghostShapeN = 6;
};

// Aliases for the datatypes of Hull and Vertices for easier usage
using Face = glm::uvec3;
using HullSegment = std::vector<Face>;
using Hull = std::vector<HullSegment>;
using Vertices = std::vector<glm::vec3>;

struct MazeTile {
    glm::vec3 v1, v2, v3, v4;
    glm::vec4 color;
};


#endif //COMPUTERGRAPHICS_GAME_H
