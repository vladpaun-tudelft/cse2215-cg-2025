#pragma once
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <framework/ray.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()
#include <vector>
#include <span>

#include "game.h"

std::vector<MazeTile> generateMaze(glm::ivec2 mazeCenter, const Game::Maze &maze, float time);

void drawMaze(std::span<const MazeTile> tiles);

std::vector<glm::vec2> generateCirclePoints(float radius, glm::vec2 center, size_t numPoints);

void drawPolygon(std::span<const glm::vec2> polygon, float time, const glm::vec4& color);

void drawPolygonOutline(std::span<const glm::vec2> polygon, float time, const glm::vec4& color);

std::vector<glm::vec2> applyInitialPosition(std::span<const glm::vec2> polygon, const glm::vec2& initialPosition);

std::vector<glm::vec2> applyMovementInTime(std::span<const glm::vec2> polygon,
                                           std::span<const glm::vec2> motionSteps,
                                           float t);

std::vector<glm::vec2> getPolygonT(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        float t);

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

std::tuple<Hull, Vertices> generateHullGeometry(
        std::span<const glm::vec2> polygon,
        glm::vec2 initialPosition,
        std::span<const glm::vec2> motionSteps);

void drawHullMesh(const Vertices& vertices, const Hull& hull, glm::vec4 color);

std::vector<Ray> generatePacmanRays (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        float t);
