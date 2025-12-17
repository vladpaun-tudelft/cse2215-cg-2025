// ==============================================
// ========   DO NOT MODIFY THIS FILE!   ========
// ==============================================
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <imgui/imgui.h>
#include <nativefiledialog/nfd.h>
DISABLE_WARNINGS_POP()
#include <filesystem>
#include <framework/opengl_includes.h>
#include <framework/window.h>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
    Window window { "Start-up Check", glm::ivec2(800, 480), OpenGLVersion::GL2 };
    while (!window.shouldClose()) {
        window.updateInput();

        // Initialize viewpoint.
        glViewport(0, 0, window.getFrameBufferSize().x, window.getFrameBufferSize().y);

        // Clear screen.
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

        ImGui::Begin("Hello World", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        ImGui::Text("If you are seeing this message, the project is correctly configured and you are good to go!");
        ImGui::End();
        ImGui::PopStyleVar();

        window.swapBuffers();
    }
}