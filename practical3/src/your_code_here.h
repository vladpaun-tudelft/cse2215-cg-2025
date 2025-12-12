#ifndef SHADING_ASSIGNMENT_YOUR_CODE_HERE
#define SHADING_ASSIGNMENT_YOUR_CODE_HERE

#pragma once
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <framework/opengl_includes.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()
#include <array>
#include <iostream>
#include <limits>
#include <span>
#include <utility>
#include <vector>
#include "common.h"

void printVec3(const glm::vec3& vec)
{
    std::cout << "(" << vec.x << "," << vec.y << "," << vec.z << ")" << std::endl;
}

// ==========================
// =====    EXERCISE    =====
// ==========================

// ======================= First part ==========================

// Debug function.
Color debugColor(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& cameraPos, const glm::vec3& lightPos, const Color& lightColor)
{
    // This function you can use in any way you like!
    // E.g., for debugging purposes!
    return (normal + 1.0f) / 2.0f;

    // or random color per vertex:
    // const size_t hashX = std::hash<float>()(vertexPos.x);
    // const size_t hashY = std::hash<float>()(vertexPos.y);
    // const size_t hashZ = std::hash<float>()(vertexPos.z);
    // return Color {
    //     (double)hashX / std::numeric_limits<size_t>::max(),
    //     (double)hashY / std::numeric_limits<size_t>::max(),
    //     (double)hashZ / std::numeric_limits<size_t>::max()
    // };

    // or material information:
    // return materialInformation.Kd;
}

// Standard lambertian shading: I * Kd * dot(N,L), clamped to zero when the light is illuminating the surface from behind, where L is the light vector and I is the light color.
Color diffuseOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& lightPos, const Color& lightColor)
{
    return glm::vec3(0, 0, 1);
}

// Set the correct material of the evening car to appear with the same color under evening light as the day car at daylight
MaterialInformation getMaterialEveningCar(const Color& dayLight, const Color& eveningLight, const MaterialInformation& dayCarMaterial)
{
    MaterialInformation eveningCarMaterial = dayCarMaterial;
    return eveningCarMaterial;
}

// Phong (!) Shading Specularity (http://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model)
// When computing specularities like this, verify that the light is on the illuminated side of the surface, with respect to the normal
// E.g., for a plane, the light source below the plane cannot cast light on the top, hence, there can also not be any specularity.
Color phongSpecularOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& cameraPos, const glm::vec3& lightPos, const Color& lightColor)
{
    return glm::vec3(0, 1, 0);
}

// Blinn-Phong Shading Specularity (http://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model)
// Be careful!!! The pseudo code does some additional modifications to the formula.
// The same test, regarding on which side the light source is, should be used.
Color blinnPhongSpecularOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& cameraPos, const glm::vec3& lightPos, const Color& lightColor)
{

    return glm::vec3(0, 0, 1);
}

// Difference between Phong and Blinn-Phong
// The goal is to visualize the change in specularities, the assignment gives detailed instructions.
Color diffPhongSpecularOnly(const glm::vec3& phong, const glm::vec3& blinnPhong)
{
    return glm::vec3(0, 0, 1);
}

// Gooch shading model
// A shading model that uses an interpolation between a warmer and cooler version of the original color.
// In this way, light can bend around surfaces, which can be beneficial for shape visualization purposes - also, it can look very nice.
// Here, you are asked to implement a toon shaded version, as described in the assignment.
Color gooch(const MaterialInformation& materialInformation, const glm::vec3& vertexPos, const glm::vec3& normal, const glm::vec3& lightPos, const Color& lightColor, const int n)
{
    return glm::vec3(0, 0, 1);
}

// ======================= Thermosolar power plant part ==========================

// RETURN the reflected ray direction (vector) to help you  visualize the current configuration.
glm::vec3 computeReflection(const glm::vec3& normal, const glm::vec3& incomingLightDirection)
{
    return normal;
}

// RETURN the optimal normal for the mirror, such that the light towards the given direction is reflected at mirrorPos towards the center of the pole (targetVertexPos)
glm::vec3 optimalMirrorNormal(const glm::vec3& mirrorPos, const glm::vec3& incomingLightDirection, const glm::vec3& targetVertexPos)
{
    return glm::normalize(glm::vec3(1.0, 1.0, -1.0));
}

// ======================= Shading Model Viewer ==========================

// DRAW SURFACE POINT
void drawSurfacePoint()
{
    constexpr float pointSize = 10;

    // use GL_POINTS to draw a point (quad) at the origin
    // use a color of your choice and the provided pointSize
}

// DRAW SURFACE PATCH
void drawSurfacePatch()
{
    constexpr float halfLength = 1.0f;

    // Draw a surface patch at the origin with side lengths 2*halfLength using GL_QUADS
    // use a color of your choice
}

// DRAW SURFACE BOUNDARY
void drawSurfaceBoundary()
{
    constexpr float halfLength = 1.0f;
    constexpr float lineWidth = 5.0f;

    // Draw the boundary of the surface using GL_LINES
    // use a color of your choice and the provided line width
}

// Draw Vector Helper for drawLightDirectionAndNormal(...)
void drawVector(const glm::vec3& origin, const glm::vec3& directionVector, const glm::vec3& color)
{
    constexpr float lineWidth = 4.0f;
    constexpr float pointSize = 8.0f;

    // Draw the directionVector starting at origin using GL_LINES and GL_POINTS for the tip.
    // Use the provided lineWidth, pointSize, and light color.
}

// DRAW LIGHT DIRECTION AND SURFACE NORMAL
void drawLightDirectionAndNormal(const Light& light)
{
    // Draw the normalized normal vector at the center of the patch using drawVector(...)
    drawVector({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.2f, 0.2f, 1.0f });

    // Draw the vector towards the light source using drawVector(...)
    drawVector({ 0.0f, 0.0f, 0.0f }, light.position, light.color);
}

// GENERATE SPHERE GRID
// Generate a sphere grid (that will later be deformed for the shading model reflection directions)
// n_latitude defines the desired amount of segments along the latitude
// m_longitude defines the desired amount of segments along the longitude

// The resulting sphere should have the center (0, 0, 0) and radius 1
// The top pole should be located at (0, 1, 0)
// To define a unique mesh representation we further constrain that one of the vertical subdivision lines has to have z=0
// If you use a natural implementation this is most likely the case, if it is not, it might be a sign that you are doing things to complicated
std::vector<glm::vec3> generateSphereVertices(int n_latitude, int m_longitude)
{
    // We will use a simple method for the tessellation of a sphere
    // The latitude is subdivided into n segments of equal angular width by lines parallel to the equator and manually added top and bottom vertices
    // The longitude is subdivided into m segments of equal size by lines from one pole to the other

    std::vector<glm::vec3> sphereVertices;

    // Add the top vertex of the sphere to sphereVertices

    // Create a list of sphereVertices at the intersections of the aforementioned subdivision lines

    // Add the bottom vertex of the sphere to sphereVertices

    if (!sphereVertices.empty()) {
        assert(sphereVertices.size() == (n_latitude - 1) * m_longitude + 2);
    }

    return sphereVertices;
}

// DISPLACE VERTICES BY INTENSITY

// Provide all necessary information for the computation of the light reflected from the point in the different directions (represented by the vertices of the sphere)
// In the next functions we will hand you the vertex colors calculated based on the parameters you provide here.
// Given the sphereVertices, return the position, normalVector, and viewDirections for the light computation of the vertices.
// Note: The position, normalVector, and viewDirection parameters are references. You are supposed to "return" the correct values by assigning to them.
// Only the sphere vertices are an "actual" parameter to this function.
// That means assign to these references the position of the point we want to compute the illumination of, its normal vector, and all view directions we want to compute the illumination for.
// The solution will look trivial, we just want you to quickly reflect on what we are actually computing/visualising here.
void getReflectedLightInputParameters(const std::vector<glm::vec3>& sphereVertices, glm::vec3& position, glm::vec3& normalVector, std::vector<glm::vec3>& viewDirections)
{
    // position = ...
    // normalVector = ...
    // viewDirections = ...
}

// Displace all the vertices depending on the intensity
// We define intensity to be the 2-Norm of the corresponding color vector
// Further we define that no light is reflected (intensity=0) for directions below the surface
void displaceVerticesByIntensity(std::span<const glm::vec3> vertexColors, std::span<glm::vec3> sphereVertices)
{
    // Write the output to sphereVertices:
    //  sphereVertices[i] = glm::vec3(0);
}

// VISUALIZATION MODES POINTS AND VECTORS

// Visualization mode Points
// Draw sphere grid with displaced vertices based on reflected amount of light defined by the shading model
void drawSphereGrid(std::span<const glm::vec3> vertices, std::span<const glm::vec3> vertexColors)
{
    assert(vertices.size() == vertexColors.size());
    const float pointSize = 10;

    // Given the displaced vertices and the vertex colors, draw the deformed sphere using GL_POINTS
}

// Visualization mode Vectors
void drawSphereVectors(std::span<const glm::vec3> vertices, std::span<const glm::vec3> vertexColors)
{
    assert(vertices.size() == vertexColors.size());
    const float pointSize = 10;
    const float lineWidth = 4;

    // Draw the displacement vectors using GL_LINES and GL_POINTS for the tip
    // For the lines you can use a color of your choice, use the provided pointSize and lineWidth
}

// VISUALIZATION MODE MESH

// We first need to generate a surface mesh using our already generated vertices.
// To do so, we define a list of quads densely covering the sphere based on our existing vertices
// To connect the poles to the first/last row of vertices we use triangles
// The triangles and quads are defined by the indices of their vertices in sphereVertices (generateSphereVertices(...))
// so keep the layout (order) you used in mind when implementing this function
void generateSphereMesh(
    uint32_t n_latitude,
    uint32_t m_longitude,
    std::vector<glm::uvec3>& triangles,
    std::vector<glm::uvec4>& quads)
{

    // Create a list of quads for all the segments other than the ones at the poles
    // Each quad is defined by a uvec4 containing the indices of the four vertices in sphereVertices

    // Create a list of triangles for the connection of the vertices on the first and last lines with the corresponding poles
    // Each triangle is defined by a uvec3 containing the indices of the three vertices in sphereVertices

    if (!quads.empty()) {
        assert(quads.size() == m_longitude * (n_latitude - 2));
    }
    if (!triangles.empty()) {
        assert(triangles.size() == m_longitude * 2);
    }
}

// Now we can draw the generated mesh using GL_TRIANGLES and GL_QUADS
// Use the indices defining your triangles and quads to obtain the positions and colors in vertices and vertexColors
void drawSphereMesh(
    std::span<const glm::vec3> vertices,
    std::span<const glm::vec3> vertexColors,
    std::span<const glm::uvec3> triangles,
    std::span<const glm::uvec4> quads)
{

    // Loop through the triangles and draw them using GL_TRIANGLES

    // Loop through the quads and draw them using GL_QUADS
}

#endif // SHADING_ASSIGNMENT_YOUR_CODE_HERE
