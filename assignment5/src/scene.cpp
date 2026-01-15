#include "scene.h"
#include <cmath>
#include <iostream>

Scene loadScenePrebuilt(SceneType type, const std::filesystem::path& dataDir)
{
    Scene scene;
    scene.type = type;
    switch (type) {
    case SingleTriangle: {
        // Load a 3D model with a single triangle
        auto subMeshes = loadMesh(dataDir / "triangle.obj");
        subMeshes[0].material.kd = glm::vec3(1.0f);
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        scene.lights.emplace_back(PointLight { glm::vec3(-1, 1, -1), glm::vec3(1) });
    } break;
    case Cube: {
        // Load a 3D model of a cube with 12 triangles
        auto subMeshes = loadMesh(dataDir / "cube.obj");
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        scene.lights.emplace_back(PointLight { glm::vec3(-1.0, 1.5, -1.0), glm::vec3(1) });
    } break;
    case Monkey: {
        // Load a 3D model of a Monkey
        auto subMeshes = loadMesh(dataDir / "monkey.obj", { .normalizeVertexPositions = true });
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        scene.lights.emplace_back(PointLight { glm::vec3(-1, 1, -1), glm::vec3(1) });
        scene.lights.emplace_back(PointLight { glm::vec3(1, -1, -1), glm::vec3(1) });
    } break;
    case Teapot: {
        // Load a 3D model of a Teapot
        auto subMeshes = loadMesh(dataDir / "teapot.obj", { .normalizeVertexPositions = true });
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        scene.lights.emplace_back(PointLight { glm::vec3(-1, 1, -1), glm::vec3(1) });
    } break;
    case Dragon: {
        // Load a 3D model of a Dragon
        auto subMeshes = loadMesh(dataDir / "dragon.obj", { .normalizeVertexPositions = true });
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        scene.lights.emplace_back(PointLight { glm::vec3(-1, 1, -1), glm::vec3(1) });
    } break;
    case Custom: {
        // === Replace custom.obj by your own 3D model (or call your 3D model custom.obj) ===
        auto subMeshes = loadMesh(dataDir / "custom.obj");
        std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        // === CHANGE THE LIGHTING IF DESIRED ===
        scene.lights.emplace_back(PointLight { glm::vec3(-1, 1, -1), glm::vec3(1) });
        // Spherical light: position, radius, color
        // scene.lights.push_back(SphericalLight{ glm::vec3(0, 1.5f, 0), 0.2f, glm::vec3(1) });
    } break;
    };

    return scene;
}

Scene loadSceneFromFile(const std::filesystem::path& path, const std::vector<std::variant<PointLight, SegmentLight, ParallelogramLight>>& lights)
{
    Scene scene;
    scene.lights = lights;

    auto subMeshes = loadMesh(path);
    std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));

    return scene;
}
