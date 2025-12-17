#include "helpers.h"
#include "linear.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <ostream>
#include <random>
#include <tuple>

TEST_CASE("Vector_Operands")
{
    SECTION("Scalar Multiplication")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        RandomGenerator randomScalarGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto vec1 = randomVectorGenerator.generate();
            const auto scl1 = randomScalarGenerator.generate();
            const auto reference = vec1 * scl1;

            CAPTURE(vec1, scl1, mul(vec1, scl1), reference);
            REQUIRE(mul(vec1, scl1) == reference);
        }
    }
}

TEST_CASE("Vector_Operations")
{
    SECTION("DotProduct")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto vec1 = randomVectorGenerator.generate();
            const auto vec2 = randomVectorGenerator.generate();
            const auto reference = glm::dot(vec1, vec2);

            CAPTURE(vec1, vec2, dot3(vec1, vec2), reference);
            REQUIRE(dot(vec1, vec2) == reference);
        }
    }

    SECTION("CrossProduct")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto vec1 = randomVectorGenerator.generate();
            const auto vec2 = randomVectorGenerator.generate();
            const auto reference = glm::cross(vec1, vec2);

            CAPTURE(vec1, vec2, cross3(vec1, vec2), reference);
            REQUIRE(cross(vec1, vec2) == reference);
        }
    }

    SECTION("Length")
    {
        RandomVectorGenerator randomVectorGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto vec = randomVectorGenerator.generate();
            const float reference = glm::length(vec);

            CAPTURE(vec, length(vec));
            REQUIRE(length(vec) == APPROX_FLOAT(reference));
        }
    }
}

TEST_CASE("Matrix_Operands")
{
    SECTION("Scalar Multiplication")
    {
        RandomMatrixGenerator randomMatrixGenerator { -5.0f, +5.0f };
        RandomGenerator randomScalarGenerator { -5.0f, +5.0f };
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto [mat1, ref1] = randomMatrixGenerator.generate();
            const auto scl1 = randomScalarGenerator.generate();
            const auto ref = ref1 * scl1;

            CAPTURE(mat1, scl1, mul(mat1, scl1));
            REQUIRE(mul(mat1, scl1) == ref);
        }
    }
}

TEST_CASE("Matrix_Operations")
{
    SECTION("Transpose")
    {
        RandomMatrixGenerator randomMatrixGenerator;
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto [mat, ref] = randomMatrixGenerator.generate();
            const auto reference = glm::transpose(ref);

            CAPTURE(mat, transpose(mat), reference);
            REQUIRE(transpose(mat) == reference);
        }
    }

    SECTION("Determinant")
    {
        RandomMatrixGenerator randomMatrixGenerator;
        for (size_t i = 0; i < testsPerSection; i++) {
            const auto [mat, ref] = randomMatrixGenerator.generate();
            const float reference = glm::determinant(ref);

            CAPTURE(mat, determinant(mat), reference);
            REQUIRE(determinant(mat) == APPROX_FLOAT(reference));
        }
    }

    SECTION("Inverse")
    {
        RandomMatrixGenerator randomMatrixGenerator;
        size_t i = 0;
        while (i < testsPerSection) {
            const auto [mat, ref] = randomMatrixGenerator.generate();
            // Skip matrices which are almost singular (non invertible).
            const float det = glm::determinant(ref);
            if (std::abs(det) < 0.01f)
                continue;
            try {
                const auto reference = glm::inverse(ref);
                CAPTURE(mat, inverse(mat), reference);
                REQUIRE(inverse(mat) == reference);
            } catch (const std::exception) {
                CAPTURE(ref, "Exception thrown");
                REQUIRE(false);
            }
            ++i;
        }
    }
}
