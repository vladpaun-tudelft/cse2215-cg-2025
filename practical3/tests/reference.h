#pragma once
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>
#include <vector>
#include "../src/common.h"

namespace reference {
Color referenceDiffuseOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos,
                           const glm::vec3& normal, const glm::vec3& lightPos, const Color& lightColor);

MaterialInformation referenceGetMaterialEveningCar(const Color& dayLight, const Color& eveningLight,
                                                   const MaterialInformation& dayCarMaterial);

Color referencePhongSpecularOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos,
                                 const glm::vec3& normal, const glm::vec3& cameraPos,
                                 const glm::vec3& lightPos, const Color& lightColor);

Color referenceBlinnPhongSpecularOnly(const MaterialInformation& materialInformation, const glm::vec3& vertexPos,
                                      const glm::vec3& normal, const glm::vec3& cameraPos,
                                      const glm::vec3& lightPos, const Color& lightColor);

Color referenceDiffPhongSpecularOnly(const glm::vec3& phong, const glm::vec3& blinnPhong);

Color referenceGooch(const MaterialInformation& materialInformation, const glm::vec3& vertexPos,
                     const glm::vec3& normal, const glm::vec3& lightPos, const Color& lightColor, const int n);

// ======================= Table part ==========================
glm::vec3 referenceOptimalLightSourceLocLambertian(const MaterialInformation& materialInformation,
                                                   const glm::vec3& vertexPos, const glm::vec3& normal,
                                                   const Color& lightColor, const glm::vec3 L1, const glm::vec3 L2);


// ======================= Thermosolar power plant part ==========================
glm::vec3 referenceComputeReflection(const glm::vec3& normal, const glm::vec3& incomingLightDirection);

glm::vec3 referenceOptimalMirrorNormal(const glm::vec3& mirrorPos, const glm::vec3& incomingLightDirection,
                                       glm::vec3 targetVertexPos);


// ======================= BRDF Viewer Part ==========================
void generateSphereVertices(int n_latitude,
							int m_longitude,
                            std::vector<glm::vec3>& sphereVertices);

void displaceVerticesByIntensity(const std::vector<glm::vec3>& vertexColors, std::vector<glm::vec3>& sphereVertices);

void generateSphereMesh(uint32_t n_latitude,
                        uint32_t m_longitude,
                        std::vector<glm::uvec3>& triangles,
                        std::vector<glm::uvec4>& quads);


// ====== More helpers ======
glm::vec3 _reflect(const glm::vec3& n, const glm::vec3& l);

}  // END NAMESPACE REFERENCE

