#pragma once
#include "common.h"
#include <framework/ray.h>

/*
 * Provided function for triangle intersection tests. The implementations are provided in the precompiled libraries.
 * intersectRayWithTriangle is a reference implementation of the triangle intersection method you implemented in Assignment 4A.
 * Given: A triangle defined by 3 vertices and a ray.
 * If an intersection is found that is closer than the current intersection (<= ray.t), ray.t is updated and true returned.
 * Otherwise, ray is not updated and false returned.
 */
bool intersectRayWithTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, Ray& ray);
