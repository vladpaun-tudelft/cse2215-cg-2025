#pragma once
#include "sprite.h"
#include <framework/disable_all_warnings.h>
#include <vector>
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
DISABLE_WARNINGS_POP()

class Maze {
public:
    int width, height;
    glm::ivec2 goal;

public:
    static Maze createEmpty(int width, int height);
    static Maze createRandomized(int width, int height, unsigned randomSeed = 123);

    void addWall(int x, int y);
    void removeWall(int x, int y);
    bool isWall(int x, int y) const;
    bool isWall(const glm::ivec2& pos) const;

    void setViewTransform() const;
    void draw(float depth = 0.0f) const;

private:
    Maze(int width, int height);

private:
    std::vector<bool> m_maze;
    Sprite m_grass, m_tree, m_goal;
};