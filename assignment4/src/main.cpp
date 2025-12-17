#include "draw.h"
#include "ray_tracing.h"
// Disable compiler warnings in third-party code (which we cannot change).
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/trackball.h>
#include <framework/window.h>
#include <optional>

#include "pacman.h"


// This is the main application. The code in here does not need to be modified.
constexpr glm::ivec2 windowResolution { 800, 800 };

struct ProgramState {
    std::optional<Ray> optRay {};

    const std::vector<glm::vec2> pacmanShape = generateCirclePoints(0.4f, glm::vec2(0.0f, 0.0f), Game::pacmanShapeN);
    const std::array<glm::vec2, Game::ghostShapeN> ghostShape {{
        {-0.5f,  -0.5f},
        {0.5f,   -0.5f},
        {0.5f,   0.0f},
        {0.25f,  0.5f},
        {-0.25f, 0.5f},
        {-0.5f,  0.0f}
    }};

    Vertices pacmanVertices {};
    Hull pacmanHull {};

    std::vector<Vertices> ghostVertices{Game::nGhosts};
    std::vector<Hull> ghostHulls {Game::nGhosts};

    float focusTime = 0;

    struct VisualizationSettings{
        bool testScene = false;
        bool drawMaze = true;
        bool drawPacman = true;
        bool drawGhosts = true;
        bool drawHulls = true;
        bool drawRays = true;
        bool drawCameraLookAt = true;
        bool drawCoordinateSystem = true;
        bool drawEndCaps = true;
    } visuals;

    TestScene testScene;
    std::optional<Ray> testRay;
};

void updateFocusTime(ProgramState &ps, Trackball &camera, float change) {
    ps.focusTime += change;
    auto currentLookAt = camera.lookAt();
    currentLookAt.z -= change;

    float diff_max = ps.focusTime - (float) Game::sequenceLength;
    if (diff_max > 0.0f) {
        ps.focusTime = (float) Game::sequenceLength;
        currentLookAt.z += diff_max;
    }
    else if (ps.focusTime < 0.0f) {
        currentLookAt.z += ps.focusTime;
        ps.focusTime = 0.0f;
    }

    camera.setCamera(currentLookAt, camera.rotationEulerAngles(), camera.distanceFromLookAt());
}


void keyboard (Window &window, Trackball &camera, ProgramState &ps) {
    constexpr float timeIncOnButtonPress = 0.1f;
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_R: {
                    // Shoot a ray. Produce a ray from camera to the far plane.
                    if (!ps.visuals.testScene) {
                        ps.optRay = camera.generateRay(window.getNormalizedCursorPos() * 2.0f - 1.0f);
                        intersectRayWithGhosts(ps.ghostVertices, ps.ghostHulls, *ps.optRay);
                    }
                    else {
                        ps.testRay = camera.generateRay(window.getNormalizedCursorPos() * 2.0f - 1.0f);
                        intersectTestScene(ps.testScene, *ps.testRay);
                    }
                    ps.visuals.drawRays = true;
                } break;
                case GLFW_KEY_ESCAPE: {
                    window.close();
                } break;
                case GLFW_KEY_UP: {
                    updateFocusTime(ps, camera, timeIncOnButtonPress);
                } break;
                case GLFW_KEY_DOWN: {
                    updateFocusTime(ps, camera, -timeIncOnButtonPress);
                } break;
            }
        }
        else if (action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_UP: {
                    updateFocusTime(ps, camera, timeIncOnButtonPress);
                } break;
                case GLFW_KEY_DOWN: {
                    updateFocusTime(ps, camera, -timeIncOnButtonPress);
                } break;
            };
        }
    });
}

void gameSanityCheck(const ProgramState &ps) {
    assert(Game::pacmanMotion.size() == Game::sequenceLength);
    assert(Game::ghostColors.size() == Game::nGhosts);
    assert(Game::ghostInitialPositions.size() == Game::nGhosts);
    assert(Game::ghostMotions.size() == Game::nGhosts);
    for (const auto &motion : Game::ghostMotions) { assert(motion.size() == Game::sequenceLength); }
    assert(ps.ghostVertices.size() == Game::nGhosts);
    assert(ps.ghostHulls.size() == Game::nGhosts);
    assert(Game::maze.size() == Game::mazeSize.y);
    for (const auto &col : Game::maze) { assert(col.size() == Game::mazeSize.x); }
}

void drawImGui(ProgramState &ps) {
    ImGui::Begin("Ray Tracing Assignment");
    {
        ImGui::Text("Press [R] to trace a ray from the camera at the current cursor position.\nMove camera afterwards to make it visible.");
        ImGui::Spacing();
        ImGui::Text("Press or hold down [UP]/[DOWN] to navigate in time. This will also change the camera lookAt to follow along.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Show the test scene and not the pacman scene. The test scene is to be (optionally) implemented by you,\n"
                    "if you want to debug the triangle intersections with something other than the ghost hulls.");
        ImGui::Spacing();
        ImGui::Checkbox("Test Scene", &ps.visuals.testScene);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("To simplify debugging you can deactivate parts of the visualization with these checkboxes:");
        ImGui::Spacing();
        ImGui::Checkbox("Coordinate system", &ps.visuals.drawCoordinateSystem);
        ImGui::Checkbox("Camera lookat as a small sphere", &ps.visuals.drawCameraLookAt);
        if (!ps.visuals.testScene) {
            ImGui::Spacing();
            ImGui::Checkbox("Game grid", &ps.visuals.drawMaze);
            ImGui::Checkbox("Pacman", &ps.visuals.drawPacman);
            ImGui::Checkbox("Ghosts", &ps.visuals.drawGhosts);
            ImGui::Spacing();
            ImGui::Checkbox("Space Time Hulls", &ps.visuals.drawHulls);
            ImGui::Checkbox("Endcaps for the Hulls", &ps.visuals.drawEndCaps);
            ImGui::Spacing();
        }
        ImGui::Checkbox("Rays (manually generated and pacman ones)", &ps.visuals.drawRays);
    }
    ImGui::End();
}

void openglPreamble(const Trackball & camera) {
    // Clear screen.
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Normals will be normalized in the graphics pipeline.
    glEnable(GL_NORMALIZE);

    // Interpolate vertex colors over the triangles.
    glShadeModel(GL_SMOOTH);

    // Load view and projection matrix matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const glm::mat4 viewMatrix = camera.viewMatrix();
    glMultMatrixf(glm::value_ptr(viewMatrix));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const glm::mat4 projectionMatrix = camera.projectionMatrix();
    glMultMatrixf(glm::value_ptr(projectionMatrix));

    // Specify backface treatment, disable depth tests and activate alpha blending
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawPacmanGame(const ProgramState &ps) {
    // Draw Game Grid at currently selected time stamp
    std::vector<MazeTile> tiles = generateMaze(Game::mazeCenter, Game::maze, ps.focusTime);
    if (ps.visuals.drawMaze) {
        drawMaze(tiles);
    }

    // draw transparent hulls, we want back face culling for the hulls
    if (ps.visuals.drawHulls) {
        drawHullMesh(ps.pacmanVertices, ps.pacmanHull, glm::vec4(Game::pacmanColor, 0.2f));
        for (size_t i = 0; i < Game::nGhosts; i++) {
            drawHullMesh(ps.ghostVertices[i], ps.ghostHulls[i], glm::vec4(Game::ghostColors[i], 0.2f));
        }
    }

    // draw end caps for the hull for a nicer look / no effect for the intersections!
    if (ps.visuals.drawEndCaps) {
        drawPolygonT(ps.pacmanShape, Game::pacmanInitialPosition, Game::pacmanMotion, 0, Game::pacmanColor);
        drawPolygonT(ps.pacmanShape, Game::pacmanInitialPosition, Game::pacmanMotion, Game::sequenceLength,Game::pacmanColor);

        for (size_t i = 0; i < Game::nGhosts; i++) {
            drawPolygonT(ps.ghostShape, Game::ghostInitialPositions[i], Game::ghostMotions[i], 0, Game::ghostColors[i]);
            drawPolygonT(ps.ghostShape, Game::ghostInitialPositions[i], Game::ghostMotions[i], Game::sequenceLength,Game::ghostColors[i]);
        }
    }

    // draw pacman and ghosts at currently selected time step
    if (ps.visuals.drawPacman) {
        drawSelectedPolygon(ps.pacmanShape, Game::pacmanInitialPosition, Game::pacmanMotion, ps.focusTime, Game::pacmanColor);
    }
    if (ps.visuals.drawGhosts) {
        for (size_t i = 0; i < Game::nGhosts; i++) {
            drawSelectedPolygon(ps.ghostShape, Game::ghostInitialPositions[i], Game::ghostMotions[i], ps.focusTime,
                                Game::ghostColors[i]);
        }
    }
}


int main(int argc, char** argv)
{
    Trackball::printHelp();

    ProgramState ps {};
    gameSanityCheck(ps);

    Window window { argv[0], windowResolution, OpenGLVersion::GL2 };
    Trackball camera { &window, glm::radians(50.0f), 3.0f };
    camera.setCamera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(glm::vec3(20.0f, 120.0f, 0.0f)), 20.0f);


    keyboard(window, camera, ps);

    // create the hull geometry defined by the polygon shape and the motion behavior
    std::tie(ps.pacmanHull, ps.pacmanVertices) = generateHullGeometry(
            ps.pacmanShape, Game::pacmanInitialPosition, Game::pacmanMotion);

    for (size_t i = 0; i < Game::nGhosts; i++) {
        std::tie(ps.ghostHulls[i], ps.ghostVertices[i]) = generateHullGeometry(
                ps.ghostShape, Game::ghostInitialPositions[i], Game::ghostMotions[i]);
    }

    ps.testScene = generateTestScene();

    while (!window.shouldClose()) {
        window.updateInput();
        drawImGui(ps);

        glViewport(0, 0, window.getFrameBufferSize().x, window.getFrameBufferSize().y);

        openglPreamble(camera);

        if (!ps.visuals.testScene) {
            drawPacmanGame(ps);
        }
        else {
            drawTestScene(ps.testScene);
        }

        if (ps.visuals.drawCoordinateSystem) {
            drawCoordSystem(1.0f, {ColorPalette::COLOR6, ColorPalette::COLOR7, ColorPalette::COLOR8});
        }

        if (ps.visuals.drawRays) {
            if (ps.optRay && !ps.visuals.testScene) {
                drawRay(*ps.optRay, {ColorPalette::COLOR9, ColorPalette::COLOR10, ColorPalette::COLOR11});
            }
            if (ps.testRay && ps.visuals.testScene) {
                drawRay(*ps.testRay, {ColorPalette::COLOR9, ColorPalette::COLOR10, ColorPalette::COLOR11});
            }
        }

        if (!ps.visuals.testScene) {
            std::vector<Ray> rays = generatePacmanRays(ps.pacmanShape, Game::pacmanInitialPosition, Game::pacmanMotion, ps.focusTime);
            for (auto &ray: rays) {
                intersectRayWithGhosts(ps.ghostVertices, ps.ghostHulls, ray);
                if (ps.visuals.drawRays) {
                    drawRay(ray, {ColorPalette::COLOR9, ColorPalette::COLOR10, ColorPalette::COLOR11});
                }
            }
        }

        // Draw a colored sphere at the location at which the trackball is looking/rotating around.
        if (ps.visuals.drawCameraLookAt) {
            drawSphere(camera.lookAt(), 0.1f, ColorPalette::COLOR5);
        }
        window.swapBuffers();
    }

    return 0; // execution never reaches this point
}
