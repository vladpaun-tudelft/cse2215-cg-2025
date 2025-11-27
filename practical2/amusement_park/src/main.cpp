#include "your_code_here_amusement_park.h"
// Include your_code_here first so that students have to include what they use.
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <framework/file_picker.h>
#include <framework/imgui_helper.h>
#include <framework/mesh.h>
#include <framework/trackball.h>
#include <framework/window.h>
#include <fstream>
#include <limits>
#include <numeric>
#include <span>
#include <vector>

static const std::filesystem::path dataDirPath { DATA_DIR };
static const std::filesystem::path saveFilePath = dataDirPath / "save.bin";
using Clock = std::chrono::high_resolution_clock;

void drawMesh(const Mesh& mesh)
{
    glBegin(GL_TRIANGLES);
    glColor3fv(glm::value_ptr(mesh.material.kd));
    for (const auto& triangle : mesh.triangles) {
        for (int v = 0; v < 3; v++) {
            const auto& vertex = mesh.vertices[triangle[v]];
            glNormal3fv(glm::value_ptr(vertex.normal));
            glVertex3fv(glm::value_ptr(vertex.position));
        }
    }
    glEnd();
}

struct TopSpinTransition {
    int id; // Used for UI only.
    float transitionTime = 1.0f; // Time in seconds.
    TopSpinState state;
};

class TopSpin {
public:
    Mesh meshSeats;
    Mesh meshHarness;
    Mesh meshArms;
    Mesh meshSupports;

public:
    static TopSpin create()
    {
        TopSpin out {};
        out.meshSeats = loadMesh(dataDirPath / "topspin_seats.obj", { .cacheVertices = false })[0];
        out.meshHarness = loadMesh(dataDirPath / "topspin_harness.obj", { .cacheVertices = false })[0];
        out.meshArms = loadMesh(dataDirPath / "topspin_arms.obj", { .cacheVertices = false })[0];
        out.meshSupports = loadMesh(dataDirPath / "topspin_supports.obj", { .cacheVertices = false })[0];
        return out;
    }

    void draw(const TopSpinMatrices& matrices) const
    {
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(matrices.supports));
        drawMesh(meshSupports);
        glPopMatrix();

        glPushMatrix();
        glMultMatrixf(glm::value_ptr(matrices.arms));
        drawMesh(meshArms);
        glPopMatrix();

        glPushMatrix();
        glMultMatrixf(glm::value_ptr(matrices.seats));
        drawMesh(meshSeats);
        glPopMatrix();

        for (const auto& harnessMatrix : matrices.harnesses) {
            glPushMatrix();
            glMultMatrixf(glm::value_ptr(harnessMatrix));
            drawMesh(meshHarness);
            glPopMatrix();
        }
    }
};

TopSpinState interpolateTopSpinState(std::span<const TopSpinTransition> program, float currentTime)
{
    // The provided program describes the time between steps. Convert this to the absolute time at each step.
    std::vector<TopSpinTransition> programAtTimeSteps;
    {
        float prevStateEndTime = 0.0f;
        programAtTimeSteps.push_back({ .transitionTime = 0.0f, .state = {} }); // Initial state.
        for (const auto& transition : program) {
            prevStateEndTime += transition.transitionTime;
            programAtTimeSteps.push_back({ .transitionTime = prevStateEndTime, .state = transition.state });
        }
    }

    auto iter = std::lower_bound(std::begin(programAtTimeSteps), std::end(programAtTimeSteps), TopSpinTransition { .transitionTime = currentTime }, [](const auto& lhs, const auto& rhs) { return lhs.transitionTime < rhs.transitionTime; });
    assert(iter != std::end(programAtTimeSteps));

    if (iter == std::begin(programAtTimeSteps))
        return programAtTimeSteps[0].state;

    const auto& nextTransition = *iter;
    const auto& currentTransition = *(--iter);
    const auto interpolation = (currentTime - currentTransition.transitionTime) / (nextTransition.transitionTime - currentTransition.transitionTime);
    return TopSpinState {
        .armAngle = glm::mix(currentTransition.state.armAngle, nextTransition.state.armAngle, interpolation),
        .seatsAngle = glm::mix(currentTransition.state.seatsAngle, nextTransition.state.seatsAngle, interpolation),
        .harnessAngle = glm::mix(currentTransition.state.harnessAngle, nextTransition.state.harnessAngle, interpolation)
    };
}

float computeProgramDuration(std::span<const TopSpinTransition> program)
{
    float programDuration = 0.0f;
    for (const TopSpinTransition& transition : program)
        programDuration += transition.transitionTime;
    return programDuration;
}

void drawProgramGUI(std::vector<TopSpinTransition>& program, float& time, bool& paused)
{
    ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Save")) {
        if (auto optFilePath = pickSaveFile("bin")) {
            optFilePath->replace_extension(".bin");
            std::ofstream outFile { *optFilePath, std::ios::binary };
            outFile << program.size();
            outFile.write((const char*)program.data(), program.size() * sizeof(TopSpinTransition));
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        if (auto optFilePath = pickOpenFile("bin")) {
            std::ifstream inFile { *optFilePath, std::ios::binary };
            size_t programSize;
            inFile >> programSize;
            program.resize(programSize);
            inFile.read((char*)program.data(), programSize * sizeof(TopSpinTransition));
        }
    }

    ImGui::Spacing();

    ImGui::SliderFloat("Time", &time, 0.0f, computeProgramDuration(program), "%.2f s", 1.0f);
    ImGui::SameLine();
    if (ImGui::Button(paused ? "play" : "pause"))
        paused = !paused;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    drawImGuiDynamicArray(
        program,
        [](TopSpinTransition& transition) {
            ImGui::InputFloat("Transition Time (s)", &transition.transitionTime);
            ImGui::InputFloat("Arm Angle (degrees)", &transition.state.armAngle);
            ImGui::InputFloat("Seats Angle (degrees)", &transition.state.seatsAngle);
            ImGui::InputFloat("Harness Angle (degrees)", &transition.state.harnessAngle);
            transition.transitionTime = std::max(transition.transitionTime, 0.1f); // Prevent transitionTime==0
        });
    ImGui::End();
}

int main(int argc, char** argv)
{
    int renderWidthInPixels = 800;
    int guiWidthInPixels = 450;
    Window window { "Amusement Park", glm::ivec2(renderWidthInPixels + guiWidthInPixels, renderWidthInPixels), OpenGLVersion::GL2 };
    Trackball camera { &window, glm::radians(60.0f), glm::vec3(0, 8, 0), 30.0f, 0.4f, -1.0f };
    TopSpin topSpin = TopSpin::create();
    const Mesh groundPlane = loadMesh(dataDirPath / "ground_plane.obj")[0];

    // Account for high-dpi resolution scaling.
    renderWidthInPixels = int(window.getDpiScalingFactor() * renderWidthInPixels);
    guiWidthInPixels = int(window.getDpiScalingFactor() * guiWidthInPixels);

    std::vector<TopSpinTransition> program {
        { .transitionTime = 1.0f, .state = { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 90.0f } },
        { .transitionTime = 0.5f, .state = { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.5f, .state = { .armAngle = 180.0f, .seatsAngle = 0.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 1.0f, .state = { .armAngle = 180.0f, .seatsAngle = 180.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.0f, .state = { .armAngle = 0.0f, .seatsAngle = 180.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.0f, .state = { .armAngle = -180.0f, .seatsAngle = -20.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.0f, .state = { .armAngle = -360.0f, .seatsAngle = -240.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.0f, .state = { .armAngle = -540.0f, .seatsAngle = -360.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 1.8f, .state = { .armAngle = -540.0f, .seatsAngle = -360.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 1.8f, .state = { .armAngle = -360.0f, .seatsAngle = -360.0f, .harnessAngle = 0.0f } },
        { .transitionTime = 2.5f, .state = { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 00.0f } },
        { .transitionTime = 0.5f, .state = { .armAngle = 0.0f, .seatsAngle = 0.0f, .harnessAngle = 90.0f } }
    };
    for (int i = 0; i < program.size(); ++i)
        program[i].id = i;

    bool paused = false;
    float timeInSeconds = 0.0f;
    auto prevFrameTimePoint = Clock::now();
    while (!window.shouldClose()) {
        window.updateInput();

        const auto now = Clock::now();
        const auto dt = std::chrono::duration<float>(now - prevFrameTimePoint);
        prevFrameTimePoint = now;
        if (!paused)
            timeInSeconds += dt.count();
        timeInSeconds = std::min(timeInSeconds, computeProgramDuration(program));

        renderWidthInPixels = window.getFrameBufferSize().x - guiWidthInPixels; // Account for user resizing the window.
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(guiWidthInPixels, 0, renderWidthInPixels, window.getFrameBufferSize().y);

        // Enable depth testing.
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_SMOOTH);

        // Activate the light in the legacy OpenGL mode.
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        constexpr glm::vec4 lightPos { 0, 10, -20, 1 };
        glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(lightPos));

        // Initialize projection and view matrices.
        glMatrixMode(GL_PROJECTION);
        const auto projectionMatrix = camera.projectionMatrix((float)renderWidthInPixels / (float)window.getFrameBufferSize().y);
        glLoadMatrixf(glm::value_ptr(projectionMatrix));
        glMatrixMode(GL_MODELVIEW);
        const auto viewMatrix = camera.viewMatrix();
        glLoadMatrixf(glm::value_ptr(viewMatrix));

        drawMesh(groundPlane);
        const auto topSpinState = interpolateTopSpinState(program, timeInSeconds);
        const auto matrices = computeTopSpinTransformations(topSpinState);
        topSpin.draw(matrices);
        glPopAttrib();

        // Draw the Graphical User Interface
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)guiWidthInPixels, (float)renderWidthInPixels), ImGuiCond_Always);
        drawProgramGUI(program, timeInSeconds, paused);

        window.swapBuffers();
    }
}
