#ifndef COMPUTERGRAPHICS_REFERENCE_PACMAN_H
#define COMPUTERGRAPHICS_REFERENCE_PACMAN_H

#include <iostream>
#include "../src/ray_tracing.h"
#include "../src/game.h"

namespace reference {
    std::ostream &operator<<(std::ostream &stream, const MazeTile &maze_tile);
    std::vector<MazeTile> generateMaze(glm::ivec2 mazeCenter, const Game::Maze &maze, float time);
    std::vector<glm::vec2> generateCirclePoints(float radius, glm::vec2 center, size_t numPoints);
    std::vector<glm::vec2> applyInitialPosition(std::span<const glm::vec2> polygon, const glm::vec2& initialPosition);
    std::vector<glm::vec2> applyMovementInTime(std::span<const glm::vec2> polygon,
                                               std::span<const glm::vec2> motionSteps,
                                               float t);
    std::vector<glm::vec2> getPolygonT(
            std::span<const glm::vec2> polygon,
            const glm::vec2& initialPosition,
            std::span<const glm::vec2> motionSteps,
            float t);

    std::tuple<Hull, Vertices> generateHullGeometry(
            std::span<const glm::vec2> polygon,
            glm::vec2 initialPosition,
            std::span<const glm::vec2> motionSteps);
    std::vector<Ray> generatePacmanRays (
            std::span<const glm::vec2> polygon,
            const glm::vec2& initialPosition,
            std::span<const glm::vec2> motionSteps,
            float t);

    bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p);
    bool intersectRayWithPlane(const Plane& plane, Ray& ray);
    Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
    bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray);
    bool intersectRayWithGhosts(std::span<Vertices> vertices, std::span<Hull> hulls, Ray &ray);
}


#endif //COMPUTERGRAPHICS_REFERENCE_PACMAN_H
