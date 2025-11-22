#include "linear.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>
#include <algorithm>

std::string to_string(const Matrix3& matrix)
{
    return "matrix with columns:\n Col1 " + to_string(matrix.col1) + "\n Col2 " + to_string(matrix.col2) + "\n Col3 " + to_string(matrix.col3);
}

// ==================================
// ========    Exercise 0    ========
// ==================================
// Multiplication of a vector with a scalar
glm::vec3 mul(const glm::vec3& lhs, float rhs)
{
    glm::vec3 result {};
    // Your solution goes here
    // Do *NOT* use glm functionality here, but implement the multiplication yourself.
    return result;
}

// Dot product of two vectors
float dot3(const glm::vec3& lhs, const glm::vec3& rhs)
{
    float result;
    // Your solution goes here
    // Do *NOT* use glm::dot here, but implement the dot product yourself.
    result = 0.0f;
    return result;
}

// Cross product of two vectors
glm::vec3 cross3(const glm::vec3& lhs, const glm::vec3& rhs)
{
    glm::vec3 result;
    // Your solution goes here
    // Do *NOT* use glm::cross here, but implement the cross product yourself.
    return result;
}

// Length of a vector
float length(const glm::vec3& lhs)
{
    float result;
    // Your solution goes here
    // Do *NOT* use glm::length here, but implement the length yourself.
    result = 0.0f;
    return result;
}

// For the following exercises, a matrix is defined as:
// | m00  m01  m02 |
// | m10  m11  m12 |
// | m20  m21  m22 |
//
// The columns are stored as Vector3's, where:
// col1 = (m00, m10, m20)
// col2 = (m01, m11, m21)
// col3 = (m02, m12, m22)
//
// For a Matrix3 matrix object, these can be accessed via matrix.col1, matrix.col2, matrix.3.

// Matrix multiplication with a scalar
Matrix3 mul(const Matrix3& lhs, float rhs)
{
    Matrix3 result {};
    // Your solution goes here
    return result;
}

// Taking the transpose of a matrix means changing it's columns into rows and vice versa.
// Given a matrix of the form:
// | m00  m01  m02 |
// | m10  m11  m12 |
// | m20  m21  m22 |
// its transpose is given as:
// | m00  m10  m20 |
// | m01  m11  m21 |
// | m02  m12  m22 |
Matrix3 transpose(const Matrix3& m)
{
    Matrix3 result {};
    // Your solution goes here
    return result;
}

// The determinant is needed to compute the inverse of a matrix.
// If you need a Linear Algebra refresher, please check out:
// https://www.tudelft.nl/en/eemcs/study/online-education/math-explained/linear-algebra/#c144161
float determinant(const Matrix3& m)
{
    float result;
    // Your solution goes here
    result = 0.0f;
    return result;
}

// Computing the inverse of the given matrix. If you implemented it correctly then matrix M multiplied
// by its inverse should give the identity matrix). More information on how to compute the inverse of a
// 3x3 matrix can be found here:
// https://www.mathsisfun.com/algebra/matrix-inverse-minors-cofactors-adjugate.html
// For this method, you can assume that the given matrix is invertable, i.e., that the determinant is
// not zero. However, you are responsible for checking that all matrices passed to your function in your 
// own code are invertible.
Matrix3 inverse(const Matrix3& matrix)
{
    Matrix3 result {};
    // Hint:
    // You can follow, e.g., the method described here:
    // https://www.mathsisfun.com/algebra/matrix-inverse-minors-cofactors-adjugate.html

    // Step 0: It is probably handy to define a method that computes the determinant of a 2x2 matrix.
    // Step 1: Compute the Matrix of Minors.
    // Step 2: Compute the Matrix of Cofactors
    // Step 3: Adjugate the Matrix (Note that you have the transpose available from above).
    // Step 4: Multiply by 1/determinant (Note that you have the multiplication available from above).
    return result;
}

// ==================================
// ========    Exercise 1    ========
// ==================================
// Given a set of vertices of a regular or irregular n-Gon (n>2) on a plane in 3D, we want to order them clock- or counterclockwise
// You may assume that the n-Gon is convex.
std::vector<int> orderOfnGonVertices(const std::vector<glm::vec3> nGon)
{
    std::vector<int> result;
    // You can assume that all vertices of the n-Gon are residing in some plane in 3D, that there are at least 3 vertices in the n-Gon,
    // and that the n-Gon itself is convex.
    // Your solution here, result should have the indices of the vertices from the n-Gon in either counter- or clockwise order.
    return result;
}


// ==================================
// ========    Exercise 2    ========
// ==================================
// To visualize a given plane, we want to find four points on this plane that span a rectangle.
std::array<glm::vec3, 4> rectangleOnPlane(const Plane& plane)
{
    std::array<glm::vec3, 4> result;

    // Hint:
    // You are given the normal vector and a base point of the plane via the Plane struct.
    // (1) Find a vector that is perpendicular to the normal vector. You can assume that the normal vector is not the zero-vector. 
    //     You can use the dot product you implemented above to check that the vectors are indeed perpendicular.
    // (2) Use the cross-product to compute a second vector that is perpendicular to both the vector found in (1) and the normal 
    //     of the given plane. Again, you can check yourself using the dot product implemented above.
    // (3) As these two vectors you found span the plane, use them to offset the base point in four different directions.
    //     To be able to draw a nice rectangle from them, be sure to order them either clockwise or counter-clockwise.

    // Regarding the visuals: It's really about passing four points that lie on that plane. The rectangles can be tiny, intersect, 
    // be rotated, ... whatever, we don't care. The one thing you should check is that the four points actually lie on the plane 
    // and have the shape of (roughly) a square.

    return result;
}

// A solid can be given by the planes that are surrounding it. We want to find the vertices of a solid given by its planes.
std::vector<glm::vec3> verticesFromPlanes(std::span<const Plane> planes)
{
    std::vector<glm::vec3> result;

    // Hint:
    // You are given a set of planes.
    // A triple of these planes can intersect in a point.
    // There are two things to watch out for:
    // - a triple of planes might not have a common intersection.
    // - a triple of planes might have a common intersection point that is not within the solid.

    // Therefore:
    // - Iterate over all triples (p1,p2,p3) of given planes.
    // - Set up a linear system that represents the intersection of the three planes, you can ignore the case where the three planes intersect in a line.
    // - Test whether the system has a solution (you can, e.g., use the determinant for matrices).
    // - If it has a solution, compute it (e.g., via using the inverse matrix).
    // - Assuming that the normal of the plane points to the inside of the solid, test for all other planes whether the computed point of intersection is indeed within the solid.

    return result;
}

// ==================================
// ========    Exercise 3    ========
// ==================================
// Given a (possibly irregular) polygon by its vertices (at least 3, ordered either clock- or counterclockwise) that all lie on some 2D plane, 
// compute its area by splitting it into triangles and summing up their areas.
// You may assume that the n-Gon is convex.

float areaOfTriangle(const std::array<glm::vec3, 3> triangle)
{
    float result;
    // Your solution here
    return result;
}

std::vector<std::array<glm::vec3, 3>> splitNGonIntoTriangles(const std::vector<glm::vec3> nGon)
{
    std::vector<std::array<glm::vec3, 3>> result;
    // Your solution here
    return result;
}

float areaOfIrregularNGon(const std::vector<glm::vec3> nGon)
{
    // Split the nGon into triangles
    std::vector<std::array<glm::vec3, 3>> triangles = splitNGonIntoTriangles(nGon);
    // Sum up their areas
    float area = 0.;
    for (const auto& triangle : triangles) {
        area += areaOfTriangle(triangle);
    }
    return area;
}
