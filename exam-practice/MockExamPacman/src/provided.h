#ifndef COMPUTERGRAPHICS_PROVIDED_H
#define COMPUTERGRAPHICS_PROVIDED_H

#include <framework/disable_all_warnings.h>
#include <framework/ray.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()
#include <vector>
#include <span>
#include "game.h"


/***********
 ***********
 * Maybe relevant for you, if you want to use getPolygonT(...) and don't remember the behavior from the exercise
 ***********
 ***********/
/*
 * APPLY INITIAL POSITIONS
 * Get the initial 2D (xy) location of the vertices of the polygon (pacman or ghosts) given:
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost)
 *   and the initialPosition of that shape provided as a 2D displacement in the xy-plane.
 * Return the displaced polygon
 */
std::vector<glm::vec2> applyInitialPosition(std::span<const glm::vec2> polygon, const glm::vec2& initialPosition);

/*
 * APPLY MOVEMENT IN TIME
 * Get the 2D (xy) location of the vertices of the polygon (pacman or ghosts) at time t
 * Given
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost),
 *   the motionSteps provided as a span of 2D displacements over time,
 *   and the time of interest t
 *
 * Return the displaced polygon
 */
std::vector<glm::vec2> applyMovementInTime(std::span<const glm::vec2> polygon,
                                           std::span<const glm::vec2> motionSteps,
                                           float t);

inline std::vector<glm::vec2> getPolygonT(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t) {
    std::vector<glm::vec2> translatedPolygon;
    translatedPolygon.assign(polygon.begin(), polygon.end());
    translatedPolygon = applyInitialPosition(translatedPolygon, initialPosition);
    translatedPolygon = applyMovementInTime(translatedPolygon, motionSteps, t);
    return translatedPolygon;
}

/***********
 ***********
 * Likely not relevant for you: internals known from the exercise, likely not required for your task
 ***********
 ***********/
std::vector<MazeTile> generateMaze(glm::ivec2 mazeCenter, const Game::Maze &maze, float time);
void drawMaze(std::span<const MazeTile> tiles);
void drawPolygon(std::span<const glm::vec2> polygon, float time, const glm::vec4& color);
void drawPolygonOutline(std::span<const glm::vec2> polygon, float time, const glm::vec4& color);
void drawPolygonT(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        float t,
        glm::vec3 color);
void drawSelectedPolygon (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        float t,
        glm::vec3 color);
void drawHullMesh(const Vertices& vertices, const Hull& hull, glm::vec4 color);

#endif //COMPUTERGRAPHICS_PROVIDED_H
