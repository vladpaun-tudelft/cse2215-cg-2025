#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <numbers>
#include <ranges>
#include "shading.h"

namespace solution {

// Shading

glm::vec3 LinearGradientSample(const LinearGradient& g, float ti);

// Recursive

Ray generateReflectionRay(const Ray &ray, const HitInfo &hitInfo);

void accumulateSpecularColor(const Material& material, glm::vec3 lightColor, glm::vec3 &result);

Ray generatePassthroughRay(const Ray &ray, const HitInfo &hitInfo);

void accumulateTransparentColor(const Material& material, glm::vec3 lightColor, glm::vec3& result);

// Texture

std::pair<int, glm::vec3> sampleTextureNearest(const Image& image, const glm::vec2& tx, bool flipped);

glm::vec3 sampleTextureBilinear(const Image& image, const glm::vec2& tx, bool flipped);

// Interpolation

glm::vec3 computeBarycentricCoord(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& p);

glm::vec3 interpolateNormal(const glm::vec3& n0, const glm::vec3& n1, const glm::vec3& n2, const glm::vec3 barycentricCoord);

glm::vec2 interpolateTexCoord(const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2, const glm::vec3 barycentricCoord);

// Light and shadows

glm::vec3 mixSegmentLightColor(const SegmentLight& light, const glm::vec3& position);

glm::vec3 mixParallelogramLightColor(const ParallelogramLight& light, const glm::vec3& position, const glm::mat3& parallel_inv);

// Multisampling

}
