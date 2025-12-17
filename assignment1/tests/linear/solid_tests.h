#include "helpers.h"
#include "linear.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <ostream>
#include <random>
#include <tuple>

// Provide planes and vertices for a tetrahedron with vertices (1,0,0), (-1,0,0), (0,1,0), and (0,0,1)
std::vector<Plane> tetrahedron()
{

    std::vector<Plane> planes;
    planes.push_back(Plane { glm::vec3 { 0.0, 0.0, 0.0 }, glm::vec3 { 0.0, 0.0, 1.0 } });
    planes.push_back(Plane { glm::vec3 { 1.0, 0.0, 0.0 }, glm::vec3 { 0.0, 1.0, 0.0 } });
    planes.push_back(Plane { glm::vec3 { 0.0, 1.0, 0.0 }, glm::vec3 { -1.0, -1.0, -1.0 } });
    planes.push_back(Plane { glm::vec3 { 0.0, 1.0, 0.0 }, glm::vec3 { 1.0, -1.0, -1.0 } });
    return planes;
}
std::vector<glm::vec3> tetrahedronVertices()
{
    std::vector<glm::vec3> vertices;
    vertices.push_back(glm::vec3 { 1.0, 0.0, 0.0 });
    vertices.push_back(glm::vec3 { -1.0, 0.0, 0.0 });
    vertices.push_back(glm::vec3 { 0.0, 1.0, 0.0 });
    vertices.push_back(glm::vec3 { 0.0, 0.0, 1.0 });
    return vertices;
}

// Provide planes for a frustum with vertices (2,2,0), (-2,2,0), (-2,-2,0), (2,-2,0), (1,1,1), (-1,1,1), (-1,-1,1), and (1,-1,1).
std::vector<Plane> frustum()
{
    std::vector<Plane> planes;
    planes.push_back(Plane { glm::vec3 { 0.0, 0.0, 0.0 }, glm::vec3 { 0.0, 0.0, 1.0 } });
    planes.push_back(Plane { glm::vec3 { 0.0, 0.0, 1.0 }, glm::vec3 { 0.0, 0.0, -1.0 } });
    planes.push_back(Plane { glm::vec3 { 1.0, -1.0, 1.0 }, glm::vec3 { -1.0, 0.0, -1.0 } });
    planes.push_back(Plane { glm::vec3 { 1.0, -1.0, 1.0 }, glm::vec3 { 0.0, 1.0, -1.0 } });
    planes.push_back(Plane { glm::vec3 { -1.0, 1.0, 1.0 }, glm::vec3 { 1.0, 0.0, -1.0 } });
    planes.push_back(Plane { glm::vec3 { -1.0, 1.0, 1.0 }, glm::vec3 { 0.0, -1.0, -1.0 } });
    return planes;
}
std::vector<glm::vec3> frustumVertices()
{
    std::vector<glm::vec3> vertices;
    vertices.push_back(glm::vec3 { 2.0, 2.0, 0.0 });
    vertices.push_back(glm::vec3 { -2.0, 2.0, 0.0 });
    vertices.push_back(glm::vec3 { -2.0, -2.0, 0.0 });
    vertices.push_back(glm::vec3 { 2.0, -2.0, 0.0 });
    vertices.push_back(glm::vec3 { 1.0, 1.0, 1.0 });
    vertices.push_back(glm::vec3 { -1.0, 1.0, 1.0 });
    vertices.push_back(glm::vec3 { -1.0, -1.0, 1.0 });
    vertices.push_back(glm::vec3 { 1.0, -1.0, 1.0 });
    return vertices;
}

std::array<glm::vec2, 4> projectToPlane(std::array<glm::vec3, 4> points, Plane plane)
{
    glm::vec3 normal = plane.n;
    normal = glm::normalize(normal);

    // Follow https://math.stackexchange.com/questions/1956699/getting-a-transformation-matrix-from-a-normal-vector in building a rotation matrix that sets the normal to be the z axis of the new coordinate system.
    glm::mat3x3 rotation = glm::mat3x3(
        normal.y / std::sqrt((normal.x * normal.x) + (normal.y * normal.y)),
        (normal.x * normal.z) / std::sqrt((normal.x * normal.x) + (normal.y * normal.y)),
        normal.x,
        (-normal.x) / std::sqrt((normal.x * normal.x) + (normal.y * normal.y)),
        (normal.y * normal.z) / std::sqrt((normal.x * normal.x) + (normal.y * normal.y)),
        normal.y,
        0.0,
        -std::sqrt((normal.x * normal.x) + (normal.y * normal.y)),
        normal.z);

    // Move the points to the origin via subtracting the plane's base point, then rotate into x-y-plane
    std::array<glm::vec2, 4> projectedPoints;
    for (int i = 0; i < points.size(); i++) {
        glm::vec3 projectedPoint = rotation * (points[i] - plane.p);
        projectedPoints[i] = glm::vec2 { projectedPoint.x, projectedPoint.y };
    }
    return projectedPoints;
}

// Given three collinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
        return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == APPROX_FLOAT(0))
        return 0; // collinear

    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are collinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1))
        return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1))
        return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2))
        return true;

    // p2, q2 and q1 are collinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2))
        return true;

    return false; // Doesn't fall in any of the above cases
}

TEST_CASE("Planar_Points")
{
    SECTION("Planarity")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto point = randomVectorGenerator.generate();
            const auto normal = randomVectorGenerator.generate();
            const Plane plane = { point, normal };

            std::array<glm::vec3, 4> points;
            try {
                points = rectangleOnPlane(plane);
            } catch (const std::exception&) {
                std::fill(std::begin(points), std::end(points), point + normal);
            }

            // Test whether all vertices actually lie on the plane
            CAPTURE(
                point,
                normal,
                glm::dot(points[0], normal),
                glm::dot(points[1], normal),
                glm::dot(points[2], normal),
                glm::dot(points[3], normal),
                dot(point, normal));
            REQUIRE((
                glm::dot(points[0], normal) == APPROX_FLOAT(dot(point, normal)) && glm::dot(points[1], normal) == APPROX_FLOAT(dot(point, normal)) && glm::dot(points[2], normal) == APPROX_FLOAT(dot(point, normal)) && glm::dot(points[3], normal) == APPROX_FLOAT(dot(point, normal))));
        }
    }
}

TEST_CASE("Cyclic_Order")
{
    SECTION("Order")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto point = randomVectorGenerator.generate();
            const auto normal = randomVectorGenerator.generate();
            const Plane plane = { point, normal };
            
            std::array<glm::vec3, 4> points;
            try {
                points = rectangleOnPlane(plane);
            } catch (const std::exception&) {
                std::fill(std::begin(points), std::end(points), point + normal);
            }            

            std::array<glm::vec2, 4> projectedPoints = projectToPlane(points, plane);

            CAPTURE(
                "Plane base point and normal: ",
                plane.p, 
                plane.n
            );
            // Test whether all vertices are ordered by performing an intersection check
            // https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
            REQUIRE((
                !doIntersect(projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3])
                && !doIntersect(projectedPoints[0], projectedPoints[3], projectedPoints[1], projectedPoints[2])));
        }
    }
}

TEST_CASE("Solid_Vertices_Correct")
{
    SECTION("Tetrahedron")
    {
        // Check whether the vertices provided are all vertices of the tetrahedron
        std::vector<Plane> planes = tetrahedron();
        std::vector<glm::vec3> verticesRef = tetrahedronVertices();
        std::vector<glm::vec3> vertices = verticesFromPlanes(planes);
        vertices.resize(verticesRef.size(), glm::vec3(-1000));

        std::vector<bool> verticesRefFound(verticesRef.size(), false);
        for (int i = 0; i < verticesRef.size(); i++) {
            for (int j = 0; j < verticesRef.size(); j++) {
                // Found a reference vertex corresponding to the computed vertex
                if (vertices[i] == verticesRef[j]) {
                    verticesRefFound[j] = true;
                    break;
                }
            }
        }

        CAPTURE(
            vertices,
            "tetrahedron with vertices (1,0,0), (-1,0,0), (0,1,0), and (0,0,1)");
        for (bool found : verticesRefFound) {
            REQUIRE(found);
        }
    }

    SECTION("Frustum")
    {
        // Check whether the vertices provided are all vertices of the tetrahedron
        std::vector<Plane> planes = frustum();
        std::vector<glm::vec3> verticesRef = frustumVertices();
        std::vector<glm::vec3> vertices = verticesFromPlanes(planes);
        vertices.resize(verticesRef.size(), glm::vec3(-1000));

        std::vector<bool> verticesRefFound(verticesRef.size(), false);
        for (int i = 0; i < verticesRef.size(); i++) {
            bool vertexFound = false;
            for (int j = 0; j < verticesRef.size(); j++) {
                // Found a reference vertex corresponding to the computed vertex
                if (vertices[i] == verticesRef[j]) {
                    verticesRefFound[j] = true;
                    break;
                }
            }
        }

        CAPTURE(
            vertices,
            "frustum with vertices (2,2,0), (-2,2,0), (-2,-2,0), (2,-2,0), (1,1,1), (-1,1,1), (-1,-1,1), and (1,-1,1)");
        for (bool found : verticesRefFound) {
            REQUIRE(found);
        }
    }
}
