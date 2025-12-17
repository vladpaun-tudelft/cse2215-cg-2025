#pragma once
#include "../src/ray_tracing.h"
#include <framework/mesh.h>
#include <framework/ray.h>

enum SceneType {
    SingleTriangle,
    Cube,
    Dragon
};

struct Scene {
    std::vector<Mesh> meshes;
};

bool intersectRayWithShape(const Mesh& mesh, Ray& ray)
{
    bool hit = false;
    for (const auto& tri : mesh.triangles) {
        const auto v0 = mesh.vertices[tri[0]];
        const auto v1 = mesh.vertices[tri[1]];
        const auto v2 = mesh.vertices[tri[2]];
        hit |= intersectRayWithTriangle(v0.position, v1.position, v2.position, ray);
    }
    return hit;
}

bool intersectRayWithScene(const Scene& scene, Ray& ray)
{
    bool hit = false;
    for (const auto& mesh : scene.meshes)
        hit |= intersectRayWithShape(mesh, ray);
    return hit;
}

Scene loadTestScene(SceneType type, const std::filesystem::path& dataDir, std::optional<Ray>& optRay)
{
    Scene scene;
    switch (type) {
        case SingleTriangle: {
            // Load a 3D model with a single triangle
            auto subMeshes = loadMesh(dataDir / "triangle.obj");
            std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        } break;
        case Cube: {
            // Load a 3D model of a cube with 12 triangles
            auto subMeshes = loadMesh(dataDir / "cube.obj");
            std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        } break;
        case Dragon: {
            // Load a 3D model of a Dragon
            auto subMeshes = loadMesh(dataDir / "dragon.obj");
            std::move(std::begin(subMeshes), std::end(subMeshes), std::back_inserter(scene.meshes));
        } break;
    }

    return scene;
}
