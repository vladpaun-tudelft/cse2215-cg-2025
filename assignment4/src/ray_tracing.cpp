#include "ray_tracing.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <framework/opengl_includes.h>

DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtx/string_cast.hpp>
DISABLE_WARNINGS_POP()
#include <cmath>
#include <iostream>
#include <limits>


bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p)
{
    glm::vec3 a = v0;
    glm::vec3 b = v1;
    glm::vec3 c = v2;

    glm::vec3 normal = glm::cross((b-a),(c-a));

    glm::vec3 na = glm::cross((c - b), (p - b));
    glm::vec3 nb = glm::cross((a - c), (p - c));
    glm::vec3 nc = glm::cross((b - a), (p - a));

    float alpha = glm::dot(normal, na) / glm::dot(normal, normal);
    float beta = glm::dot(normal, nb) / glm::dot(normal, normal);
    float gamma = glm::dot(normal, nc) / glm::dot(normal, normal);

    if ((alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f) ||
        (alpha <= 0.0f && beta <= 0.0f && gamma <= 0.0f)) {
        return true;
    }
    return false;
}

bool intersectRayWithPlane(const Plane& plane, Ray& ray)
{
    float div = glm::dot(ray.direction, plane.normal);
    if (div == 0) return false;
    float t = (plane.D - glm::dot(ray.origin, plane.normal)) / div;
    if (t < 0) return false;
    if (t < ray.t) ray.t = t;
    return true;
}

/// Input: the three vertices of the triangle
/// Output: the plane (represented via its normal vector and distance from the origin) defined by the input vertices
Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    Plane plane;

    plane.normal = glm::normalize(glm::cross((v1-v0),(v2-v0)));
    plane.D = glm::dot(plane.normal,v0);

    return plane;
}

/// Input: the three vertices of the triangle
/// Output: if ray intersects triangle defined by input vertices, then modify the hit parameter ray.t and return true, otherwise return false
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray)
{
    Plane plane = trianglePlane(v0,v1,v2);
    float old_t = ray.t;
    if (intersectRayWithPlane(plane, ray)) {
        glm::vec3 p = ray.origin + ray.direction * ray.t;
        glm::vec3 n = glm::normalize(glm::cross(v1-v0,v2-v0));
        if (pointInTriangle(v0,v1,v2,n,p)) return true;
        else {
            ray.t = old_t;
        }
    }
    return false;
}

/*
 * INTERSECT RAY WITH GHOSTS
 * Given the vertices of all ghosts and the hulls of all ghosts,
 * check if the ray intersects with any of their triangles
 */
bool intersectRayWithGhosts(const std::span<Vertices> vertices, const std::span<Hull> hulls, Ray& ray)
{
    bool hit = false;
    for (size_t i = 0; i < hulls.size(); i++) {
        Hull hull = hulls[i];
        Vertices verts = vertices[i];
        for (HullSegment hullSegment : hull) {
            for (Face face : hullSegment) {
                if (intersectRayWithTriangle(verts[face.x], verts[face.y], verts[face.z], ray)) {
                    hit = true;
                }
            }
        }
    }

    return hit;
}
/*
 *
 * TEST SCENE
 * If you want to test your triangle intersections without using the ghost hull,
 * feel free to implement a test scene here and use the UI option test scene.
 * This part serves the purpose of simplifying your debugging and is not required.
 */

TestScene generateTestScene () {
    // implement this properly if you need it
    return {
        .vertices = {{1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0}},
        .triangles = {{.vertex_indices= {0, 1, 2}, .color={0.5, 0.5, 0.5}}}
    };
}

void drawTestScene (const TestScene &testScene) {
    // implement this properly if you need it
    glBegin(GL_TRIANGLES);
    glColor3fv(glm::value_ptr(testScene.triangles[0].color));
    glVertex3fv(glm::value_ptr(testScene.vertices[testScene.triangles[0].vertex_indices[0]]));
    glVertex3fv(glm::value_ptr(testScene.vertices[testScene.triangles[0].vertex_indices[1]]));
    glVertex3fv(glm::value_ptr(testScene.vertices[testScene.triangles[0].vertex_indices[2]]));
    glEnd();
}

bool intersectTestScene (const TestScene &testScene, Ray &ray) {
    // implement this properly if you need it
    return intersectRayWithTriangle(testScene.vertices[testScene.triangles[0].vertex_indices[0]],
                                    testScene.vertices[testScene.triangles[0].vertex_indices[1]],
                                    testScene.vertices[testScene.triangles[0].vertex_indices[2]],
                                    ray);
}
