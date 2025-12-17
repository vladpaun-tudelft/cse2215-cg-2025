#include "maze.h"
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <framework/opengl_includes.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <array>
#include <cassert>
#include <random>
#include <set>
#include <stack>

Maze Maze::createEmpty(int width, int height)
{
    return Maze(width, height);
}

Maze Maze::createRandomized(int width, int height, unsigned randomSeed)
{
    // Random maze generation algorithm:
    // https://en.wikipedia.org/wiki/Maze_generation_algorithm
    std::uniform_int_distribution<int> angleDistribution(0xFFFFFFFF);
    std::mt19937 rng { randomSeed };

    Maze maze = createEmpty(width, height);

    // Mark the initial walls at the boundary of the maze as "visited" so we wont remove them.
    // Subsequently fill the entire maze with walls.
    struct Cell {
        int x, y;
        constexpr auto operator<=>(const Cell&) const = default;
    };
    std::set<Cell> visitedCells;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (maze.isWall(x, y))
                visitedCells.insert({ x, y });
            maze.addWall(x, y);
        }
    }

    size_t goalStackDepth = 0;
    std::stack<Cell> workItems;
    workItems.push({ .x = 1, .y = 1 });
    while (!workItems.empty()) {
        const auto currentCell = workItems.top();
        workItems.pop();
        maze.removeWall(currentCell.x, currentCell.y);
        visitedCells.emplace(currentCell);

        if (workItems.size() > goalStackDepth) {
            maze.goal = glm::ivec2(currentCell.x, currentCell.y);
            goalStackDepth = workItems.size();
        }

        int numNeighbours = 0;
        std::array<Cell, 4> neighbours;
        const auto addPotentialNeighbour = [&](int dx, int dy) {
            const Cell neighbour { .x = currentCell.x + dx, .y = currentCell.y + dy };
            if (neighbour.x < 0 || neighbour.x >= width || neighbour.y < 0 || neighbour.y >= height)
                return;
            if (!visitedCells.contains(neighbour))
                neighbours[numNeighbours++] = neighbour;
        };
        addPotentialNeighbour(-2, 0);
        addPotentialNeighbour(2, 0);
        addPotentialNeighbour(0, -2);
        addPotentialNeighbour(0, 2);

        if (numNeighbours == 0)
            continue;

        const auto selectedNeighbour = neighbours[angleDistribution(rng) % numNeighbours];
        const auto dx = selectedNeighbour.x - currentCell.x, dy = selectedNeighbour.y - currentCell.y;
        maze.removeWall(currentCell.x + dx / 2, currentCell.y + dy / 2); // Remove the wall between the current cell and the neighbour we're visiting.
        workItems.push(currentCell);
        workItems.push(selectedNeighbour);
    }

    return maze;
}

void Maze::addWall(int x, int y)
{
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    m_maze[y * width + x] = true;
}

void Maze::removeWall(int x, int y)
{
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    m_maze[y * width + x] = false;
}

bool Maze::isWall(int x, int y) const
{
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
    return m_maze[y * width + x];
}

bool Maze::isWall(const glm::ivec2& pos) const
{
    return isWall(pos.x, pos.y);
}

void Maze::setViewTransform() const
{
    glm::mat4 viewTransform = glm::identity<glm::mat4>();
    viewTransform = glm::translate(viewTransform, glm::vec3(-1, -1, 0));
    viewTransform = glm::scale(viewTransform, glm::vec3(2.0f / width, 2.0f / height, 1));
    glLoadMatrixf(glm::value_ptr(viewTransform));
}

void Maze::draw(float depth /*= 0.0f*/) const
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    setViewTransform();

    const float fWidthPlusOne = (float)width;
    const float fHeightPlusOne = (float)height;
    const float fCubeWidth = 2.0f / fWidthPlusOne;
    const float fCubeHeight = 2.0f / fHeightPlusOne;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float relativeX = (float(x) / fWidthPlusOne) * 2.0f - 1.0f;
            const float relativeY = (float(y) / fHeightPlusOne) * 2.0f - 1.0f;
            m_grass.draw(x, y, 0.9f);
            if (isWall(x, y)) {
                m_tree.draw(x, y, 0.8f);
            }
        }
    }
    m_goal.draw(goal, 0.1f);

    glPopMatrix();
}

Maze::Maze(int width, int height)
    : m_maze(width * height, 0)
    , m_grass("grass.png")
    , m_tree("tree.png")
    , m_goal("goal.png")
    , width(width)
    , height(height)
{
    for (int x = 0; x < width; ++x) {
        addWall(x, 0);
        addWall(x, height - 1);
    }
    for (int y = 0; y < width; ++y) {
        addWall(0, y);
        addWall(width - 1, y);
    }
}
