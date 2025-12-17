#include "your_code_here.h"
// Include your_code_here first so that students have to include what they use.
#include "draw.h"
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <framework/glm_ostream.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vector_relational.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <chrono>
#include <framework/mesh.h>
#include <framework/trackball.h>
#include <framework/window.h>
#include <iostream>
#include <random>
#include <span>
#include <string>
#include <thread>

enum class DiffuseMode {
    None,
    Lambert,
    Gooch
};

enum class SpecularMode {
    None,
    Phong,
    BlinnPhong,
    Difference
};

// Different display modes.
struct DisplayModes {
    bool debug = false;
    DiffuseMode diffuse = DiffuseMode::Lambert;
    SpecularMode specular = SpecularMode::None;
};

enum class LightPlacement {
    Sphere,
    Shadow,
    Specular
};

// For Shading Model Viewer
enum class SphereMode {
    SphereNone,
    SpherePoints,
    SphereVectors,
    SphereMesh
};

struct ProgramState {
    struct Line {
        glm::vec3 p0, p1;
        glm::vec3 color;
    };
    struct Quad {
        glm::vec3 p0, p1, p2, p3;
    };

    DisplayModes displayModes;
    glm::vec3 backgroundColor { 1.0f, 1.0f, 1.0f };

    Light light;

    Mesh myMesh;
    MaterialInformation materialInformation;

    unsigned drawLines = false;
    std::vector<Line> lines;
    std::vector<Quad> quads;

    int goochN = 1;

    // Shading Model Viewer
    struct ShadingModelSphere {
        std::uint32_t resolution_lat = 50;
        std::uint32_t resolution_long = 50;
        SphereMode mode = SphereMode::SpherePoints;
    };
    ShadingModelSphere shadingModelSphere {};
};

// --- Init global variables
glm::vec3 dayLight(1.0f);
glm::vec3 eveningLight(0.9f, 0.7f, 0.7f);
glm::vec3 poleTipPosition(0, 0.11, 0);
glm::vec3 reflectorPosition(1, -0.91, 0);
glm::vec3 reflectorNormal = glm::normalize(glm::vec3(-.5, 1, -.5));
MaterialInformation DayCarMaterial {
    .Kd = glm::vec3(0.9f, 0.25f, 0.6f),
    .Ks = glm::vec3(1.0f, 1.0f, 1.0f),
    .shininess = 25.0f
};
MaterialInformation SolarPlant {
    .Kd = glm::vec3(50, 50, 50) / 255.0f,
    .Ks = glm::vec3(255, 255, 255) / 255.0f,
    .shininess = 200.0f
};
int section = 0;

glm::vec3 computeLighting(const ProgramState& programState, const Vertex& vertex, const glm::vec3& cameraPos, const Light& light)
{
    //    const auto& vertex = programState.myMesh.vertices[vertexIndex];

    // do not change any global variables here! This function will be called for EACH vertex
    // of the mesh, so your change would happen several times
    if (programState.displayModes.debug) {
        return debugColor(programState.materialInformation, vertex.position, vertex.normal, cameraPos, light.position, light.color);
    }

    glm::vec3 result { 0.0f };
    switch (programState.displayModes.diffuse) {
    case DiffuseMode::Lambert: {
        result += diffuseOnly(programState.materialInformation, vertex.position, vertex.normal, light.position, light.color);
    } break;
    case DiffuseMode::Gooch: {
        result += gooch(programState.materialInformation, vertex.position, vertex.normal, light.position, light.color, programState.goochN);
    } break;
    case DiffuseMode::None:
        break;
    };

    switch (programState.displayModes.specular) {
    case SpecularMode::Phong: {
        result += phongSpecularOnly(programState.materialInformation, vertex.position, vertex.normal, cameraPos, light.position, light.color);
    } break;
    case SpecularMode::BlinnPhong: {
        result += blinnPhongSpecularOnly(programState.materialInformation, vertex.position, vertex.normal, cameraPos, light.position, light.color);
    } break;
    case SpecularMode::Difference: {
        const auto phong = phongSpecularOnly(programState.materialInformation, vertex.position, vertex.normal, cameraPos, light.position, light.color);
        const auto blinnPhong = blinnPhongSpecularOnly(programState.materialInformation, vertex.position, vertex.normal, cameraPos, light.position, light.color);
        result += diffPhongSpecularOnly(phong, blinnPhong);
    } break;
    case SpecularMode::None:
        break;
    };

    return result;
}

void keyboard(unsigned char key, ProgramState& state, Window& window, const Trackball& camera)
{
    switch (key) {
    case 'a': {
        if (section == 1) {
            reflectorNormal = glm::normalize(reflectorNormal + glm::vec3(-0.01f, 0.0f, 0.0f));
        }
    } break;
    case 'd': {
        if (section == 1) {
            reflectorNormal = glm::normalize(reflectorNormal + glm::vec3(0.01f, 0.0f, 0.0f));
        }
    } break;
    case 's': {
        if (section == 1) {
            reflectorNormal = glm::normalize(reflectorNormal + glm::vec3(0.0f, 0.01, 0.0f));
        }
    } break;
    case 'w': {
        if (section == 1) {
            reflectorNormal = glm::normalize(reflectorNormal + glm::vec3(0.0f, -0.01, 0.0f));
        }
    } break;
    case 'q': {
        window.close();
    } break;
    };
}

void draw(const ProgramState& state, const Trackball& camera, std::span<const glm::vec3> vertexColors)
{
    glClearColor(state.backgroundColor.r, state.backgroundColor.g, state.backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Initialize projection and view matrices.
    glMatrixMode(GL_PROJECTION);
    const auto projectionMatrix = camera.projectionMatrix();
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    glMatrixMode(GL_MODELVIEW);
    const auto viewMatrix = camera.viewMatrix();
    glLoadMatrixf(glm::value_ptr(viewMatrix));

    // Drawing mode options.
    glEnable(GL_DEPTH_TEST); // Enable depth test.
    glDepthMask(GL_TRUE); // Enable depth write.
    glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    drawMeshWithColors(state.myMesh, vertexColors);

    // Disable depth write because we don't want the points in our depth buffer (messes with user interaction).
    glDepthMask(GL_FALSE); // Disable depth write.

    // Draw lights as points (squares) in the lights color.
    glPointSize(50);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex3fv(glm::value_ptr(state.light.position));
    glEnd();

    glPointSize(40);
    glBegin(GL_POINTS);
    glColor3fv(glm::value_ptr(state.light.color));
    glVertex3fv(glm::value_ptr(state.light.position));
    glEnd();

    if (state.drawLines) {
        for (size_t i = 0; i < state.lines.size(); i++) {
            const auto& line = state.lines[i];
            glBegin(GL_LINES);
            glColor3f(line.color.x, line.color.y, line.color.z);
            glVertex3f(line.p0.x, line.p0.y, line.p0.z);
            glVertex3f(line.p1.x, line.p1.y, line.p1.z);
            glEnd();
        }
    }

    if (!state.quads.empty()) {
        for (const auto& quad : state.quads) {
            glBegin(GL_QUADS);
            glColor3f(0.1f, 0.1f, 0.1f);
            glVertex3fv(glm::value_ptr(quad.p0));
            glVertex3fv(glm::value_ptr(quad.p1));
            glVertex3fv(glm::value_ptr(quad.p2));
            glVertex3fv(glm::value_ptr(quad.p3));
            glEnd();
        }
    }

    glDepthMask(GL_TRUE); // Disable depth write.
}

void computeLighting(const ProgramState& state, const glm::vec3& cameraPos, std::span<glm::vec3> outVertexColors)
{
    for (size_t v = 0; v < state.myMesh.vertices.size(); v++) {
        const Vertex& vertex = state.myMesh.vertices[v];
        outVertexColors[v] = computeLighting(state, vertex, cameraPos, state.light);
    }
}

void printHelp()
{
    std::cout << "Program Usage:" << std::endl;
    std::cout << "PRESS q to exit" << std::endl;
    // std::cout << "SPACE - replaces mouse click for selection, will then call your light placement function" << std::endl;
}

static void setBarbieCar(ProgramState& state, Trackball& camera)
{
    state.light = Light { glm::vec3(0.8f, 0.8f, -0.0f), glm::vec3(0.8f) };
    state.drawLines = false;
    state.lines.clear();

    camera.setCamera(glm::vec3(0.0f), glm::vec3(0.4, -2.4, 0), 2.0);
    state.myMesh = loadMesh(std::filesystem::path(DATA_DIR) / "Lambo.obj", { .normalizeVertexPositions = true })[0];
    meshFlipZ(state.myMesh);
    state.materialInformation = DayCarMaterial;
}

void drawUI(ProgramState& state, Trackball& camera, std::span<glm::vec3> vertexColors)
{
    constexpr std::array diffuseLabels {
        "None",
        "Lambert",
        "Gooch"
    };
    constexpr std::array specularLabels {
        "None",
        "Phong",
        "Blinn-Phong",
        "Difference"
    };
    constexpr std::array lightPlacementLabels {
        "Sphere",
        "Shadow",
        "Specular"
    };

    // For Shading Model Viewer
    constexpr std::array sphereModeLabels {
        "No Sphere",
        "Points",
        "Vectors",
        "Mesh"
    };

    ImGui::Begin("Options");
    if (ImGui::Button("Barbie's car")) {
        section = 0;
        state.backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
        setBarbieCar(state, camera);
    }
    //ImGui::SameLine();
    if (ImGui::Button("The thermosolar powerplant")) {
        state.light = Light { glm::vec3(0.1f, 0.0f, -1.0f), glm::vec3(1.0f) };
        camera.setCamera(glm::vec3(0.0f), glm::vec3(0.965487f, 0.202287f, 0.000000f), 2.0f);

        state.displayModes.diffuse = DiffuseMode::None;
        state.displayModes.specular = SpecularMode::Phong;
        state.myMesh = loadMesh(std::filesystem::path(DATA_DIR) / "ThermoSolarPowerPlant.obj", { .normalizeVertexPositions = true })[0];
        meshFlipZ(state.myMesh);

        state.backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
        state.materialInformation = SolarPlant;
        section = 1;
    }
    //ImGui::SameLine();
    if (ImGui::Button("Shading Model Viewer")) {
        state.light = Light { glm::vec3(0.8f, 0.8f, -0.0f), glm::vec3(0.8f) };
        camera.setCamera(glm::vec3(0.0f), glm::vec3(0.4, -2.4, 0), 3.5);

        state.drawLines = false;
        state.lines.clear();

        state.myMesh = { {}, {}, {} };

        state.backgroundColor = glm::vec3(0.0f);
        state.materialInformation = DayCarMaterial;
        state.displayModes.debug = false;

        section = 2;
    }

    if (section == 0) // barbie's car
    {
        ImGui::Text("Shading");
        ImGui::Spacing();
        ImGui::Checkbox("Debug shading", &state.displayModes.debug);
        if (!state.displayModes.debug) {
            // https://github.com/ocornut/imgui/issues/1658
            if (ImGui::BeginCombo("Diffuse shading", diffuseLabels[size_t(state.displayModes.diffuse)])) {
                for (size_t i = 0; i < diffuseLabels.size(); i++) {
                    bool isSelected = (i == size_t(state.displayModes.diffuse));
                    if (ImGui::Selectable(diffuseLabels[i], isSelected)) {
                        state.displayModes.diffuse = DiffuseMode(i);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Specular shading", specularLabels[size_t(state.displayModes.specular)])) {
                for (size_t i = 0; i < specularLabels.size(); i++) {
                    const bool isSelected = (i == size_t(state.displayModes.specular));
                    if (ImGui::Selectable(specularLabels[i], isSelected)) {
                        state.displayModes.specular = SpecularMode(i);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Material");
        ImGui::Spacing();
        ImGui::ColorEdit3("Diffuse color (Kd)", glm::value_ptr(state.materialInformation.Kd));
        ImGui::ColorEdit3("Specular color (Ks)", glm::value_ptr(state.materialInformation.Ks));
        ImGui::SliderFloat("Shininess", &state.materialInformation.shininess, 1.0f, 50.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
        if (state.displayModes.diffuse == DiffuseMode::Gooch)
            ImGui::SliderInt("Gooch n", &state.goochN, 1, 10);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Test the vertex #33");
        ImGui::Spacing();

        if (ImGui::Button("Day")) {
            // Do not modify these values
            state.materialInformation.Kd = DayCarMaterial.Kd;
            state.materialInformation.Ks = DayCarMaterial.Ks;
            state.materialInformation.shininess = DayCarMaterial.shininess;
            state.light = Light { glm::vec3(0.8f, 0.8f, -0.0f), dayLight };
            camera.setCamera(glm::vec3(0.0f), glm::vec3(0.5f, -0.7f, 0.0f), 1.7f);
            state.backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Evening")) {
            state.light = Light { glm::vec3(0.8f, 0.8f, -0.0f), eveningLight };
            camera.setCamera(glm::vec3(0.0f), glm::vec3(0.5f, -0.7f, 0.0f), 1.7f);
            state.materialInformation = getMaterialEveningCar(dayLight, eveningLight, DayCarMaterial);
            state.backgroundColor = eveningLight;
        }
        if (vertexColors.size() >= 33) {
            ImGui::TextUnformatted((std::string("Color: RGB = ") + glm::to_string(glm::vec<3, glm::uint8>(vertexColors[33] * glm::vec3(255)))).c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::ColorEdit3("Light color", glm::value_ptr(state.light.color));

        state.quads.clear();
    } else if (section == 1) { // thermosolar power plant
        ImGui::Text("Shading");
        ImGui::Spacing();

        state.displayModes.diffuse = DiffuseMode::Lambert;
        ImGui::Text("Diffuse Shading: None");
        ImGui::Text("Specular Shading: Phong");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Material");
        ImGui::Spacing();
        ImGui::ColorEdit3("Diffuse color (Kd)", glm::value_ptr(state.materialInformation.Kd), ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit3("Specular color (Ks)", glm::value_ptr(state.materialInformation.Ks), ImGuiColorEditFlags_NoInputs);
        ImGui::TextUnformatted(("Shininess: " + std::to_string(state.materialInformation.shininess)).c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("change lightPos to camPos")) {
            state.light = Light { camera.position(), glm::vec3(1.0f) };
        }

        glm::vec3 reflectedRay = computeReflection(reflectorNormal, glm::normalize(reflectorPosition - state.light.position));

        state.drawLines = true;
        state.lines.clear();
        state.lines.push_back({ .p0 = reflectorPosition,
            .p1 = glm::length(reflectorPosition - poleTipPosition) * reflectedRay + reflectorPosition,
            .color = glm::vec3(0.9, 0.6, 0) });
        state.lines.push_back({ .p0 = reflectorPosition,
            .p1 = state.light.position,
            .color = glm::vec3(0.9, 0.7, 0) });
        state.lines.push_back({ .p0 = reflectorPosition,
            .p1 = reflectorNormal + reflectorPosition,
            .color = glm::vec3(0, 0, 1) });

        glm::vec3 orth = reflectorNormal; // glm::vec3(1, 0, 0);
        if (std::abs(reflectorNormal.x) >= 0.2f) {
            orth.x = reflectorNormal.y;
            orth.y = -reflectorNormal.x;
            orth.z = 0;
        } else {
            orth.x = 0; // reflectorNormal.y;
            orth.y = -reflectorNormal.z; //-reflectorNormal.x;
            orth.z = reflectorNormal.y; // 0;
        }
        glm::vec3 u = glm::cross(orth, reflectorNormal);
        glm::vec3 v = glm::cross(u, reflectorNormal);

        state.quads.clear();
        state.quads.push_back({
            .p0 = reflectorPosition + 0.1f * (+u - 0.5f * u - 0.5f * v),
            .p1 = reflectorPosition + 0.1f * (+u + v - 0.5f * u - 0.5f * v),
            .p2 = reflectorPosition + 0.1f * (+v - 0.5f * u - 0.5f * v),
            .p3 = reflectorPosition + 0.1f * (-0.5f * u - 0.5f * v),
        });

        if (ImGui::Button("Optimal mirror normal")) {
            state.drawLines = false;
            state.lines.clear();

            reflectorNormal = optimalMirrorNormal(reflectorPosition, glm::normalize(reflectorPosition - state.light.position), poleTipPosition);
        }
    } else if (section == 2) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Visualization Mode");
        if (ImGui::BeginCombo("Mode", sphereModeLabels[size_t(state.shadingModelSphere.mode)])) {
            for (size_t i = 0; i < sphereModeLabels.size(); i++) {
                bool isSelected = (i == size_t(state.shadingModelSphere.mode));
                if (ImGui::Selectable(sphereModeLabels[i], isSelected)) {
                    state.shadingModelSphere.mode = SphereMode(i);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
        ImGui::SliderInt("Resolution Longitude", (int*)&state.shadingModelSphere.resolution_long, 5, 500, "%.0f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Resolution Latitude", (int*)&state.shadingModelSphere.resolution_lat, 5, 500, "%.0f", ImGuiSliderFlags_Logarithmic);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Shading");
        ImGui::Spacing();
        // https://github.com/ocornut/imgui/issues/1658
        if (ImGui::BeginCombo("Diffuse shading", diffuseLabels[size_t(state.displayModes.diffuse)])) {
            for (size_t i = 0; i < diffuseLabels.size(); i++) {
                bool isSelected = (i == size_t(state.displayModes.diffuse));
                if (ImGui::Selectable(diffuseLabels[i], isSelected)) {
                    state.displayModes.diffuse = DiffuseMode(i);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("Specular shading", specularLabels[size_t(state.displayModes.specular)])) {
            for (size_t i = 0; i < specularLabels.size(); i++) {
                const bool isSelected = (i == size_t(state.displayModes.specular));
                if (ImGui::Selectable(specularLabels[i], isSelected)) {
                    state.displayModes.specular = SpecularMode(i);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Material");
        ImGui::Spacing();
        ImGui::ColorEdit3("Diffuse color (Kd)", glm::value_ptr(state.materialInformation.Kd));
        ImGui::ColorEdit3("Specular color (Ks)", glm::value_ptr(state.materialInformation.Ks));
        ImGui::SliderFloat("Shininess", &state.materialInformation.shininess, 1.0f, 50.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
        if (state.displayModes.diffuse == DiffuseMode::Gooch)
            ImGui::SliderInt("Gooch n", &state.goochN, 1, 10);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Light");
        ImGui::Spacing();
        ImGui::ColorEdit3("Light color", glm::value_ptr(state.light.color));
        ImGui::DragFloat3("Light Position", glm::value_ptr(state.light.position), 0.05f, -2.0f, +2.0f, "%0.2f", 1.0f);

        state.quads.clear();
    }

    ImGui::End();
}

void drawShadingModelViewer(const ProgramState& state, const Trackball& camera)
{
    glClearColor(state.backgroundColor.r, state.backgroundColor.g, state.backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Initialize projection and view matrices.
    glMatrixMode(GL_PROJECTION);
    const auto projectionMatrix = camera.projectionMatrix();
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    glMatrixMode(GL_MODELVIEW);
    const auto viewMatrix = camera.viewMatrix();
    glLoadMatrixf(glm::value_ptr(viewMatrix));

    // Drawing mode options.
    glEnable(GL_DEPTH_TEST); // Enable depth test.
    glDepthMask(GL_TRUE); // Enable depth write.
    glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    /*
     * Run their methods
     */
    if (state.shadingModelSphere.mode != SphereMode::SphereNone) {
        // TODO Make sure all of this works when only parts of it are implemented yet.
        auto displacedVertices = generateSphereVertices(static_cast<int>(state.shadingModelSphere.resolution_lat),
                                                        static_cast<int>(state.shadingModelSphere.resolution_long));

        glm::vec3 position;
        glm::vec3 normal;
        std::vector<glm::vec3> viewDirections { displacedVertices.size() };
        getReflectedLightInputParameters(displacedVertices, position, normal, viewDirections);
        
        std::vector<glm::vec3> sphereVertexColors { displacedVertices.size() };
        for (size_t v = 0; v < displacedVertices.size(); v++) {
            sphereVertexColors[v] = computeLighting(state,
                { .position = position, .normal = normal },
                viewDirections[v],
                state.light);
        }

        displaceVerticesByIntensity(sphereVertexColors, displacedVertices);

        if (state.shadingModelSphere.mode == SphereMode::SpherePoints) {
            drawSphereGrid(displacedVertices, sphereVertexColors);
        } else if (state.shadingModelSphere.mode == SphereMode::SphereVectors) {
            drawSphereVectors(displacedVertices, sphereVertexColors);
        } else if (state.shadingModelSphere.mode == SphereMode::SphereMesh) {
            std::vector<glm::uvec3> triangles {};
            std::vector<glm::uvec4> quads {};

            generateSphereMesh(state.shadingModelSphere.resolution_lat, state.shadingModelSphere.resolution_long, triangles, quads);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            drawSphereMesh(displacedVertices, sphereVertexColors, triangles, quads);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    drawSurfacePatch();
    drawSurfacePoint();
    drawSurfaceBoundary();
    drawLightDirectionAndNormal(state.light);
}

int main(int argc, char** argv)
{
    Window window { "Shading Assignment", glm::ivec2(800), OpenGLVersion::GL2 };
    Trackball trackball { &window, glm::radians(60.0f), 2.0f, 0.4f, -2.4f };
    trackball.disableTranslation();

    printHelp();

    // this function loads a mesh
    ProgramState state {};
    setBarbieCar(state, trackball);

    window.registerCharCallback([&](unsigned unicodeCodePoint) {
        keyboard(static_cast<unsigned char>(unicodeCodePoint), state, window, trackball);
    });

    while (!window.shouldClose()) {
        window.updateInput();
        glViewport(0, 0, window.getFrameBufferSize().x, window.getFrameBufferSize().y);

        std::vector<glm::vec3> vertexColors;
        if (section != 2) {
            vertexColors = std::vector<glm::vec3>(state.myMesh.vertices.size());
            computeLighting(state, trackball.position(), vertexColors);
            draw(state, trackball, vertexColors);
        } else {
            drawShadingModelViewer(state, trackball);
        }
        drawUI(state, trackball, vertexColors);

        window.swapBuffers();
    }
}
