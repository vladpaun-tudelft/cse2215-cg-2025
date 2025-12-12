#ifndef COMPUTERGRAPHICS_COMMON_H
#define COMPUTERGRAPHICS_COMMON_H

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()

using Color = glm::vec3;

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};

// !!! DO NOT MODIFY THIS STRUCT EXCEPT FOR THE VALUES !!!
struct MaterialInformation {
    Color Kd { 0.5f, 0.5f, 0.5f }; // Diffuse coefficient per vertex.
    Color Ks { 0.5f, 0.5f, 0.5f }; // Specularity coefficient per vertex.
    float shininess = 20.0f; // Exponent for Phong and Blinn-Phong specularities per vertex.

    // Gooch shading parameters
    float goochB = 0.55f;
    float goochY = 0.3f;
    float goochAlpha = 0.15f;
    float goochBeta = 0.6f;
};

#endif //COMPUTERGRAPHICS_COMMON_H
