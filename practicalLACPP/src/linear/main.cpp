#include "draw.h"
#include "linear.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <cassert>
#include <filesystem>
#include <framework/trackball.h>
#include <framework/window.h>
#include <fstream>
#include <iostream>
#include <random>

enum class Exercise {
    VECTOR,
    MATRIX,
    ORDER,
    PLANE,
    NGON
};

struct exerciseVectorMulTest {
    glm::vec3 lhs;
    float rhs;
    glm::vec3 result;
};

struct exerciseVectorDotTest {
    glm::vec3 lhs;
    glm::vec3 rhs;
    float result;
};

struct exerciseVectorCrossTest {
    glm::vec3 lhs;
    glm::vec3 rhs;
    glm::vec3 result;
};

struct exerciseVectorLengthTest {
    glm::vec3 lhs;
    float result;
};
struct exerciseMatrixMulTest {
    Matrix3 lhs;
    float rhs;
    Matrix3 result;
};

struct exerciseMatrixTransposeTest {
    Matrix3 lhs;
    Matrix3 result;
};

struct exerciseMatrixDeterminantTest {
    Matrix3 lhs;
    float result;
};

struct exerciseMatrixInverseTest {
    Matrix3 lhs;
    Matrix3 result;
};

struct exerciseTriangleAreaTest {
    std::array<glm::vec3, 3> triangle;
    float result;
};

const float EPSILON = float(0.001);

bool equalWithEpsilon(float lhs, float rhs)
{
    if (std::abs(lhs - rhs) < EPSILON) {
        return true;
    } else {
        return false;
    }
}

bool equalWithEpsilon(glm::vec3 lhs, glm::vec3 rhs) {
    return equalWithEpsilon(lhs.x, rhs.x) && equalWithEpsilon(lhs.y, rhs.y) && equalWithEpsilon(lhs.z, rhs.z);
}

bool equalWithEpsilon(Matrix3 lhs, Matrix3 rhs) {
    return equalWithEpsilon(lhs.col1, rhs.col1) && equalWithEpsilon(lhs.col2, rhs.col2) && equalWithEpsilon(lhs.col3, rhs.col3);
}

int main(int /* argc */, char** argv)
{
    Exercise exercise = Exercise::VECTOR;
    std::string solution = "NA";

    bool drawScrambledNGonVertices = false;
    bool drawSortedNGonVertices = false;

    bool drawCube = false;
    bool drawPrism = false;
    bool drawCubeVertices = false;
    bool drawPrismVertices = false;

    bool drawNGonVertices = false;
    bool drawNGonTriangles = false;

    Trackball::printHelp();

    Window window { argv[0], glm::ivec2(800), OpenGLVersion::GL2 };
    window.registerKeyCallback(
        [&](int key, int /* scancode */, int action, int /* mods */) {
            if (action == GLFW_PRESS) {
                switch (key) {
                case GLFW_KEY_ESCAPE: {
                    window.close();
                    break;
                }
                };
            }
        });
    Trackball trackball { &window, glm::radians(50.0f), 13.0f, 0.785398602f, 3.75943637f };

    // read pre-computed solutions from file to vector
    /*******************************************************************/
    std::ifstream ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "vectors_mul");
    assert(ifs.is_open());
    std::vector<exerciseVectorMulTest> vectorMulTest;
    exerciseVectorMulTest vectorMulTestElements;
    while (!ifs.eof()) {
        ifs >> vectorMulTestElements.lhs.x >> vectorMulTestElements.lhs.y >> vectorMulTestElements.lhs.z >> vectorMulTestElements.rhs >> vectorMulTestElements.result.x >> vectorMulTestElements.result.y >> vectorMulTestElements.result.z;
        vectorMulTest.push_back(vectorMulTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "vectors_dot");
    assert(ifs.is_open());
    std::vector<exerciseVectorDotTest> vectorDotTest;
    exerciseVectorDotTest vectorDotTestElements;
    while (!ifs.eof()) {
        ifs >> vectorDotTestElements.lhs.x >> vectorDotTestElements.lhs.y >> vectorDotTestElements.lhs.z >> vectorDotTestElements.rhs.x >> vectorDotTestElements.rhs.y >> vectorDotTestElements.rhs.z >> vectorDotTestElements.result;
        vectorDotTest.push_back(vectorDotTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "vectors_cross");
    assert(ifs.is_open());
    std::vector<exerciseVectorCrossTest> vectorCrossTest;
    exerciseVectorCrossTest vectorCrossTestElements;
    while (!ifs.eof()) {
        ifs >> vectorCrossTestElements.lhs.x >> vectorCrossTestElements.lhs.y >> vectorCrossTestElements.lhs.z >> vectorCrossTestElements.rhs.x >> vectorCrossTestElements.rhs.y >> vectorCrossTestElements.rhs.z >> vectorCrossTestElements.result.x >> vectorCrossTestElements.result.y >> vectorCrossTestElements.result.z;
        vectorCrossTest.push_back(vectorCrossTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "vectors_length");
    assert(ifs.is_open());
    std::vector<exerciseVectorLengthTest> vectorLengthTest;
    exerciseVectorLengthTest vectorLengthTestElements;
    while (!ifs.eof()) {
        ifs >> vectorLengthTestElements.lhs.x >> vectorLengthTestElements.lhs.y >> vectorLengthTestElements.lhs.z >> vectorLengthTestElements.result;
        vectorLengthTest.push_back(vectorLengthTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "matrix_mul");
    assert(ifs.is_open());
    std::vector<exerciseMatrixMulTest> matrixMulTest;
    exerciseMatrixMulTest matrixMulTestElements;
    while (!ifs.eof()) {
        ifs >> matrixMulTestElements.lhs.col1.x >> matrixMulTestElements.lhs.col1.y >> matrixMulTestElements.lhs.col1.z >> matrixMulTestElements.lhs.col2.x >> matrixMulTestElements.lhs.col2.y >> matrixMulTestElements.lhs.col2.z >> matrixMulTestElements.lhs.col3.x >> matrixMulTestElements.lhs.col3.y >> matrixMulTestElements.lhs.col3.z >> matrixMulTestElements.rhs >> matrixMulTestElements.result.col1.x >> matrixMulTestElements.result.col1.y >> matrixMulTestElements.result.col1.z >> matrixMulTestElements.result.col2.x >> matrixMulTestElements.result.col2.y >> matrixMulTestElements.result.col2.z >> matrixMulTestElements.result.col3.x >> matrixMulTestElements.result.col3.y >> matrixMulTestElements.result.col3.z;
        matrixMulTest.push_back(matrixMulTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "matrix_transpose");
    assert(ifs.is_open());
    std::vector<exerciseMatrixTransposeTest> matrixTransposeTest;
    exerciseMatrixTransposeTest matrixTransposeTestElements;
    while (!ifs.eof()) {
        ifs >> matrixTransposeTestElements.lhs.col1.x >> matrixTransposeTestElements.lhs.col1.y >> matrixTransposeTestElements.lhs.col1.z >> matrixTransposeTestElements.lhs.col2.x >> matrixTransposeTestElements.lhs.col2.y >> matrixTransposeTestElements.lhs.col2.z >> matrixTransposeTestElements.lhs.col3.x >> matrixTransposeTestElements.lhs.col3.y >> matrixTransposeTestElements.lhs.col3.z >> matrixTransposeTestElements.result.col1.x >> matrixTransposeTestElements.result.col1.y >> matrixTransposeTestElements.result.col1.z >> matrixTransposeTestElements.result.col2.x >> matrixTransposeTestElements.result.col2.y >> matrixTransposeTestElements.result.col2.z >> matrixTransposeTestElements.result.col3.x >> matrixTransposeTestElements.result.col3.y >> matrixTransposeTestElements.result.col3.z;
        matrixTransposeTest.push_back(matrixTransposeTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "matrix_determinant");
    assert(ifs.is_open());
    std::vector<exerciseMatrixDeterminantTest> matrixDeterminantTest;
    exerciseMatrixDeterminantTest matrixDeterminantTestElements;
    while (!ifs.eof()) {
        ifs >> matrixDeterminantTestElements.lhs.col1.x >> matrixDeterminantTestElements.lhs.col1.y >> matrixDeterminantTestElements.lhs.col1.z >> matrixDeterminantTestElements.lhs.col2.x >> matrixDeterminantTestElements.lhs.col2.y >> matrixDeterminantTestElements.lhs.col2.z >> matrixDeterminantTestElements.lhs.col3.x >> matrixDeterminantTestElements.lhs.col3.y >> matrixDeterminantTestElements.lhs.col3.z >> matrixDeterminantTestElements.result;
        matrixDeterminantTest.push_back(matrixDeterminantTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "matrix_inverse");
    assert(ifs.is_open());
    std::vector<exerciseMatrixInverseTest> matrixInverseTest;
    exerciseMatrixInverseTest matrixInverseTestElements;
    while (!ifs.eof()) {
        ifs >> matrixInverseTestElements.lhs.col1.x >> matrixInverseTestElements.lhs.col1.y >> matrixInverseTestElements.lhs.col1.z >> matrixInverseTestElements.lhs.col2.x >> matrixInverseTestElements.lhs.col2.y >> matrixInverseTestElements.lhs.col2.z >> matrixInverseTestElements.lhs.col3.x >> matrixInverseTestElements.lhs.col3.y >> matrixInverseTestElements.lhs.col3.z >> matrixInverseTestElements.result.col1.x >> matrixInverseTestElements.result.col1.y >> matrixInverseTestElements.result.col1.z >> matrixInverseTestElements.result.col2.x >> matrixInverseTestElements.result.col2.y >> matrixInverseTestElements.result.col2.z >> matrixInverseTestElements.result.col3.x >> matrixInverseTestElements.result.col3.y >> matrixInverseTestElements.result.col3.z;
        matrixInverseTest.push_back(matrixInverseTestElements);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "cube");
    assert(ifs.is_open());
    std::vector<Plane> planeTestCube;
    Plane exsPlane;
    while (!ifs.eof()) {
        ifs >> exsPlane.p.x >> exsPlane.p.y >> exsPlane.p.z >> exsPlane.n.x >> exsPlane.n.y >> exsPlane.n.z;
        planeTestCube.push_back(exsPlane);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "prism");
    assert(ifs.is_open());
    std::vector<Plane> planeTestPrism;
    while (!ifs.eof()) {
        ifs >> exsPlane.p.x >> exsPlane.p.y >> exsPlane.p.z >> exsPlane.n.x >> exsPlane.n.y >> exsPlane.n.z;
        planeTestPrism.push_back(exsPlane);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "prism_vertices");
    assert(ifs.is_open());
    std::vector<glm::vec3> prismVertices;
    glm::vec3 prismVertex;
    while (!ifs.eof()) {
        ifs >> prismVertex.x >> prismVertex.y >> prismVertex.z;
        prismVertices.push_back(prismVertex);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "triangleArea");
    assert(ifs.is_open());
    std::vector<exerciseTriangleAreaTest> triangleAreaTest;
    exerciseTriangleAreaTest triangleAreaTestElement;
    glm::vec3 vertex1, vertex2, vertex3;
    float area;
    while (!ifs.eof()) {
        ifs >> vertex1.x >> vertex1.y >> vertex1.z >> vertex2.x >> vertex2.y >> vertex2.z >> vertex3.x >> vertex3.y >> vertex3.z >> area;
        triangleAreaTestElement.triangle = { vertex1, vertex2, vertex3 };
        triangleAreaTestElement.result = area;
        triangleAreaTest.push_back(triangleAreaTestElement);
    }
    ifs.close();

    ifs = std::ifstream(std::filesystem::path(DATA_DIR) / "nGon");
    assert(ifs.is_open());
    std::vector<glm::vec3> nGonTest;
    glm::vec3 vertex;
    while (!ifs.eof()) {
        ifs >> vertex.x >> vertex.y >> vertex.z;
        nGonTest.push_back(vertex);
    }
    ifs.close();

    std::vector<int> randomOrder;
    for (int i = 0; i < nGonTest.size(); i++)
        randomOrder.push_back(i);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(randomOrder.begin(), randomOrder.end(), g);
    
    std::vector<std::tuple<glm::vec3, int>> nGonScrambledTest;
    for (int index : randomOrder) {
        nGonScrambledTest.push_back(std::make_tuple(nGonTest[index], index));
    }

    /*******************************************************************/

    while (!window.shouldClose()) {
        window.updateInput();

        // Initialize viewpoint.
        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        // Clear screen.
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth test.
        glEnable(GL_DEPTH_TEST);
        // Load model/view matrix.
        glMatrixMode(GL_MODELVIEW);
        const glm::mat4 viewTransform = trackball.viewMatrix();
        glLoadMatrixf(glm::value_ptr(viewTransform));

        // Load projection matrix.
        glMatrixMode(GL_PROJECTION);
        const glm::mat4 perspectiveMatrix = trackball.projectionMatrix();
        glLoadMatrixf(glm::value_ptr(perspectiveMatrix));

        drawCoordSystem();

        ImGui::Begin("Half Plane Solid");
        {
            std::array items { "0. Vectors", "0. Matrices", "1. Order", "2. Planes", "3. nGon Area" };
            ImGui::ListBox("Exercises\n", reinterpret_cast<int*>(&exercise), items.data(), int(items.size()), 6);
        }

        // Draw GUI Elements and define their behavior
        bool update = false;

        // Check the exercises of the Vector programming tasks
        if (exercise == Exercise::VECTOR) {
            // Button to check the multiplication of vector and scalar
            if (ImGui::Button("Check mul(vertex, scalar)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : vectorMulTest) {
                    glm::vec3 studentResult = mul(testCase.lhs, testCase.rhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Multiplication of " + to_string(testCase.lhs) + "\n with " + std::to_string(testCase.rhs) + "\n resulted in " + to_string(studentResult) + "\n instead of " + to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }
            // Button to check the dot product
            if (ImGui::Button("Check dot(vertex, vertex)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : vectorDotTest) {
                    float studentResult = dot3(testCase.lhs, testCase.rhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Dot product of " + to_string(testCase.lhs) + "\n and " + to_string(testCase.rhs) + "\n resulted in " + std::to_string(studentResult) + "\n instead of " + std::to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }
            // Button to check the cross product
            if (ImGui::Button("Check cross(vertex, vertex)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : vectorCrossTest) {
                    glm::vec3 studentResult = cross3(testCase.lhs, testCase.rhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Cross product of " + to_string(testCase.lhs) + "\n and " + to_string(testCase.rhs) + "\n resulted in " + to_string(studentResult) + "\n instead of " + to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }
            // Button to check the cross product
            if (ImGui::Button("Check length(vertex)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : vectorLengthTest) {
                    float studentResult = length(testCase.lhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Length of " + to_string(testCase.lhs) + "\n resulted in " + std::to_string(studentResult) + "\n instead of " + std::to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }

        // Check the exercises of the Matrix programming task
        } else if (exercise == Exercise::MATRIX) {
            // Button to check matrix multiplication with a scalar
            if (ImGui::Button("Check mul(matrix, scalar)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : matrixMulTest) {
                    Matrix3 studentResult = mul(testCase.lhs, testCase.rhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Product of " + to_string(testCase.lhs) + "\n and " + std::to_string(testCase.rhs) + "\n resulted in " + to_string(studentResult) + "\n instead of " + to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }

            // Button to check matrix transpose
            if (ImGui::Button("Check transpose(matrix)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : matrixTransposeTest) {
                    Matrix3 studentResult = transpose(testCase.lhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Transpose of " + to_string(testCase.lhs) + "\n resulted in " + to_string(studentResult) + "\n instead of " + to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }
            // Button to check matrix determinant
            if (ImGui::Button("Check determinant(matrix)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : matrixDeterminantTest) {
                    float studentResult = determinant(testCase.lhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Determinant of " + to_string(testCase.lhs) + "\n resulted in " + std::to_string(studentResult) + "\n instead of " + std::to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }

            // Button to check matrix inverse
            if (ImGui::Button("Check inverse(matrix)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : matrixInverseTest) {
                    Matrix3 studentResult = inverse(testCase.lhs);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Inverse of " + to_string(testCase.lhs) + "\n resulted in " + to_string(studentResult) + "\n instead of " + to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }

        } else if (exercise == Exercise::ORDER) {

            solution = "N/A";

            // Button to check matrix inverse
            if (ImGui::Button("Change vertex permutation")) {
                std::shuffle(randomOrder.begin(), randomOrder.end(), g);
                nGonScrambledTest.clear();
                for (int index : randomOrder) {
                    nGonScrambledTest.push_back(std::make_tuple(nGonTest[index], index));
                }
                update = true;
            }

            // Draw the vertices of the (scrambled) test case
            ImGui::Checkbox("Draw scrambled nGon Vertices", &drawScrambledNGonVertices);
            if (drawScrambledNGonVertices) {
                for (int i = 0; i < int(nGonScrambledTest.size()); i++) {
                    float color = float(i) / ( float(nGonScrambledTest.size()) - 1.f );
                    drawSphere(get<0>(nGonScrambledTest[i]), float(0.1), 1.f - color, 1.f - color, color);
                }
                update = true;
            }

            // Draw the vertices of the (sorted) test case
            ImGui::Checkbox("Draw sorted nGon Vertices", &drawSortedNGonVertices);
            if (drawSortedNGonVertices) {
                std::vector<glm::vec3> scrambledNGonVertices;
                for (const std::tuple<glm::vec3, int> pointAndIndex : nGonScrambledTest) {
                    scrambledNGonVertices.push_back(get<0>(pointAndIndex));
                }
                std::vector<int> vertexOrder = orderOfnGonVertices(scrambledNGonVertices);
                for (int i = 0; i < int(vertexOrder.size()); i++) {
                    float color = float(i) / (float(nGonScrambledTest.size()) - 1.f);
                    drawSphere(scrambledNGonVertices[vertexOrder[i]], float(0.1), 1.f - color, 1.f - color, color);
                }
                update = true;
            }

        } else if (exercise == Exercise::PLANE) {
            
            ImGui::Checkbox("Draw Cube Planes", &drawCube);
            ImGui::Checkbox("Draw Cube Vertices", &drawCubeVertices);
            ImGui::Checkbox("Draw Hexagonal Prism Planes", &drawPrism);
            ImGui::Checkbox("Draw Hexagonal Prism Vertices", &drawPrismVertices);
            if (drawCube) {
                for (auto& plane : planeTestCube) {
                    const auto points = rectangleOnPlane(plane);
                    for (const auto& point : points) {
                        if (!equalWithEpsilon(glm::dot(point, plane.n), glm::dot(plane.p, plane.n))) {
                            solution = "\n Err: Point " + to_string(point) + " does not lie on the plane with \n normal vector " + to_string(plane.n) + "\n and base point " + to_string(plane.p);
                            break;
                        }
                    }
                    drawRectangle(points, plane);
                }
                update = true;
            }
            if (drawPrism) {
                for (const auto& plane : planeTestPrism) {
                    const auto points = rectangleOnPlane(plane);
                    for (const auto& point : points) {
                        if (!equalWithEpsilon(glm::dot(point, plane.n), glm::dot(plane.p, plane.n))) {
                            solution = "\n Err: Point " + to_string(point) + " does not lie on the plane with \n normal vector " + to_string(plane.n) + "\n and base point " + to_string(plane.p);
                            break;
                        }
                    }
                    drawRectangle(points, plane);
                }
                update = true;
            }
            if (drawCubeVertices) {
                std::vector<glm::vec3> points = verticesFromPlanes(planeTestCube);
                if (points.size() != 8) {
                    solution = "\n Err: Cube should have 8 vertices, but got " + std::to_string(points.size());
                }
                for (const auto& point : points) {
                    if (!(equalWithEpsilon(std::abs(point.x), 1.0) && equalWithEpsilon(std::abs(point.y), 1.0) && equalWithEpsilon(std::abs(point.z), 1.0))) 
                    {
                        solution = "\n Err: Point " + to_string(point) + " is not a point of the cube.";
                    }
                    drawSphere(point, float(0.1));
                }
                update = true;
            }
            if (drawPrismVertices) {
                std::vector<glm::vec3> points = verticesFromPlanes(planeTestPrism);
                if (points.size() != 12) {
                    solution = "\n Err: Prism should have 12 vertices, but got " + std::to_string(points.size());
                }
                for (const glm::vec3 point : points) {
                    bool pointCorrect = false;
                    for (const glm::vec3 correctPoint : prismVertices) {
                        if (!(equalWithEpsilon(std::abs(point.x), correctPoint.x) && equalWithEpsilon(std::abs(point.y), correctPoint.y) && equalWithEpsilon(std::abs(point.z), correctPoint.z))) {
                            pointCorrect = true;
                            break;
                        }
                    }
                    if (!pointCorrect) {
                        solution = "\n Err: Point " + to_string(point) + " is not a point of the prism.";
                    }
                    drawSphere(point, float(0.1));
                }
                update = true;
            }
        } else if (exercise == Exercise::NGON) {

            // Button to check area method separately
            if (ImGui::Button("Check area(triangle)")) {
                solution = "Passed example inputs";
                for (const auto& testCase : triangleAreaTest) {
                    float studentResult = areaOfTriangle(testCase.triangle);
                    if (!equalWithEpsilon(studentResult, testCase.result)) {
                        solution = "\n Err: Area of triangle A: " + to_string(testCase.triangle[0]) + " B: " + to_string(testCase.triangle[1]) + " C: " + to_string(testCase.triangle[2]) + "\n resulted in " + std::to_string(studentResult) + "\n instead of " + std::to_string(testCase.result);
                        break;
                    }
                }
                update = true;
            }

            ImGui::Checkbox("Draw nGon Vertices", &drawNGonVertices);
            if (drawNGonVertices) {
                // Draw the vertices of the test case
                for (const auto& point : nGonTest) {
                    drawSphere(point, float(0.1));
                }
                update = true;
            }

            ImGui::Checkbox("Draw nGon Triangles", &drawNGonTriangles);
            if (drawNGonTriangles) {
                // Visualize the triangles
                std::vector<std::array<glm::vec3, 3>> triangles = splitNGonIntoTriangles(nGonTest);
                for (std::array<glm::vec3, 3> triangle : triangles) {
                    drawTriangle(triangle);
                }
                update = true;
            }
        }

        ImGui::Text("Solution: %s",  solution.c_str());

        ImGui::End();

        window.swapBuffers();
    }
}
