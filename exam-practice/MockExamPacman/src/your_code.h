#ifndef COMPUTERGRAPHICS_YOUR_CODE_H
#define COMPUTERGRAPHICS_YOUR_CODE_H

#include <framework/disable_all_warnings.h>
#include <framework/ray.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()
#include <vector>
#include <span>
#include "game.h"

std::vector<glm::vec2> generateCirclePoints(float radius, glm::vec2 center, size_t numPoints);

std::tuple<Hull, Vertices> generateHullGeometry(
        std::span<const glm::vec2> polygon,
        glm::vec2 initialPosition,
        std::span<const glm::vec2> motionSteps);

std::vector<Ray> generatePacmanRays (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        float t);


struct Plane {
        float D = 0.0f;
        glm::vec3 normal { 0.0f, 1.0f, 0.0f };
};

bool intersectRayWithPlane(const Plane& plane, Ray& ray);

// Returns true if the point p is inside the triangle spanned by v0, v1, v2 with normal n.
bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p);

Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);

bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray);

using Face = glm::uvec3;
using HullSegment = std::vector<Face>;
using Hull = std::vector<HullSegment>;
using Vertices = std::vector<glm::vec3>;
bool intersectRayWithGhosts(std::span<Vertices> vertices, std::span<Hull> hulls, Ray &ray);

#endif //COMPUTERGRAPHICS_YOUR_CODE_H
