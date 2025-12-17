#pragma once
#include <framework/disable_all_warnings.h>
// Suppress warnings in third-party code.
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include "linear.h"
#include <array>
#include <filesystem>

void drawTriangle(const std::array<glm::vec3, 3>& points);

void drawRectangle(const std::array<glm::vec3, 4>& points, const Plane& plane);

void drawCoordSystem(const float length = 1);

void drawSphere(const glm::vec3& position, float radius, float red = 0.8f, float green = 0.8f, float blue = 0.1f);
