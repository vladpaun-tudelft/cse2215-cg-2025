#pragma once

#include "scene.h"

void getTestScene(Scene& testScene) {
    testScene = Scene {
            .type = Custom,
            .meshes = {},
            .spheres = {},
            .lights = {}
    };

    // Place three triangles parallel to each other with two of them very close to each other
    for (int i=0; i<3; ++i) {
        float depth = i != 2 ? static_cast<float>(i) : 1.01f;
        testScene.meshes.push_back({
            .vertices = {
                    {.position = {1.f, 1.f, depth}, .normal = {}, .texCoord = {}},
                    {.position = {1.f, -1.f, depth}, .normal = {}, .texCoord = {}},
                    {.position = {-1.f, 1.f, depth},.normal = {}, .texCoord = {}}
                    },
            .triangles = {{0, 1, 2}},
            .material = {
                .kd = {1.f, 1.f, 1.f},
                .ks = {0.f, 0.f, 0.f},
                .shininess = 1.0f,
                .transparency = 1.0f,
                .kdTexture{nullptr}
            }
        });
    }
}

struct lightTest {
    Ray ray;
    HitInfo hitInfo;
    glm::vec3 position;
    bool expectation;
    std::string name;
};

void getLightTestData(std::vector<lightTest>& lightTests) {
    // make such that origin + t * raydirection is where you want the test to start
    // different light configurations (position, hitInfo.normal, and ray)

    // Basic Visible
    // coming from between the first two triangles, hit the second with normal directly opposite
    // check for an unobstructed light source, also between the two of them
    lightTests.push_back({
        .ray {
            .origin = {.5f, .5f, .5f},
            .direction = {0.f, 0.f, 1.f},
            .t = 0.5f
        },
        .hitInfo {.normal = {0.f, 0.f, -1.f}},  // TODO maybe fill rest with dummy information
        .position{.8f, .8f, .8f},
        .expectation = true,
        .name = "Basic Visible",
    });

    // Basic Not Visible
    // coming from between the first two triangles, hit the second with normal directly opposite
    // check for a light source behind the first triangle in a reasonable distance
    lightTests.push_back({
        .ray {
            .origin = {.5f, .5f, .5f},
            .direction = {0.f, 0.f, 1.f},
            .t = 0.5f
        },
        .hitInfo {.normal = {0.f, 0.f, -1.f}},  // TODO maybe fill rest with dummy information
        .position{.7f, .7f, -1.f},
        .expectation = false,
        .name = "Basic Not Visible",
    });
}