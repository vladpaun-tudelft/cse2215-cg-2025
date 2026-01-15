#pragma once
#include "scene.h"
#include <framework/mesh.h>
#include <framework/ray.h>
#include <utility> // std::forward

// Flag to enable/disable the debug drawing.
extern bool enableDebugDraw;

void drawRay(const Ray& ray, const glm::vec3& color = glm::vec3(1.0f));

void drawAABB(const AxisAlignedBox& box, DrawMode drawMode = DrawMode::Filled, const glm::vec3& color = glm::vec3(1.0f), float transparency = 1.0f);

void drawTriangle (const Vertex& v0, const Vertex& v1, const Vertex& v2 );
void drawMesh(const Mesh& mesh);
void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(1.0f));
void drawScene(const Scene& scene);

