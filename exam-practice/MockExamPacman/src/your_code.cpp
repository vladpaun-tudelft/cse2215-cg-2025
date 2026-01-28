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
    std::vector<glm::vec2> points (numPoints);


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
        std::span<const glm::vec2> motionSteps) {
    Hull hull {};
    Vertices vertices {};

    const size_t polygonN = polygon.size();
    const size_t sequenceLength = motionSteps.size();


    return {hull, vertices};
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
std::vector<Ray> generatePacmanRays (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t) {

    if (t >= (float) motionSteps.size()) { return {}; }
    std::vector<Ray> rays {};




    return rays;
}

/*
 * NOTE Triangle Intersections: return true if something is hit, returns false otherwise.
 * Only find hits if they are closer than t stored in the ray and the intersection
 * is on the correct side of the origin (the new t >= 0).
 */


bool pointInTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& n, const glm::vec3& p)
{
    return false;
}

bool intersectRayWithPlane(const Plane& plane, Ray& ray)
{
    return false;
}

/*
 * Input: the three vertices of the triangle
 * Output: the plane (represented via its normal vector and distance from the origin) defined by the input vertices
 */
Plane trianglePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    Plane plane;
    return plane;
}

/*
 * Input: the three vertices of the triangle
 * Output: if ray intersects triangle defined by input vertices, then modify the hit parameter ray.t and return true, otherwise return false
 */
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray)
{
    return false;
}

/*
 * INTERSECT RAY WITH GHOSTS
 * Given the vertices of all ghosts and the hulls of all ghosts,
 * check if the ray intersects with any of their triangles
 */
bool intersectRayWithGhosts(const std::span<Vertices> vertices, const std::span<Hull> hulls, Ray &ray) {
    bool hit = false;


    return hit;
}
