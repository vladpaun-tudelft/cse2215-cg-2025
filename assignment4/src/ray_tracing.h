#pragma once
#include <cfloat>
#include <vector>
#include <span>
#include <framework/ray.h>

// NOTE(Mathijs): return true if something is hit, returns false otherwise.
// Only find hits if they are closer than t stored in the ray and the intersection
// is on the correct side of the origin (the new t >= 0).

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


struct TestScene{
    Vertices vertices;

    struct Triangle {
        glm::uvec3 vertex_indices;
        glm::vec3 color;
    };

    std::vector<Triangle> triangles;
};

TestScene generateTestScene ();
void drawTestScene (const TestScene &testScene);
bool intersectTestScene (const TestScene &testScene, Ray &ray);
