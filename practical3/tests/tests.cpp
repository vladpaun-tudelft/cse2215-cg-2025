#include "../src/your_code_here.h"
#include "helpers.h"
#include "reference.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
DISABLE_WARNINGS_POP()

namespace glm {
    std::ostream& operator<<(std::ostream &stream, const glm::vec3 &vec) {
        stream << "vec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return stream;
    }
}

std::ostream& operator<<(std::ostream& stream, const MaterialInformation& materialInformation)
{
    stream << "MaterialInformation { ";
    stream << "Kd = " << materialInformation.Kd;
    stream << ", Ks = " << materialInformation.Ks;
    stream << ", shininess = " << materialInformation.shininess;
    stream << ", goochB = " << materialInformation.goochB;
    stream << ", goochY = " << materialInformation.goochY;
    stream << ", goochAlpha = " << materialInformation.goochAlpha;
    stream << ", goochAlpha = " << materialInformation.goochBeta;
    stream << "}";
    return stream;
}

TEST_CASE("Dream Car") {
    SECTION("Diffuse")
    {
        RandomVec3Generator vecGen;

        SECTION("Diffuse Complete Test")
        {
            for (size_t i = 0; i < testsPerSection; i++) {
                const MaterialInformation materialInformation {
                        .Kd = vecGen.generate()
                };
                const glm::vec3 vertexPos = 5.0f * vecGen.generate();
                const glm::vec3 lightPos = 5.0f * vecGen.generate();
                const glm::vec3 normal = glm::normalize(vecGen.generate());
                const glm::vec3 lightColor = vecGen.generate();

                const glm::vec3 studentResult = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                const glm::vec3 referenceResult = reference::referenceDiffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, studentResult, referenceResult);
                REQUIRE(glm::length(studentResult - referenceResult) == Catch::Approx(0.0f).margin(errorMargin));
            }
        }

        SECTION("[DIAGNOSTIC] Diffuse Basics")
        {
            RandomHemisphereGenerator hemisphere;

            const MaterialInformation materialInformation {
                    .Kd = glm::vec3(1.0f)
            };

            SECTION("Random normal")
            {
                const glm::vec3 vertexPos = glm::vec3(0);
                const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                const glm::vec3 lightColor = glm::vec3(1.0f);
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();

                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));

                    const glm::vec3 expected = glm::vec3(normal.y); // N*L => N*(0,1,0) => N.y
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector")
            {
                const glm::vec3 normal = glm::vec3(0, 1, 0);
                const glm::vec3 lightColor = glm::vec3(1.0f);
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = hemisphere.generate();

                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));

                    const glm::vec3 expected = glm::vec3(lightPos.y); // N*L => (0,1,0)*L => L.y
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector + random normal")
            {
                const glm::vec3 lightColor = glm::vec3(1.0f);
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();
                    const glm::vec3 lightPosSurfaceLocalSpace = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = normalCoordinateSystem(normal, lightPosSurfaceLocalSpace);

                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));

                    const glm::vec3 expected = glm::vec3(lightPosSurfaceLocalSpace.y);
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }
        }

        SECTION("[DIAGNOSTIC] Diffuse Correct computation of directions vectors")
        {
            RandomHemisphereGenerator hemisphere;
            RandomGenerator random;

            MaterialInformation materialInformation {};
            materialInformation.Kd = glm::vec3(1.0f);

            SECTION("Light vector computed relative to vertex position")
            {
                const glm::vec3 lightColor = glm::vec3(1.0f);
                const glm::vec3 normal = glm::vec3(0, 1, 0);
                const glm::vec3 baseLightPos = hemisphere.generate();
                const glm::vec3 vertexPos = glm::vec3(0.0f);
                const glm::vec3 lightPos = baseLightPos;
                const glm::vec3 expected = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);

                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 offset = 10.0f * glm::vec3(random.generate(), random.generate(), random.generate());
                    const glm::vec3 shiftedVertexPos = vertexPos + offset;
                    const glm::vec3 shiftedLightPos = lightPos + offset;
                    const glm::vec3 result = diffuseOnly(materialInformation, shiftedVertexPos, normal, shiftedLightPos, lightColor);
                    CAPTURE(materialInformation, shiftedVertexPos, normal, shiftedLightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor), diffuseOnly(materialInformation, shiftedVertexPos, normal, shiftedLightPos, lightColor));

                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Light vector is normalizd")
            {
                const glm::vec3 lightColor = glm::vec3(1.0f);
                const glm::vec3 normal = glm::vec3(0, 1, 0);
                for (size_t i = 0; i < testsPerSection; i++) {
                    // Light distance between 1 and 2.
                    const glm::vec3 lightPosNormalized = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPosDist1 = 1.0f * lightPosNormalized;
                    const glm::vec3 lightPosDist2 = 2.0f * lightPosNormalized;

                    const glm::vec3 resultWithLightAtDist1 = diffuseOnly(materialInformation, vertexPos, normal, lightPosDist1, lightColor);
                    const glm::vec3 resultWithLightAtDist2 = diffuseOnly(materialInformation, vertexPos, normal, lightPosDist2, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPosDist1, lightPosDist2, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPosDist1, lightColor), diffuseOnly(materialInformation, vertexPos, normal, lightPosDist2, lightColor));
                    REQUIRE(resultWithLightAtDist1.r == Catch::Approx(resultWithLightAtDist2.r).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.g == Catch::Approx(resultWithLightAtDist2.g).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.b == Catch::Approx(resultWithLightAtDist2.b).margin(errorMargin));
                }
            }
        }

        SECTION("[DIAGNOSTIC] Diffuse No Light From Behind")
        {
            const MaterialInformation materialInformation {
                    .Kd = glm::vec3(1.0f)
            };
            const Color lightColor = Color(1.0f);

            SECTION("Light source behind normal results in black")
            {
                {
                    // Testing for incorrect implementation of L=normalize(light_pos - vertex_pos)
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 vertexPos = glm::vec3(1.0f);
                    const glm::vec3 lightPos = vertexPos + glm::vec3(0, -1, 0);
                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, 0, 0);
                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, +1, -1);
                    const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }
            }
        }

        SECTION("[DIAGNOSTIC] Diffuse Colors")
        {
            const glm::vec3 vertexPos = glm::vec3(0);
            const glm::vec3 lightPos = glm::vec3(0, 1, 0);
            const glm::vec3 normal { 0, 1, 0 };

            SECTION("Kd color is used")
            {

                const MaterialInformation materialInformation {
                        .Kd = glm::vec3(0.2f, 0.5f, 0.8f)
                };
                const Color lightColor { 1.0f };

                const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));
                REQUIRE(result.r == Catch::Approx(0.2f).margin(errorMargin));
                REQUIRE(result.g == Catch::Approx(0.5f).margin(errorMargin));
                REQUIRE(result.b == Catch::Approx(0.8f).margin(errorMargin));
            }

            SECTION("Light color is used")
            {

                const MaterialInformation materialInformation {
                        .Kd = glm::vec3(1.0f)
                };
                const Color lightColor { 0.2f, 0.5f, 0.8f };

                const glm::vec3 result = diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, diffuseOnly(materialInformation, vertexPos, normal, lightPos, lightColor));
                REQUIRE(result.r == Catch::Approx(0.2f).margin(errorMargin));
                REQUIRE(result.g == Catch::Approx(0.5f).margin(errorMargin));
                REQUIRE(result.b == Catch::Approx(0.8f).margin(errorMargin));
            }
        }
    }
    SECTION("Car Material") {
        MaterialInformation dayCarMaterial {
                .Kd = glm::vec3(0.9f, 0.25f, 0.6f),
                .Ks = glm::vec3(0.0f, 0.0f, 0.0f),
                .shininess = 25.0f
        };

        glm::vec3 dayLight(1.0f);
        glm::vec3 eveningLight(0.9f, 0.7f, 0.7f);
        const MaterialInformation studentResult = getMaterialEveningCar(dayLight, eveningLight, dayCarMaterial);
        const MaterialInformation referenceResult = reference::referenceGetMaterialEveningCar(dayLight, eveningLight, dayCarMaterial);

        CAPTURE(dayCarMaterial, dayLight, eveningLight, studentResult, referenceResult);
        REQUIRE(glm::length(studentResult.Kd - referenceResult.Kd) == Catch::Approx(0.0f).margin(errorMargin));
    }

    SECTION("Phong") {
        SECTION("Phong Complete Test")
        {
            RandomVec3Generator vecGen;
            RandomGenerator randomGen;

            for (size_t i = 0; i < testsPerSection; ++i) {
                const glm::vec3 vertexPos = 5.0f * vecGen.generate();
                const glm::vec3 lightPos = 5.0f * vecGen.generate();
                const glm::vec3 cameraPos = 5.0f * vecGen.generate();
                const MaterialInformation materialInformation {
                        .Ks = vecGen.generate(),
                        .shininess = randomGen.generate()
                };
                const glm::vec3 normal = glm::normalize(vecGen.generate());

                vecGen.generate();
                const Color lightColor = Color { 1.0f };

                // Don't test camera looking at the back of a surface (wrong side of normal); degenerate mesh.
                if (glm::dot(cameraPos - vertexPos, normal) <= 0.01f)
                    continue;

                const glm::vec3 studentResult = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                const glm::vec3 referenceResult = reference::referencePhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, studentResult, referenceResult);
                REQUIRE(glm::length(studentResult - referenceResult) == Catch::Approx(0.0f).margin(errorMargin));
            }
        }

        SECTION("[DIAGNOSTIC] Phong Basics")
        {
            RandomHemisphereGenerator hemisphere;

            const MaterialInformation materialInformation {
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const glm::vec3 lightColor { 1 };

            SECTION("Random normal")
            {
                const glm::vec3 vertexPos = glm::vec3(0);
                const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = glm::normalize(glm::vec3(0, 1, 0) + hemisphere.generate());

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // Can leave out the clamping because of the way we generate normals.
                    const float cos = 2 * normal.y * normal.y - 1.0f;
                    const glm::vec3 expected { cos };
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector")
            {
                const glm::vec3 normal { 0, 1, 0 };
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = hemisphere.generate();
                    const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // R = -(L - 2 * dot(N, L) * N)
                    // If N = (0,1,0), then: R.y = -(L.y - 2 * L.y) = R.y = -(-L.y) = L.y
                    //
                    // With N=(0,1,0) and V=(0,1,0):
                    // dot(V, R) => dot(V, L) => L.y
                    const glm::vec3 expected = glm::vec3(lightPos.y);
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector + random normal")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();
                    const glm::vec3 baseLightPos = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = normalCoordinateSystem(normal, baseLightPos);
                    const glm::vec3 cameraPos = normalCoordinateSystem(normal, glm::vec3(0, 1, 0));

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // R = -(L - 2 * dot(N, L) * N)
                    // If N = (0,1,0), then: R.y = -(L.y - 2 * L.y) = R.y = -(-L.y) = L.y
                    //
                    // With N=(0,1,0) and V=(0,1,0):
                    // dot(V, R) => dot(V, L) => L.y
                    const glm::vec3 expected = glm::vec3(baseLightPos.y);
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random view vector")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                    const glm::vec3 cameraPos = hemisphere.generate();

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // With N=(0,1,0) and L=(0,1,0):
                    // dot(V, R) => dot(V, (0,1,0) => V.y
                    const glm::vec3 expected = glm::vec3(cameraPos.y);
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random view vector + random normal")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();
                    const glm::vec3 baseLightPos { 0, 1, 0 };
                    const glm::vec3 baseCameraPos = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = normalCoordinateSystem(normal, baseLightPos);
                    const glm::vec3 cameraPos = normalCoordinateSystem(normal, baseCameraPos);

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // With N=(0,1,0) and L=(0,1,0):
                    // dot(V, R) => dot(V, (0,1,0) => V.y
                    const glm::vec3 expected = glm::vec3(baseCameraPos.y);
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }
        }

        SECTION("[DIAGNOSTIC] Phong Correct computation of directions vectors")
        {
            RandomHemisphereGenerator hemisphere;
            RandomGenerator random;

            const MaterialInformation materialInformation {
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const glm::vec3 normal { 0, 1, 0 };
            const Color lightColor { 1 };

            SECTION("View/light vectors computed relative to vertex position")
            {
                {
                    const glm::vec3 vertexPos = glm::vec3(0, 2, 0);
                    const glm::vec3 lightPos = glm::vec3(0, 3, 0);
                    const glm::vec3 cameraPos = glm::vec3(1, 3, 0);

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    // With these settings, R=(0,1,0) so the result is V.y = 1/sqrt(2)
                    REQUIRE(result.r == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                }

                {
                    const glm::vec3 vertexPos = glm::vec3(0, 2, 0);
                    const glm::vec3 lightPos = glm::vec3(1, 3, 0);
                    const glm::vec3 cameraPos = glm::vec3(0, 3, 0);

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    // With these settings, R=(0,1,0) so the result is V.y = 1/sqrt(2)
                    REQUIRE(result.r == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(1.0f / std::sqrt(2.0f)).margin(errorMargin));
                }
            }

            SECTION("Light vector is normalized")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    // Light distance between 1 and 2.
                    const glm::vec3 lightPosNormalized = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPosDist1 = 1.0f * lightPosNormalized;
                    const glm::vec3 lightPosDist2 = 2.0f * lightPosNormalized;
                    const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                    const glm::vec3 resultWithLightAtDist1 = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightColor);
                    const glm::vec3 resultWithLightAtDist2 = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist2, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightPosDist2, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightColor), phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist2, lightColor));
                    REQUIRE(resultWithLightAtDist1.r == Catch::Approx(resultWithLightAtDist2.r).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.g == Catch::Approx(resultWithLightAtDist2.g).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.b == Catch::Approx(resultWithLightAtDist2.b).margin(errorMargin));
                }
            }

            SECTION("View vector is normalized")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    // Camera distance between 1 and 2.
                    const glm::vec3 cameraPosNormalized = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                    const glm::vec3 cameraPosDist1 = 1.0f * cameraPosNormalized;
                    const glm::vec3 cameraPosDist2 = 2.0f * cameraPosNormalized;

                    const glm::vec3 result1 = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist1, lightPos, lightColor);
                    const glm::vec3 result2 = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist2, lightPos, lightColor);

                    CAPTURE(materialInformation, vertexPos, normal, cameraPosDist1, cameraPosDist2, lightPos, lightColor, phongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist1, lightPos, lightColor), phongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist2, lightPos, lightColor));
                    REQUIRE(result1.r == Catch::Approx(result2.r).margin(errorMargin));
                    REQUIRE(result1.g == Catch::Approx(result2.g).margin(errorMargin));
                    REQUIRE(result1.b == Catch::Approx(result2.b).margin(errorMargin));
                }
            }

            SECTION("View/reflection vector angle pas 90 degrees results in 0")
            {
                // Camera distance between 1 and 2.
                const glm::vec3 cameraPos = glm::vec3(2.0f, 1.0f, 0.0f);
                const glm::vec3 vertexPos = glm::vec3(0.0f);
                const glm::vec3 lightPos = cameraPos;

                const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, result);
                REQUIRE(result.r == ApproxZero);
                REQUIRE(result.g == ApproxZero);
                REQUIRE(result.b == ApproxZero);
            }
        }

        SECTION("[DIAGNOSTIC] Phong No Light From Behind")
        {
            const MaterialInformation materialInformation{
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const Color lightColor = Color(1.0f);

            SECTION("Light source behind normal results in black")
            {
                // dot(R, V) > 0 but dot(N, L) < 0
                {
                    // Camera distance between 1 and 2.
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 cameraPos = glm::vec3(2.0f, 1.0f, 0.0f);
                    const glm::vec3 lightPos = glm::vec3(-2.0f, -1.0f, 0.0f);

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos,
                                                               lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, result);
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Light behind normal.
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 vertexPos = glm::vec3(0, 1, 0);
                    const glm::vec3 lightPos = vertexPos - normal;
                    const glm::vec3 cameraPos = vertexPos + normal;
                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos,
                                                               lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor,
                            phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, 0, 0);
                    const glm::vec3 cameraPos = normal;

                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos,
                                                               lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor,
                            phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, +1, -1);
                    const glm::vec3 cameraPos = normal;
                    const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos,
                                                               lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor,
                            phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }
            }
        }

        SECTION("[DIAGNOSTIC] Phong Colors")
        {
            RandomHemisphereGenerator hemisphere;
            RandomGenerator random;

            const glm::vec3 normal{0, 1, 0};
            const glm::vec3 vertexPos = glm::vec3(0);
            const glm::vec3 lightPos = glm::vec3(0, 1, 0);
            const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

            SECTION("Ks color is used")
            {
                const MaterialInformation materialInformation{
                        .Ks = glm::vec3(0.2f, 0.5f, 0.8f),
                        .shininess = 1.0f
                };
                const Color lightColor{1.0f};

                const glm::vec3 result = phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos,
                                                           lightColor);
                CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor,
                        phongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                REQUIRE(result.r == Catch::Approx(0.2f).margin(errorMargin));
                REQUIRE(result.g == Catch::Approx(0.5f).margin(errorMargin));
                REQUIRE(result.b == Catch::Approx(0.8f).margin(errorMargin));
            }

            SECTION("Shininess is used")
            {
                const Color lightColor{1.0f};

                const MaterialInformation materialInformationWithShininess1{.Ks = Color(1), .shininess = 1.0f};
                const MaterialInformation materialInformationWithShininess2{.Ks = Color(1), .shininess = 2.0f};
                const MaterialInformation materialInformationWithShininess3{.Ks = Color(1), .shininess = 3.0f};

                const glm::vec3 resultWithShininess1 = phongSpecularOnly(materialInformationWithShininess1, vertexPos,
                                                                         normal, cameraPos, lightPos, lightColor);
                const glm::vec3 resultWithShininess2 = phongSpecularOnly(materialInformationWithShininess2, vertexPos,
                                                                         normal, cameraPos, lightPos, lightColor);
                const glm::vec3 resultWithShininess3 = phongSpecularOnly(materialInformationWithShininess3, vertexPos,
                                                                         normal, cameraPos, lightPos, lightColor);
                CAPTURE(materialInformationWithShininess1, materialInformationWithShininess2,
                        materialInformationWithShininess3, vertexPos, normal, cameraPos, lightPos, lightColor,
                        resultWithShininess1, resultWithShininess2, resultWithShininess3);
                REQUIRE(resultWithShininess2.r ==
                      Catch::Approx(resultWithShininess1.r * resultWithShininess1.r).margin(errorMargin));
                REQUIRE(resultWithShininess2.g ==
                      Catch::Approx(resultWithShininess1.g * resultWithShininess1.g).margin(errorMargin));
                REQUIRE(resultWithShininess2.b ==
                      Catch::Approx(resultWithShininess1.b * resultWithShininess1.b).margin(errorMargin));

                REQUIRE(resultWithShininess3.r ==
                      Catch::Approx(resultWithShininess2.r * resultWithShininess1.r).margin(errorMargin));
                REQUIRE(resultWithShininess3.g ==
                      Catch::Approx(resultWithShininess2.g * resultWithShininess1.g).margin(errorMargin));
                REQUIRE(resultWithShininess3.b ==
                      Catch::Approx(resultWithShininess2.b * resultWithShininess1.b).margin(errorMargin));
            }
        }
    }
    SECTION("Blinn-Phong") {
        SECTION("Blinn-Phong")
        {
            RandomVec3Generator vecGen;
            RandomGenerator randomGen;

            size_t testsWithLightInFront = 0;
            //size_t testsRejected = 0;
            for (size_t i = 0; i < testsPerSection;) {
                const glm::vec3 vertexPos = 5.0f * vecGen.generate();
                const glm::vec3 normal = glm::normalize(vecGen.generate());
                const glm::vec3 lightPos = 5.0f * vecGen.generate();
                const glm::vec3 cameraPos = 5.0f * vecGen.generate();
                const MaterialInformation materialInformation {
                        .Ks = vecGen.generate(),
                        .shininess = randomGen.generate()
                };
                const Color lightColor = Color { 1.0f };

                // Don't test camera looking at the back of a surface (wrong side of normal); degenerate mesh.
                if (glm::dot(cameraPos - vertexPos, normal) <= 0.01f)
                    continue;
                if (glm::dot(lightPos - vertexPos, normal) >= 0.01f && testsWithLightInFront++ > (testsPerSection / 2)) {
                    //++testsRejected;
                    continue;
                }

                const glm::vec3 studentResult = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                const glm::vec3 referenceResult = reference::referenceBlinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, studentResult, referenceResult);
                REQUIRE(glm::length(studentResult - referenceResult) == Catch::Approx(0.0f).margin(errorMargin));
                ++i;
            }
        }

        SECTION("[DIAGNOSTIC] Blinn-Phong Basics")
        {
            RandomHemisphereGenerator hemisphere;

            const MaterialInformation materialInformation {
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const glm::vec3 lightColor { 1 };

            SECTION("Random normal")
            {
                const glm::vec3 vertexPos = glm::vec3(0);
                const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();

                    // Assuming L = (0, 1, 0) and V = (0, 1, 0)
                    //
                    // const glm::vec3 H = normalize(L + V)
                    // const glm::vec3 H = glm::vec3(0, 1, 0);
                    // const float cos = max(0, dot(N, H));
                    // const float cos = max(0, N.y);
                    // color = Ks * cos**shininess
                    //
                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // Can leave out the clamping because of the way we generate normals.
                    const glm::vec3 expected { normal.y };
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector")
            {
                const glm::vec3 normal { 0, 1, 0 };
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = hemisphere.generate();
                    const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // H = normalize(L + V)
                    // col = H * N = H.y
                    const glm::vec3 expected = glm::vec3((lightPos.y + 1.0f) / glm::length(lightPos + glm::vec3(0, 1, 0)));
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random light vector + random normal")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();
                    const glm::vec3 baseLightPos = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = normalCoordinateSystem(normal, baseLightPos);
                    const glm::vec3 cameraPos = normalCoordinateSystem(normal, glm::vec3(0, 1, 0));

                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // NOTE: vertex pos at origin and vectors are normalized, so L=lightPos and V=cameraPos.
                    const glm::vec3 expected = glm::vec3(glm::dot(normal, glm::normalize(lightPos + cameraPos)));
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random view vector")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                    const glm::vec3 cameraPos = hemisphere.generate();

                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // H = normalize(L + V)
                    // col = H * N = H.y
                    const glm::vec3 expected = glm::vec3((cameraPos.y + 1.0f) / glm::length(cameraPos + glm::vec3(0, 1, 0)));
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Random view vector + random normal")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 normal = hemisphere.generate();
                    const glm::vec3 baseLightPos { 0, 1, 0 };
                    const glm::vec3 baseCameraPos = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0);
                    const glm::vec3 lightPos = normalCoordinateSystem(normal, baseLightPos);
                    const glm::vec3 cameraPos = normalCoordinateSystem(normal, baseCameraPos);

                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    // NOTE: vertex pos at origin and vectors are normalized, so L=lightPos and V=cameraPos.
                    const glm::vec3 expected = glm::vec3(glm::dot(normal, glm::normalize(lightPos + cameraPos)));
                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }
        }

        SECTION("[DIAGNOSTIC] Blinn-Phong Correct computation of directions vectors")
        {
            RandomHemisphereGenerator hemisphere;
            RandomGenerator random;

            const MaterialInformation materialInformation {
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const glm::vec3 normal { 0, 1, 0 };
            const Color lightColor { 1 };

            SECTION("View/light vectors computed relative to vertex position")
            {
                const glm::vec3 baseLightPos = hemisphere.generate();
                const glm::vec3 baseCameraPos = hemisphere.generate();
                const glm::vec3 vertexPos = glm::vec3(0.0f);
                const glm::vec3 lightPos = baseLightPos;
                const glm::vec3 cameraPos = baseCameraPos;
                const glm::vec3 expected = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);

                for (size_t i = 0; i < testsPerSection; i++) {
                    const glm::vec3 offset = 10.0f * glm::vec3(random.generate(), random.generate(), random.generate());
                    const glm::vec3 shiftedVertexPos = vertexPos + offset;
                    const glm::vec3 shiftedLightPos = lightPos + offset;
                    const glm::vec3 shiftedCameraPos = cameraPos + offset;
                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, shiftedVertexPos, normal, shiftedCameraPos, shiftedLightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, shiftedVertexPos, normal, cameraPos, shiftedCameraPos, lightPos, shiftedLightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));

                    REQUIRE(result.r == Catch::Approx(expected.r).margin(errorMargin));
                    REQUIRE(result.g == Catch::Approx(expected.g).margin(errorMargin));
                    REQUIRE(result.b == Catch::Approx(expected.b).margin(errorMargin));
                }
            }

            SECTION("Light vector is normalized")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    // Light distance between 1 and 2.
                    const glm::vec3 lightPosNormalized = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPosDist1 = 1.0f * lightPosNormalized;
                    const glm::vec3 lightPosDist2 = 2.0f * lightPosNormalized;
                    const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

                    const glm::vec3 resultWithLightAtDist1 = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightColor);
                    const glm::vec3 resultWithLightAtDist2 = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist2, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightPosDist2, lightColor,
                            blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist1, lightColor),
                            blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPosDist2, lightColor));
                    REQUIRE(resultWithLightAtDist1.r == Catch::Approx(resultWithLightAtDist2.r).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.g == Catch::Approx(resultWithLightAtDist2.g).margin(errorMargin));
                    REQUIRE(resultWithLightAtDist1.b == Catch::Approx(resultWithLightAtDist2.b).margin(errorMargin));
                }
            }

            SECTION("View vector is normalized")
            {
                for (size_t i = 0; i < testsPerSection; i++) {
                    // Camera distance between 1 and 2.
                    const glm::vec3 cameraPosNormalized = hemisphere.generate();
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(0, 1, 0);
                    const glm::vec3 cameraPosDist1 = 1.0f * cameraPosNormalized;
                    const glm::vec3 cameraPosDist2 = 2.0f * cameraPosNormalized;

                    const glm::vec3 result1 = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist1, lightPos, lightColor);
                    const glm::vec3 result2 = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist2, lightPos, lightColor);

                    CAPTURE(materialInformation, vertexPos, normal, cameraPosDist1, cameraPosDist2, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist1, lightPos, lightColor), blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPosDist2, lightPos, lightColor));
                    REQUIRE(result1.r == Catch::Approx(result2.r).margin(errorMargin));
                    REQUIRE(result1.g == Catch::Approx(result2.g).margin(errorMargin));
                    REQUIRE(result1.b == Catch::Approx(result2.b).margin(errorMargin));
                }
            }
        }

        SECTION("[DIAGNOSTIC] Blinn-Phong No Light From Behind")
        {
            const MaterialInformation materialInformation {
                    .Ks = glm::vec3(1.0f),
                    .shininess = 1.0f
            };
            const Color lightColor = Color(1.0f);

            SECTION("Light source behind normal results in black")
            {
                {
                    // Light behind normal.
                    const glm::vec3 normal = glm::vec3(0, 1, 0);
                    const glm::vec3 vertexPos = glm::vec3(0, 5, 0);
                    const glm::vec3 lightPos = vertexPos + glm::vec3(-2, -1, 0);
                    const glm::vec3 cameraPos = vertexPos + normal;
                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, 0, 0);
                    const glm::vec3 cameraPos = normal;

                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }

                {
                    // Different axis.
                    const glm::vec3 normal = glm::vec3(1, 0, 0);
                    const glm::vec3 vertexPos = glm::vec3(0.0f);
                    const glm::vec3 lightPos = glm::vec3(-1, +1, -1);
                    const glm::vec3 cameraPos = normal;
                    const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor);
                    CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor, blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor));
                    REQUIRE(result.r == ApproxZero);
                    REQUIRE(result.g == ApproxZero);
                    REQUIRE(result.b == ApproxZero);
                }
            }
        }

        SECTION("[DIAGNOSTIC] Blinn-Phong Colors")
        {
            RandomHemisphereGenerator hemisphere;
            RandomGenerator random;

            const glm::vec3 normal{0, 1, 0};
            const glm::vec3 vertexPos = glm::vec3(0);
            const glm::vec3 lightPos = glm::vec3(0, 1, 0);
            const glm::vec3 cameraPos = glm::vec3(0, 1, 0);

            SECTION("Ks color is used")
            {
                const MaterialInformation materialInformation{
                        .Ks = glm::vec3(0.2f, 0.5f, 0.8f),
                        .shininess = 1.0f
                };
                const Color lightColor{1.0f};

                const glm::vec3 result = blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos,
                                                                lightPos, lightColor);
                CAPTURE(materialInformation, vertexPos, normal, cameraPos, lightPos, lightColor,
                        blinnPhongSpecularOnly(materialInformation, vertexPos, normal, cameraPos, lightPos,
                                               lightColor));
                REQUIRE(result.r == Catch::Approx(0.2f).margin(errorMargin));
                REQUIRE(result.g == Catch::Approx(0.5f).margin(errorMargin));
                REQUIRE(result.b == Catch::Approx(0.8f).margin(errorMargin));
            }

            SECTION("Shininess is used")
            {
                const Color lightColor{1.0f};

                const MaterialInformation materialInformationWithShininess1{.Ks = Color(1), .shininess = 1.0f};
                const MaterialInformation materialInformationWithShininess2{.Ks = Color(1), .shininess = 2.0f};
                const MaterialInformation materialInformationWithShininess3{.Ks = Color(1), .shininess = 3.0f};

                const glm::vec3 resultWithShininess1 = blinnPhongSpecularOnly(materialInformationWithShininess1,
                                                                              vertexPos, normal, cameraPos, lightPos,
                                                                              lightColor);
                const glm::vec3 resultWithShininess2 = blinnPhongSpecularOnly(materialInformationWithShininess2,
                                                                              vertexPos, normal, cameraPos, lightPos,
                                                                              lightColor);
                const glm::vec3 resultWithShininess3 = blinnPhongSpecularOnly(materialInformationWithShininess3,
                                                                              vertexPos, normal, cameraPos, lightPos,
                                                                              lightColor);
                CAPTURE(materialInformationWithShininess1, materialInformationWithShininess2,
                        materialInformationWithShininess3, vertexPos, normal, cameraPos, lightPos, lightColor,
                        resultWithShininess1, resultWithShininess2, resultWithShininess3);
                REQUIRE(resultWithShininess2.r ==
                      Catch::Approx(resultWithShininess1.r * resultWithShininess1.r).margin(errorMargin));
                REQUIRE(resultWithShininess2.g ==
                      Catch::Approx(resultWithShininess1.g * resultWithShininess1.g).margin(errorMargin));
                REQUIRE(resultWithShininess2.b ==
                      Catch::Approx(resultWithShininess1.b * resultWithShininess1.b).margin(errorMargin));

                REQUIRE(resultWithShininess3.r ==
                      Catch::Approx(resultWithShininess2.r * resultWithShininess1.r).margin(errorMargin));
                REQUIRE(resultWithShininess3.g ==
                      Catch::Approx(resultWithShininess2.g * resultWithShininess1.g).margin(errorMargin));
                REQUIRE(resultWithShininess3.b ==
                      Catch::Approx(resultWithShininess2.b * resultWithShininess1.b).margin(errorMargin));
            }
        }
    }

    SECTION("Specular difference")
    {
        RandomVec3Generator vecGen;
        RandomGenerator randomGen;

        for (size_t i = 0; i < testsPerSection; ++i) {
            const glm::vec3 phong = vecGen.generate();
            const glm::vec3 blinn_phong = vecGen.generate();

            const glm::vec3 studentResult = diffPhongSpecularOnly(phong, blinn_phong);
            const glm::vec3 studentResultSwapped = diffPhongSpecularOnly(blinn_phong, phong);
            const glm::vec3 referenceResult = reference::referenceDiffPhongSpecularOnly(phong, blinn_phong);
            CAPTURE(phong, blinn_phong, studentResult, studentResultSwapped, referenceResult);
            REQUIRE(glm::min(glm::length(studentResult - referenceResult), glm::length(studentResultSwapped - referenceResult)) == ApproxZero);
        }
    }

    SECTION("Gooch") {
        RandomVec3Generator vecGen;
        RandomGenerator floatGen;

        SECTION("Gooch Diffuse Shading Complete Test")
        {
            const auto testsWithLightOne = testsPerSection * 8 / 10;
            for (size_t i = 0; i < testsPerSection; i++) {
                MaterialInformation materialInformation {
                        .Kd = vecGen.generate(),
                        .goochB = floatGen.generate(),
                        .goochY = floatGen.generate(),
                        .goochAlpha = floatGen.generate(),
                        .goochBeta = floatGen.generate()
                };
                ensureSumLowerThan1(materialInformation.goochB, materialInformation.goochAlpha);
                ensureSumLowerThan1(materialInformation.goochY, materialInformation.goochBeta);

                const int n = int(5.0f * floatGen.generate()) + 1;
                const glm::vec3 normal = glm::normalize(vecGen.generate());
                const glm::vec3 lightColor = i < testsWithLightOne ? glm::vec3(1) : vecGen.generate();
                const glm::vec3 lightPos = 5.0f * vecGen.generate();
                const glm::vec3 vertexPos = 5.0f * vecGen.generate();

                // Flip light pos such that the result would have warm/cool swapped.
                const glm::vec3 flippedLightPos = lightPos - 2.0f * glm::dot(lightPos - vertexPos, normal) * normal;

                const glm::vec3 studentResult = runStudentGooch(materialInformation, vertexPos, normal, lightPos, lightColor, n);
                const glm::vec3 studentResultFlippedWarmCool = runStudentGooch(materialInformation, vertexPos, normal, flippedLightPos, lightColor, n);

                const glm::vec3 referenceResult = reference::referenceGooch(materialInformation, vertexPos, normal, lightPos, lightColor, n);
                CAPTURE(materialInformation, vertexPos, normal, lightPos, lightColor, studentResult, studentResultFlippedWarmCool, referenceResult);
                REQUIRE(std::min(glm::length(studentResult - referenceResult), glm::length(studentResultFlippedWarmCool - referenceResult)) == Catch::Approx(0.0f).margin(errorMargin));
            }
        }

        SECTION("Warm color when light is straight above (vertex at origin)")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos { 0, 1, 0 };

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - yellow) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Warm color when light is straight above (vertex not at origin)")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 1 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, 1, 0);

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - yellow) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Cold color when light is straight below (vertex at origin)")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos { 0, -1, 0 };

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - blue) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Half way between warm and cool")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(1, 0, 0);

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };
            const glm::vec3 halfWay = 0.5f * (yellow + blue);

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - halfWay) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Light at 45 degree from normal")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::normalize(glm::vec3(1, 1, 0));

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };
            const float weightWarm = 0.5f * (1.0f + glm::cos(glm::radians(45.0f)));
            const glm::vec3 halfWay = weightWarm * yellow + (1.0f - weightWarm) * blue;

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - halfWay) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Light at -45 degree from normal")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::normalize(glm::vec3(1, -1, 0));

            const glm::vec3 yellow { mat.goochY, mat.goochY, 0 };
            const glm::vec3 blue { 0, 0, mat.goochB };
            const float weightCool = 0.5f * (1.0f + glm::cos(glm::radians(45.0f)));
            const glm::vec3 halfWay = (1.0f - weightCool) * yellow + weightCool * blue;

            const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
            REQUIRE(glm::length(studentResult - halfWay) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("goochB controls k_cool")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0.0f),
                    .goochB = 0.5f,
                    .goochY = 0.0f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, -1, 0);

            for (int i = 0; i < 10; ++i) {
                mat.goochB = (float)i / 10.0f;
                const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
                const glm::vec3 expected_k_cool { 0, 0, mat.goochB };
                CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
                REQUIRE(glm::length(studentResult - expected_k_cool) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
            }
        }

        SECTION("goochY controls k_warm")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0.0f),
                    .goochB = 0.0f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, 1, 0);

            for (int i = 0; i < 10; ++i) {
                mat.goochY = (float)i / 10.0f;
                const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
                const glm::vec3 expected_k_warm { mat.goochY, mat.goochY, 0 };
                CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
                REQUIRE(glm::length(studentResult - expected_k_warm) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
            }
        }

        SECTION("goochAlpha controls k_d in k_cool")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(1.0f, 0.8f, 0.6f),
                    .goochB = 0.0f,
                    .goochY = 0.0f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, -1, 0);

            for (int i = 0; i < 10; ++i) {
                mat.goochAlpha = (float)i / 10.0f;
                const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
                const glm::vec3 expected_k_cool = mat.Kd * mat.goochAlpha;
                CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
                REQUIRE(glm::length(studentResult - expected_k_cool) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
            }
        }

        SECTION("goochBeta controls k_d in k_warm")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(1.0f, 0.8f, 0.6f),
                    .goochB = 0.0f,
                    .goochY = 0.0f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, 1, 0);

            for (int i = 0; i < 10; ++i) {
                mat.goochBeta = (float)i / 10.0f;
                const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, 1000);
                const glm::vec3 expected_k_warm = mat.Kd * mat.goochBeta;
                CAPTURE(mat, vertexPos, normal, lightPos, lightColor, studentResult);
                REQUIRE(glm::length(studentResult - expected_k_warm) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
            }
        }

        SECTION("Color does not depend on light distance")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };
            const glm::vec3 lightPosAtDist05 = vertexPos + glm::vec3(0, 0.5f, 0);
            const glm::vec3 lightPosAtDist1 = vertexPos + glm::vec3(0, 1.0f, 0);

            const glm::vec3 studentResultAtDist05 = runStudentGooch(mat, vertexPos, normal, lightPosAtDist05, lightColor, 1000);
            const glm::vec3 studentResultAtDist1 = runStudentGooch(mat, vertexPos, normal, lightPosAtDist1, lightColor, 1000);
            CAPTURE(mat, vertexPos, normal, lightPosAtDist05, lightPosAtDist1, lightColor, studentResultAtDist05, studentResultAtDist1);
            REQUIRE(glm::length(studentResultAtDist05 - studentResultAtDist1) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        SECTION("Multiply by light color")
        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0),
                    .goochB = 0.5f,
                    .goochY = 0.5f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor05 { 0.5f };
            const glm::vec3 lightColor1 { 1.0f };
            const glm::vec3 lightPos = vertexPos + glm::vec3(0, 1.0f, 0);

            const glm::vec3 studentResultLight05 = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor05, 1000);
            const glm::vec3 studentResultLight1 = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor1, 1000);
            CAPTURE(mat, vertexPos, normal, lightPos, lightColor05, lightColor1, studentResultLight05, studentResultLight1);
            REQUIRE(glm::length(studentResultLight05 - 0.5f * studentResultLight1) == Catch::Approx(0).margin(0.01f)); // Large margin to account for fuck-ups in discretization.
        }

        {
            MaterialInformation mat {
                    .Kd = glm::vec3(0.0f),
                    .goochB = 0.0f,
                    .goochY = 1.0f,
                    .goochAlpha = 0.0f,
                    .goochBeta = 0.0f
            };

            const glm::vec3 vertexPos { 0 };
            const glm::vec3 normal { 0, 1, 0 };
            const glm::vec3 lightColor { 1.0f };

            const int n = 2;
            const int numMeasurements = 1000;
            std::vector<float> redChannel;
            for (int i = 0; i < numMeasurements; ++i) {
                const float angleInRadians = (float)i / (numMeasurements - 1) * glm::pi<float>();
                const glm::vec3 lightPos = vertexPos + glm::vec3(glm::sin(angleInRadians), glm::cos(angleInRadians), 0.0f);
                const glm::vec3 studentResult = runStudentGooch(mat, vertexPos, normal, lightPos, lightColor, n);
                redChannel.push_back(studentResult.x);
            }

            std::vector<uint32_t> numMeasurementsPerBin { 1 };
            std::vector<float> subIntervalValuesRedChannel { redChannel[0] };
            for (size_t i = 1; i < numMeasurements; ++i) {
                const float red = redChannel[i];
                if (std::abs(red - subIntervalValuesRedChannel.back()) > 0.001f) { // Allow floating point margin in case students did something weird.
                    numMeasurementsPerBin.push_back(1);
                    subIntervalValuesRedChannel.push_back(red);
                } else {
                    numMeasurementsPerBin.back()++;
                }
            }
            assert(numMeasurementsPerBin.size() == subIntervalValuesRedChannel.size()); // Sanity check for the grader, not the students.

            const int numExpectedSubIntervals = 2 * n + 1;

            SECTION("n=2 should give 5 (2n+1) sub intervals")
            {
                CAPTURE(mat, vertexPos, normal, lightColor, n, subIntervalValuesRedChannel);
                REQUIRE(subIntervalValuesRedChannel.size() == numExpectedSubIntervals);
            }

            SECTION("sub intervals values have value according to i/2n")
            {
                auto paddedSubIntervalValuesRedChannel = subIntervalValuesRedChannel;
                paddedSubIntervalValuesRedChannel.resize(numExpectedSubIntervals, 0);
                for (size_t i = 0; i < numExpectedSubIntervals; ++i) {
                    const float expectedValue = (float)(numExpectedSubIntervals - 1 - i) / (2.0f * n);
                    CAPTURE(mat, vertexPos, normal, lightColor, n, paddedSubIntervalValuesRedChannel, expectedValue);
                    REQUIRE(paddedSubIntervalValuesRedChannel[i] == expectedValue);
                }
            }
        }
    }
}

TEST_CASE("Light Optimization")
{
    RandomVec3Generator vecGen;

    SECTION("Reflection")
    {
        for (size_t i = 0; i < testsPerSection; i++) {
            const glm::vec3 normal = glm::normalize(vecGen.generate());
            const glm::vec3 incomingLightDirection = glm::normalize(vecGen.generate());

            const glm::vec3 studentResult = glm::normalize(computeReflection(normal, incomingLightDirection)); // if the ouput is not normalized it's ok!
            const glm::vec3 referenceResult = reference::referenceComputeReflection(normal, incomingLightDirection);
            CAPTURE(normal, incomingLightDirection, studentResult, referenceResult);

            REQUIRE(glm::length(studentResult - referenceResult) == Catch::Approx(0.0f).margin(errorMargin));
        }
    }

    SECTION("Optimal Normal Vector Mirror")
    {
        for (size_t i = 0; i < testsPerSection; i++) {
            const glm::vec3 mirrorPos = vecGen.generate();
            const glm::vec3 incomingLightDirection = glm::normalize(vecGen.generate());
            const glm::vec3 targetVertexPos = vecGen.generate();

            const glm::vec3 studentMirrorNormal = glm::normalize(optimalMirrorNormal(mirrorPos, incomingLightDirection, targetVertexPos));
            const glm::vec3 reflectedDirection = reference::_reflect(studentMirrorNormal, -incomingLightDirection);
            const glm::vec3 mirrorToTargetDirection = glm::normalize(targetVertexPos - mirrorPos);

            CAPTURE(mirrorPos, incomingLightDirection, targetVertexPos, studentMirrorNormal, reflectedDirection, mirrorToTargetDirection);
            REQUIRE(glm::length(reflectedDirection - mirrorToTargetDirection) == Catch::Approx(0.0f).margin(errorMargin));
        }
    }
}

struct BRDFViewerTestSetup {
    const int n_latitude;
    const int m_longitude;

    const size_t vert_length;
    const size_t tri_length;
    const int quad_length;

    std::vector<glm::vec3> student_vertices;
    std::vector<glm::vec3> reference_vertices;

    glm::vec3 student_position;
    glm::vec3 student_normal;
    std::vector<glm::vec3> student_view_dirs;

    std::vector<glm::vec3> reference_colors;

    std::vector<glm::vec3> student_displaced;
    std::vector<glm::vec3> reference_displaced;

    std::vector<glm::uvec3> student_triangles;
    std::vector<glm::uvec3> student_triangles_remapped;
    std::vector<glm::uvec3> reference_triangles;

    std::vector<glm::uvec4> student_quads;
    std::vector<glm::uvec4> student_quads_remapped;
    std::vector<glm::uvec4> reference_quads;

    std::vector<int> index_mapping;

    struct GradingResults{
        bool wrongVertexLength = false;
        bool wrongLengthViewDirs = false;
        bool wrongVertexLengthDisplaced = false;
        bool indexMappingNotPossible = false;
        bool wrongTriangleLength = false;
        bool wrongQuadLength = false;
        bool triangleIndexOutOfBound = false;
        bool quadIndexOutOfBounds = false;
    } grading_results;

    BRDFViewerTestSetup() = delete;
    BRDFViewerTestSetup(int n_latitude, int m_longitude) :
            n_latitude(n_latitude),
            m_longitude(m_longitude),
            vert_length((n_latitude - 1) * m_longitude + 2),
            tri_length(m_longitude * 2),
            quad_length(m_longitude * (n_latitude - 2)),
            student_vertices(),
            reference_vertices(),
            student_position(-1.0f),
            student_normal(-1.0f),
            student_view_dirs((size_t) vert_length),
            reference_colors((size_t) vert_length),
            student_displaced(),
            reference_displaced((size_t) vert_length),
            student_triangles(),
            student_triangles_remapped((size_t) tri_length, glm::uvec3(0)),
            reference_triangles(),
            student_quads(),
            student_quads_remapped((size_t) quad_length, glm::uvec4(0)),
            reference_quads(),
            index_mapping((size_t) vert_length, 0)
    {}
};

struct GlobalBRDFViewerTestState {
    const glm::vec3 INVALID{123, 456, 789};

    const MaterialInformation testmaterial {
            .Kd = glm::vec3(0.9f, 0.25f, 0.6f),
            .Ks = glm::vec3(1.0f, 1.0f, 1.0f),
            .shininess = 25.0f
    };

    const glm::vec3 ref_diffuse = reference::referenceDiffuseOnly(
            testmaterial,
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.8f, 0.8f, -0.0f },
            { 0.8f, 0.8f, 0.8f });

    std::vector<BRDFViewerTestSetup> setups { { 8, 8 }, { 7, 5 } };
};

void runVertexGeneration (GlobalBRDFViewerTestState &test_data) {
    for (BRDFViewerTestSetup &ts: test_data.setups) {
        ts.student_vertices = generateSphereVertices(ts.n_latitude, ts.m_longitude);
        reference::generateSphereVertices(ts.n_latitude, ts.m_longitude, ts.reference_vertices);

        // Make sure bounds assumptions are correct in following tests
        if (ts.student_vertices.size() != (size_t) ts.vert_length) {
            ts.grading_results.wrongVertexLength = true;
            while (ts.student_vertices.size() > (size_t) ts.vert_length) ts.student_vertices.pop_back();
            while (ts.student_vertices.size() < (size_t) ts.vert_length) ts.student_vertices.push_back(test_data.INVALID);
        }
    }
}


void runVertexDisplacement (GlobalBRDFViewerTestState &test_data) {
    // shading input params
    for (auto& ts : test_data.setups) {
        // check theirs with ours vertices
        const auto &ref_vertices = ts.reference_vertices;
        getReflectedLightInputParameters(ref_vertices, ts.student_position, ts.student_normal, ts.student_view_dirs);

        if (ts.student_view_dirs.size() != (size_t) ts.vert_length) {
            ts.grading_results.wrongLengthViewDirs = true;
            while (ts.student_view_dirs.size() > (size_t) ts.vert_length) ts.student_view_dirs.pop_back();
            while (ts.student_view_dirs.size() < (size_t) ts.vert_length) ts.student_view_dirs.push_back(test_data.INVALID);
        }
    }

    // displacement
    for (auto& ts : test_data.setups) {
        // Get shading dependent on number of view directions
        for (size_t v = 0; v < ts.reference_vertices.size(); v++) {
            glm::vec3 ref_specular = reference::referencePhongSpecularOnly(test_data.testmaterial, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, ts.reference_vertices[v], { 0.8f, 0.8f, -0.0f }, { 0.8f, 0.8f, 0.8f });
            ts.reference_colors[v] = test_data.ref_diffuse + ref_specular;
        }

        const auto &ref_colors = ts.reference_colors;

        // Get student and reference displacement on reference vertices and colors
        ts.student_displaced = ts.reference_vertices;
        ts.reference_displaced = ts.reference_vertices;
        displaceVerticesByIntensity(ref_colors, ts.student_displaced);
        reference::displaceVerticesByIntensity(ref_colors, ts.reference_displaced);

        if (ts.student_displaced.size() != (size_t) ts.vert_length) {
            ts.grading_results.wrongVertexLengthDisplaced = true;
            while (ts.student_displaced.size() > (size_t) ts.vert_length) ts.student_displaced.pop_back();
            while (ts.student_displaced.size() < (size_t) ts.vert_length) ts.student_displaced.push_back(test_data.INVALID);
        }
    }
}

void runIndexMapping(GlobalBRDFViewerTestState &test_data) {
    for (auto& ts : test_data.setups) {
        // find index mapping between student vertices and ours and make sure it is possible
        for (int i = 0; i < ts.vert_length; i++) {
            const auto& v_ref = ts.reference_vertices[i];
            constexpr float margin = 1e-5;
            auto is_within_margin = [v_ref, margin](const glm::vec3& v_stud) { return glm::length(v_stud - v_ref) < margin; };
            auto result_iterator = std::ranges::find_if(ts.student_vertices, is_within_margin);

            if (result_iterator != ts.student_vertices.end()) {  // found vertex
                int index = (int) (result_iterator - ts.student_vertices.begin());
                ts.index_mapping[index] = i;
            }
            else {  // not found
                // maps to our first vertex (top vertex) by default due to 0 initialization. Basically never correct
                ts.grading_results.indexMappingNotPossible = true;
            }

        }
    }
}

void generateTriQuad(GlobalBRDFViewerTestState &test_data) {
    for (BRDFViewerTestSetup& ts : test_data.setups) {
        generateSphereMesh(ts.n_latitude, ts.m_longitude, ts.student_triangles, ts.student_quads);
        reference::generateSphereMesh(ts.n_latitude, ts.m_longitude, ts.reference_triangles, ts.reference_quads);

        constexpr glm::uvec3 INVALID_TRI = {0, 0, 0};
        if (ts.student_triangles.size() != (size_t) ts.tri_length) {
            ts.grading_results.wrongTriangleLength = true;
            while (ts.student_triangles.size() > (size_t) ts.tri_length) ts.student_triangles.pop_back();
            while (ts.student_triangles.size() < (size_t) ts.tri_length) ts.student_triangles.push_back(INVALID_TRI);
        }

        constexpr glm::uvec4 INVALID_QUAD = {0, 0, 0, 0};
        if (ts.student_quads.size() != (size_t) ts.quad_length) {
            ts.grading_results.wrongQuadLength = true;
            while (ts.student_quads.size() > (size_t) ts.quad_length) ts.student_quads.pop_back();
            while (ts.student_quads.size() < (size_t) ts.quad_length) ts.student_quads.push_back(INVALID_QUAD);
        }
    }
}

void remapIndices(GlobalBRDFViewerTestState &test_data) {
    for (BRDFViewerTestSetup& ts : test_data.setups) {
        for (size_t i = 0; i < ts.tri_length; i++) {
            const auto &stud_tri = ts.student_triangles[i];
            auto &mapped_tri = ts.student_triangles_remapped[i];

            if (stud_tri.x >= (unsigned) ts.vert_length) {
                ts.grading_results.triangleIndexOutOfBound = true;
                mapped_tri.x = 0;
            } else mapped_tri.x = (unsigned) ts.index_mapping[stud_tri.x];

            if (stud_tri.y >= (unsigned) ts.vert_length) {
                ts.grading_results.triangleIndexOutOfBound = true;
                mapped_tri.y = 0;
            } else mapped_tri.y = (unsigned) ts.index_mapping[stud_tri.y];

            if (stud_tri.z >= (unsigned) ts.vert_length) {
                ts.grading_results.triangleIndexOutOfBound = true;
                mapped_tri.z = 0;
            } else mapped_tri.z = (unsigned) ts.index_mapping[stud_tri.z];
        }
    }

    // change indices in student quads to indices in our vertex list using mapping
    // make sure to check out of bounds while doing that

    for (BRDFViewerTestSetup& ts : test_data.setups) {
        for (int i = 0; i < ts.quad_length; i++) {
            const auto &stud_quad = ts.student_quads[i];
            auto &mapped_quad = ts.student_quads_remapped[i];

            if (stud_quad.x >= (unsigned) ts.vert_length) {
                ts.grading_results.quadIndexOutOfBounds = true;
                mapped_quad.x = 0;
            } else mapped_quad.x = (unsigned) ts.index_mapping[stud_quad.x];

            if (stud_quad.y >= (unsigned) ts.vert_length) {
                ts.grading_results.quadIndexOutOfBounds = true;
                mapped_quad.y = 0;
            } else mapped_quad.y = (unsigned) ts.index_mapping[stud_quad.y];

            if (stud_quad.z >= (unsigned) ts.vert_length) {
                ts.grading_results.quadIndexOutOfBounds = true;
                mapped_quad.z = 0;
            } else mapped_quad.z = (unsigned) ts.index_mapping[stud_quad.z];
            if (stud_quad.a >= (unsigned) ts.vert_length) {
                ts.grading_results.quadIndexOutOfBounds = true;
                mapped_quad.a = 0;
            } else mapped_quad.a = (unsigned) ts.index_mapping[stud_quad.a];
        }
    }
}

TEST_CASE("Shading Model Viewer") {
    GlobalBRDFViewerTestState test_data{};

    runVertexGeneration(test_data);

    SECTION("Vertex Generation") {
        SECTION("Vector length") {
            for (BRDFViewerTestSetup& ts : test_data.setups) {
                REQUIRE_FALSE(ts.grading_results.wrongVertexLength);
            }
        }

        SECTION("Vertex Positions") {
            for (const auto &ts: test_data.setups) {
                CAPTURE(ts.n_latitude, ts.m_longitude, ts.student_vertices, ts.reference_vertices);
                for (const auto &v_ref: ts.reference_vertices) {
                    constexpr float margin = 1e-5f;
                    auto is_within_margin = [v_ref, margin](const glm::vec3 &v_stud) {
                        return glm::length(v_stud - v_ref) < margin;
                    };
                    bool vertex_found =
                            std::ranges::find_if(ts.student_vertices, is_within_margin) != ts.student_vertices.end();
                    REQUIRE(vertex_found);
                }
            }
        }
    }

    runVertexDisplacement(test_data);

    SECTION("Sphere Vertex Displacement") {
        SECTION("Check vector lengths") {
            for (BRDFViewerTestSetup& ts : test_data.setups) {
                REQUIRE_FALSE(ts.grading_results.wrongLengthViewDirs);
                REQUIRE_FALSE(ts.grading_results.wrongVertexLengthDisplaced);
            }
        }

        SECTION("Parameters for Light Computation") {
            for (const auto& ts : test_data.setups) {
                CAPTURE(ts.student_normal, ts.student_position, ts.student_view_dirs, ts.reference_vertices);
                REQUIRE(glm::vec3(0.0f, 0.0f, 0.0f) == ts.student_position);
                REQUIRE(glm::vec3(0.0f, 1.0f, 0.0f) == ts.student_normal);
                constexpr float margin = 1e-5f;
                for (int i = 0; i < ts.vert_length; i++) {
                    REQUIRE(glm::length(ts.reference_vertices[i] - ts.student_view_dirs[i]) < margin);
                }
            }
        }

        SECTION("Intensity based Displacement of Vertices") {
            for (const auto& ts : test_data.setups) {
                CAPTURE(ts.reference_displaced, ts.student_displaced, ts.reference_colors);
                constexpr float margin = 1e-5;
                for (size_t i = 0; i < ts.vert_length; i++) {
                    REQUIRE(glm::length(ts.student_displaced[i] - ts.reference_displaced[i]) < margin);
                }
            }
        }
    }


    SECTION("Mesh Generation") {

        runIndexMapping(test_data);
        SECTION("Index Mapping Impossible/Index Out of bound: Invalid vertices or indices are contained we replaced them with random ones and continued testing. Triangle/Quad Length: Wrong number of triangles/quads, we filled them with garbage/truncated and continued testing ") {
            for (BRDFViewerTestSetup& ts : test_data.setups) {
                REQUIRE_FALSE(ts.grading_results.indexMappingNotPossible);
                REQUIRE_FALSE(ts.grading_results.wrongTriangleLength);
                REQUIRE_FALSE(ts.grading_results.wrongQuadLength);
                REQUIRE_FALSE(ts.grading_results.triangleIndexOutOfBound);
                REQUIRE_FALSE(ts.grading_results.quadIndexOutOfBounds);
            }
        }

        generateTriQuad(test_data);
        remapIndices(test_data);
        SECTION("Correct triangles") {
            // find all of our triangles in student triangles using order independent equivalence relation for the uvec3s
            for (BRDFViewerTestSetup& ts : test_data.setups) {
                CAPTURE(ts.m_longitude, ts.n_latitude, ts.reference_triangles, ts.student_triangles_remapped, ts.student_triangles);
                for (const auto &tri : ts.reference_triangles) {
                    const std::set<uint32_t> vert_set {tri.x, tri.y, tri.z};
                    auto tri_equivalent = [vert_set] (const glm::uvec3 &tri2) {
                        const std::set<uint32_t> vert_set2 {tri2.x, tri2.y, tri2.z};
                        return vert_set == vert_set2;
                    };
                    auto result_iterator = std::ranges::find_if(ts.student_triangles_remapped, tri_equivalent);
                    bool found = result_iterator != ts.student_triangles_remapped.end();
                    REQUIRE(found);
                }
            }
        }

            // First find all of our quads in student quads using the same order independent equivalence as for triangles
        SECTION("Correct Quads") {
            for (BRDFViewerTestSetup &ts: test_data.setups) {
                CAPTURE(ts.m_longitude, ts.n_latitude, ts.reference_quads, ts.student_quads_remapped, ts.student_quads);
                for (const auto &quad: ts.reference_quads) {
                    const std::set<uint32_t> vert_set{quad.x, quad.y, quad.z, quad.a};
                    auto quad_equivalent = [vert_set](const glm::uvec4 &quad2) {
                        const std::set<uint32_t> vert_set2{quad2.x, quad2.y, quad2.z, quad2.a};
                        return vert_set == vert_set2;
                    };
                    auto result_iterator = std::ranges::find_if(ts.student_quads_remapped, quad_equivalent);
                    bool found = result_iterator != ts.student_quads_remapped.end();
                    REQUIRE(found);
                }
            }
        }
    }
}

