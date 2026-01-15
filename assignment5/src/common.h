#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/mesh.h>

enum class DrawMode {
    Filled,
    Wireframe
};

struct HitData {
    glm::vec3 normal;
    Material material;
};

struct AxisAlignedBox {
    glm::vec3 lower { 0.0f };
    glm::vec3 upper { 1.0f };
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
};

struct SegmentLight {
    glm::vec3 endpoint0, endpoint1; // Positions of endpoints
    glm::vec3 color0, color1; // Color of endpoints
};

struct ParallelogramLight {
    // A parallelogram light (see figure 3.14 of chapter 13.4.2 of Fundamentals of CG 4th Edition)
    glm::vec3 v0; // v0
    glm::vec3 edge01, edge02; // edges from v0 to v1, and from v0 to v2
    glm::vec3 color0, color1, color2, color3;
};
