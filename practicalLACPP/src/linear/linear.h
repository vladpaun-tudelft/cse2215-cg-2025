// Include the header files we need.
// !!! YOU DO NOT NEED TO CHANGE THIS !!!
#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <span>
#include <string>
#include <array>
#include <vector>

// Methods for handling vectors
glm::vec3 mul(const glm::vec3& lhs, float rhs);
float dot3(const glm::vec3& lhs, const glm::vec3& rhs);
glm::vec3 cross3(const glm::vec3& lhs, const glm::vec3& rhs);
float length(const glm::vec3& lhs);

// The matrix is defined as:
// | m00  m01  m02 |
// | m10  m11  m12 |
// | m20  m21  m22 |
//
// The columns are stored as Vector3's, where:
// col1 = (m00, m10, m20)
// col2 = (m01, m11, m21)
// col3 = (m02, m12, m22)
//
// !!! DO NOT MODIFY THIS STRUCT !!!
struct Matrix3 {
    glm::vec3 col1, col2, col3;
};
std::string to_string(const Matrix3& matrix);
Matrix3 mul(const Matrix3& lhs, float rhs);
Matrix3 transpose(const Matrix3& m);
float determinant(const Matrix3& m);
Matrix3 inverse(const Matrix3& matrix);

// plane is defined by a normal and point on the plane
// !!! DO NOT MODIFY THIS STRUCT !!!
struct Plane {
    glm::vec3 p;
    glm::vec3 n;
};
std::vector<int> orderOfnGonVertices(const std::vector<glm::vec3> nGon);
std::array<glm::vec3, 4> rectangleOnPlane(const Plane& plane);
std::vector<glm::vec3> verticesFromPlanes(std::span<const Plane> planes);
float areaOfTriangle(const std::array<glm::vec3, 3> triangle);
std::vector<std::array<glm::vec3, 3>> splitNGonIntoTriangles(const std::vector<glm::vec3> nGon);
float areaOfIrregularNGon(const std::vector<glm::vec3> nGon);
