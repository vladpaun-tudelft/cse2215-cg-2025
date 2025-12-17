#pragma once
#include <framework/mesh.h>
#include <framework/ray.h>
#include <array>

void drawCoordSystem(float length, std::array<glm::vec3, 3> colors);
void drawSphere(glm::vec3 center, float radius, glm::vec3 color);
void drawRay(const Ray& ray, std::array<glm::vec3, 3> colors);
