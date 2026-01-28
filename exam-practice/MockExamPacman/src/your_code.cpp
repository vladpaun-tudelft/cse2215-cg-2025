#include "your_code.h"
#include "provided.h"

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

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * SUBMISSION: You may copy the lines below into the solution.cpp on weblab.
 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*
 * Generate Circle Points
 * We approximate PacMan based on points on its bounding circle
 * Function to generate points on a circle
 * r: radius of the circle
 * center: xy-coordinates of the center of the circle
 * numPoints: number of points
 */
std::vector<glm::vec2> generateCirclePoints(float radius, glm::vec2 center, size_t numPoints)
{
    std::vector<glm::vec2> points(numPoints);

    float two_pi = 2 * glm::pi<float>();
    float step = two_pi / (float)numPoints;

    for (size_t i = 0; i < numPoints; i++) {
        float angle = step * (float)i;
        float x = center.x + radius * glm::sin(angle);
        float y = center.y + radius * glm::cos(angle);
        points[i] = { x, y };
    }

    return points;
}

/*
 * GENERATE HULL GEOMETRY
 * Given
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost),
 *   the initialPosition of that shape provided as a 2D displacement in the xy-plane,
 *   and the motionSteps provided as a span of 2D displacements over time
 *
 * Generate a hull representing the moving shape in 3D spacetime.
 * The hull should consist of HullSegments representing constant motion
 * A new segment should only be started when a direction change happens
 * the HullSegments in Hull should be in the order of the motion steps
 *
 * Each segment consists of the faces defining its outer hull, where faces are simply triangles.
 * We are only interested in the surrounding hull, i.e. we discard triangles for the front and back caps of the segments
 *
 * The triangles are defined by the three indices of their three vertices in the vertex list.
 * The Vertex list should contain the vertices. You should reuse the vertices of neighbouring segments/faces.
 *
 * Return the Hull and the Vertices
 *
 * Once again, watch out: Positive time should develop in negative z direction
 */

// Aliases for the datatypes of Hull and Vertices for easier usage
using Face = glm::uvec3;
using HullSegment = std::vector<Face>;
using Hull = std::vector<HullSegment>;
using Vertices = std::vector<glm::vec3>;

std::tuple<Hull, Vertices> generateHullGeometry(
    std::span<const glm::vec2> polygon,
    const glm::vec2 initialPosition,
    std::span<const glm::vec2> motionSteps)
{
    Hull hull {};
    Vertices vertices {};

    const size_t polygonN = polygon.size();
    const size_t sequenceLength = motionSteps.size();

    std::vector<std::pair<glm::vec2, int>> linearDisplacementSteps;

    if (!motionSteps.empty()) {
        glm::vec2 dir = motionSteps[0];
        glm::vec2 curr = dir;
        int duration = 1;

        for (size_t i = 1; i < sequenceLength; i++) {
            if (motionSteps[i] == dir) {
                curr += motionSteps[i];
                duration++;
            } else {
                linearDisplacementSteps.push_back({ curr, duration });
                dir = motionSteps[i];
                curr = dir;
                duration = 1;
            }
        }
        linearDisplacementSteps.push_back({ curr, duration });
    }

    float t = 0.0;
    glm::vec2 currPos = initialPosition;

    for (glm::vec2 p : polygon) {
        vertices.push_back({ p.x + currPos.x, p.y + currPos.y, -t });
    }
    for (std::pair<glm::vec2, int> step : linearDisplacementSteps) {
        currPos += step.first;
        t += (float)step.second;
        for (glm::vec2 p : polygon) {
            vertices.push_back({ p.x + currPos.x, p.y + currPos.y, -t });
        }
    }

    const size_t ringCount = linearDisplacementSteps.size() + 1;
    for (size_t r = 0; r + 1 < ringCount; r++) {
        unsigned int ring0StartIdx = static_cast<unsigned int>(r * polygonN);
        unsigned int ring1StartIdx = static_cast<unsigned int>((r + 1) * polygonN);
        HullSegment segment;
        for (size_t i = 0; i < polygonN; i++) {
            unsigned int next = static_cast<unsigned int>((i + 1) % polygonN);
            unsigned int a = ring0StartIdx + static_cast<unsigned int>(i);
            unsigned int b = ring0StartIdx + next;
            unsigned int c = ring1StartIdx + next;
            unsigned int d = ring1StartIdx + static_cast<unsigned int>(i);
            segment.push_back({ a, b, c });
            segment.push_back({ a, c, d });
        }
        hull.push_back(segment);
    }

    return { hull, vertices };
}
/*
 * GENERATE PACMAN RAYS
 * Generate rays for the pacman collision tests
 *
 * Given the time of interest t
 * as well as the polygon, its initial position, and the motions, defined as before;
 * Generate rays from its vertices in its current motion direction
 *
 * Return the rays as a vector of rays.
 *
 * Hint: You can use getPolygonT(...) to obtain the vertex positions of the polygon at the requested point in time.
 * You find the function signature in provided.h
 */
std::vector<Ray> generatePacmanRays(
    std::span<const glm::vec2> polygon,
    const glm::vec2& initialPosition,
    std::span<const glm::vec2> motionSteps,
    const float t)
{

    if (t >= (float)motionSteps.size()) {
        return {};
    }
    std::vector<Ray> rays {};

    std::vector<glm::vec2> newPolygon = applyInitialPosition(polygon, initialPosition);
    newPolygon = applyMovementInTime(newPolygon, motionSteps, t);
    glm::vec3 dir = { motionSteps[(size_t)std::floor(t)], -1.0 };
    dir = glm::normalize(dir);
    for (glm::vec2 o : newPolygon) {
        glm::vec3 origin = { o, -t };
        rays.push_back({ origin, dir });
    }

    return rays;
}

/*
 * NOTE Triangle Intersections: return true if something is hit, returns false otherwise.
 * Only find hits if they are closer than t stored in the ray and the intersection
 * is on the correct side of the origin (the new t >= 0).
 */

bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p)
{
    glm::vec3 a = v0;
    glm::vec3 b = v1;
    glm::vec3 c = v2;

    glm::vec3 normal = glm::cross((b - a), (c - a));

    glm::vec3 na = glm::cross((c - b), (p - b));
    glm::vec3 nb = glm::cross((a - c), (p - c));
    glm::vec3 nc = glm::cross((b - a), (p - a));

    float alpha = glm::dot(normal, na) / glm::dot(normal, normal);
    float beta = glm::dot(normal, nb) / glm::dot(normal, normal);
    float gamma = glm::dot(normal, nc) / glm::dot(normal, normal);

    if ((alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f) || (alpha <= 0.0f && beta <= 0.0f && gamma <= 0.0f)) {
        return true;
    }
    return false;
}

bool intersectRayWithPlane(const Plane& plane, Ray& ray)
{
    float div = glm::dot(ray.direction, plane.normal);
    if (div == 0)
        return false;
    float t = (plane.D - glm::dot(ray.origin, plane.normal)) / div;
    if (t < 0)
        return false;
    if (t < ray.t)
        ray.t = t;
    return true;
}

/*
 * Input: the three vertices of the triangle
 * Output: the plane (represented via its normal vector and distance from the origin) defined by the input vertices
 */
Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    Plane plane;

    plane.normal = glm::normalize(glm::cross((v1 - v0), (v2 - v0)));
    plane.D = glm::dot(plane.normal, v0);

    return plane;
}

/*
 * Input: the three vertices of the triangle
 * Output: if ray intersects triangle defined by input vertices, then modify the hit parameter ray.t and return true, otherwise return false
 */
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray)
{
    Plane plane = trianglePlane(v0, v1, v2);
    float old_t = ray.t;
    if (intersectRayWithPlane(plane, ray)) {
        glm::vec3 p = ray.origin + ray.direction * ray.t;
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (pointInTriangle(v0, v1, v2, n, p))
            return true;
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
