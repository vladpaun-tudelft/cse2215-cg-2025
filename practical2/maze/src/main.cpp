#include "your_code_here_maze.h"
// Include your_code_here first so that students have to include what they use.
#include "maze.h"
#include "sprite.h"
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <chrono>
#include <filesystem>
#include <framework/file_picker.h>
#include <framework/imgui_helper.h>
#include <framework/window.h>
#include <fstream>
#include <random>
#include <span>

static const std::filesystem::path dataDirPath { DATA_DIR };

static constexpr std::array movementCommandNames { "Move Forward", "RotateLeft", "Rotate Right" };

struct ProgramState {
    unsigned mazeSeed { 123 };
    std::vector<MovementCommand> commands;
};
static void save(const ProgramState& programState, const std::filesystem::path& filePath)
{
    {
        std::ofstream file { filePath, std::ios::binary };
        file.write((const char*)&programState.mazeSeed, sizeof(programState.mazeSeed));
        const size_t numCommands = programState.commands.size();
        file.write((const char*)&numCommands, sizeof(numCommands));
        if (numCommands > 0)
            file.write((const char*)programState.commands.data(), programState.commands.size() * sizeof(MovementCommand));
    }

    {
        std::ifstream file { filePath, std::ios::binary };
        assert(file.is_open());
    }
}
static ProgramState load(const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists(filePath))
        return {};

    std::ifstream file { filePath, std::ios::binary };
    assert(file.is_open());
    ProgramState out;
    file.read((char*)&out.mazeSeed, sizeof(out.mazeSeed));
    assert(file.is_open());
    size_t numCommands;
    file.read((char*)&numCommands, sizeof(numCommands));
    assert(file.is_open());
    out.commands.resize(numCommands);
    if (numCommands > 0)
        file.read((char*)out.commands.data(), out.commands.size() * sizeof(MovementCommand));
    return out;
}

static glm::ivec2 getPosition(const glm::mat3& transform) { return glm::ivec2(transform * glm::vec3(0.5f, 0.5f, 1)); }

static bool isValidMove(const Maze& maze, const glm::mat3& transform)
{
    const glm::vec2 newPos = transform * glm::vec3(0.5f, 0.5f, 1);
    const glm::vec2 posInVoxel = newPos - glm::floor(newPos);
    if (glm::any(glm::lessThan(posInVoxel, glm::vec2(0.49f)) | glm::greaterThan(posInVoxel, glm::vec2(0.51f))))
        return false;

    const glm::ivec2 iNewPos = glm::ivec2(newPos);
    if (glm::any(glm::lessThan(iNewPos, glm::ivec2(0)) | glm::greaterThanEqual(iNewPos, glm::ivec2(maze.width, maze.height))))
        return false;
    return !maze.isWall(iNewPos);
}

int main(int argc, char** argv)
{
    int mazeSizeInPixels = 800;
    int guiWidthInPixels = 380;
    Window window { "Maze", glm::ivec2(mazeSizeInPixels + guiWidthInPixels, mazeSizeInPixels), OpenGLVersion::GL2 };
    ProgramState programState {};
    Maze maze = Maze::createRandomized(9, 9, programState.mazeSeed);

    // Account for high-dpi resolution scaling.
    mazeSizeInPixels = int(window.getDpiScalingFactor() * mazeSizeInPixels);
    guiWidthInPixels = int(window.getDpiScalingFactor() * guiWidthInPixels);

    using Clock = std::chrono::high_resolution_clock;
    auto lastAnimationUpdate = Clock::now();
    bool playAnimation = true;
    int animationFrame = 0;

    auto lastFrame = Clock::now();
    auto showInvalidMove = Clock::now();

    const Sprite character { "character.png" };
    const Sprite characterGray { "character_gray.png" };
    const Sprite characterBlue { "character_blue.png" };
    const Sprite goal { "goal.png" };
    while (!window.shouldClose()) {
        window.updateInput();

        const auto now = Clock::now();
        const float dt = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        // Go to the next animation frame after 500 milliseconds (0.5 seconds).
        if (playAnimation && std::chrono::duration<float, std::milli>(now - lastAnimationUpdate).count() > 500) {
            animationFrame = (animationFrame + 1) % (programState.commands.size() + 1);
            lastAnimationUpdate = now;
        } else if (!playAnimation) {
            lastAnimationUpdate = now;
        }

        // Draw the game.
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(guiWidthInPixels, 0, window.getFrameBufferSize().y, window.getFrameBufferSize().y);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

        maze.draw();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        maze.setViewTransform();
        const auto transform = computeMazeTransformation(std::span(programState.commands).subspan(0, animationFrame));
        if (getPosition(transform) == maze.goal)
            characterBlue.draw(transform, 0.0f);
        else if (isValidMove(maze, transform))
            character.draw(transform, 0.0f);
        else
            characterGray.draw(transform, 0.0f);
        glPopMatrix();

        glPopAttrib();

        // Draw the Graphical User Interface
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)guiWidthInPixels, (float)mazeSizeInPixels), ImGuiCond_Always);

        ImGui::Begin("User Interface", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("New Game")) {
            animationFrame = 0;
            programState.commands.clear();
            playAnimation = false;
            maze = Maze::createRandomized(maze.width, maze.height, std::random_device()());
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (auto optFilePath = pickSaveFile("bin")) {
                optFilePath->replace_extension(".bin");
                save(programState, *optFilePath);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            if (auto optFilePath = pickOpenFile("bin")) {
                programState = load(*optFilePath);
                playAnimation = true;
                animationFrame = 0;
                maze = Maze::createRandomized(maze.width, maze.height, programState.mazeSeed);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(playAnimation ? "Pause" : "Play"))
            playAnimation = !playAnimation;
        ImGui::SameLine();
        ImGui::SliderInt("Command", &animationFrame, 0, (int)programState.commands.size());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const auto makeMove = [&](MovementCommand command) {
            programState.commands.push_back(command);
            animationFrame = (int)programState.commands.size();
            playAnimation = false;
        };
        if (ImGui::Button("Turn Left"))
            makeMove(MovementCommand::RotateLeft);
        ImGui::SameLine();
        if (ImGui::Button("Forward"))
            makeMove(MovementCommand::MoveForward);
        ImGui::SameLine();
        if (ImGui::Button("Turn Right"))
            makeMove(MovementCommand::RotateRight);
        ImGui::BeginDisabled(programState.commands.empty());
        if (ImGui::Button("Remove Last Movement")) {
            programState.commands.pop_back();
            animationFrame = std::min(animationFrame, (int)programState.commands.size());
        }
        ImGui::EndDisabled();

        bool isValidPath = true;
        for (size_t i = 0; i < programState.commands.size() + 1; ++i) {
            const glm::mat3 tmp = computeMazeTransformation(std::span(programState.commands).subspan(0, i));
            isValidPath &= isValidMove(maze, tmp);
        }

        if (!isValidPath) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::Text("INTERSECTION WTIH TREES DETECTED");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const auto numMovementCommandsBefore = programState.commands.size();
        bool movementCommandChanged = false;
        drawImGuiDynamicArray(
            programState.commands,
            [&](MovementCommand& movementCommand) {
                movementCommandChanged |= ImGui::Combo("Movement", (int*)&movementCommand, movementCommandNames.data(), (int)movementCommandNames.size());
            },
            { .allowEmptyArray = true, .showAddButton = false });
        movementCommandChanged |= (programState.commands.size() != numMovementCommandsBefore);
        if (movementCommandChanged) {
            animationFrame = (int)programState.commands.size();
            playAnimation = false;
        }

        ImGui::End();
        window.swapBuffers();
    }
}
