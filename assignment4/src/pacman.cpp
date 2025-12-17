#include "pacman.h"
#include <framework/opengl_includes.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

/*
 * GENERATE MAZE VISUALIZATION
 * Generate the maze visualization by defining the quads and their color, given
 *      the designated mazeCenter (0 based indices (x, y) of the tile whose center should be located at the origin of the xy-plane)
 *      the maze defined by a 2D-array of bools maze[y][x] indicating which tiles should be filled (true=filled)
 *      together with its sizes mazeSizeX, mazeSizeY
 *      and the time indicating the currently selected time we want to visualize (and to which the maze should be translated)
 * The size of cells and the colors to use for a filled/empty cell are provided.
 * Watch out: Positive time should evolve in negative z direction!
 * Return the generated maze visualization as a vector of MazeTiles, each containing four vertices and a color
 * ready to be rendered with GL_QUADS in the next step
 */
std::vector<MazeTile> generateMaze(const glm::ivec2 mazeCenter, const Game::Maze &maze, const float time) {
    constexpr glm::vec2 cell_size {1.0f, 1.0f};
    constexpr glm::vec4 color_filled_cell {0.8f, 0.8f, 0.8f, 0.5f};
    constexpr glm::vec4 color_empty_cell {0.8f, 0.8f, 0.8f, 0.05f};

    std::vector<MazeTile> tiles;

    const size_t mazeSizeY = maze.size();
    assert(!maze.empty());
    const size_t mazeSizeX = maze[0].size();


    return tiles;
}

/*
 * Draw the maze of the previously generated maze tiles using GL_QUADS
*/
void drawMaze(std::span<const MazeTile> tiles) {
}


/*
 * Generate Circle Points
 * We approximate PacMan based on points on its bounding circle
 * Function to generate points on a circle
 * r: radius of the circle
 * center: xy-coordinates of the center of the circle
 * numPoints: number of points
 */
std::vector<glm::vec2> generateCirclePoints(float radius, glm::vec2 center, size_t numPoints)
{
    std::vector<glm::vec2> points (numPoints);


    return points;
}

/*
 * DRAW A POLYGON in 3D defined by its 2D shape - a span of 2D vertices - using GL_POLYGON
 * the time determines the displacement in negative z direction
 * the color determines its color
 */
void drawPolygon(std::span<const glm::vec2> polygon, const float time, const glm::vec4& color)
{
}

/*
 * DRAW THE OUTLINE OF A POLYGON in 3D defined by its 2D shape - a span of 2D vertices - using GL_LINE_LOOP
 * the time determines the displacement in negative z direction
 * color determines its color
 */
void drawPolygonOutline(std::span<const glm::vec2> polygon, const float time, const glm::vec4& color)
{
    glLineWidth(2);

}

/*
 * APPLY INITIAL POSITIONS
 * Get the initial 2D (xy) location of the vertices of the polygon (pacman or ghosts) given:
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost)
 *   and the initialPosition of that shape provided as a 2D displacement in the xy-plane.
 * Return the displaced polygon
 */
std::vector<glm::vec2> applyInitialPosition(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition) {

    std::vector<glm::vec2> translatedPolygon;
    translatedPolygon.assign(polygon.begin(), polygon.end());


    return translatedPolygon;
}

/*
 * APPLY MOVEMENT IN TIME
 * Get the 2D (xy) location of the vertices of the polygon (pacman or ghosts) at time t
 * Given
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost),
 *   the motionSteps provided as a span of 2D displacements over time,
 *   and the time of interest t
 *
 * Return the displaced polygon
 */
std::vector<glm::vec2> applyMovementInTime(std::span<const glm::vec2> polygon, std::span<const glm::vec2> motionSteps, const float t) {

    std::vector<glm::vec2> translatedPolygon;
    translatedPolygon.assign(polygon.begin(), polygon.end());


    return translatedPolygon;
}

// Provided, don't touch this!
std::vector<glm::vec2> getPolygonT(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t) {
    std::vector<glm::vec2> translatedPolygon;
    translatedPolygon.assign(polygon.begin(), polygon.end());
    translatedPolygon = applyInitialPosition(translatedPolygon, initialPosition);
    translatedPolygon = applyMovementInTime(translatedPolygon, motionSteps, t);
    return translatedPolygon;
}

// Provided, don't touch this!
void drawPolygonT(
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t,
        const glm::vec3 color) {
    auto translatedPolygon = getPolygonT(polygon, initialPosition, motionSteps, t);
    drawPolygon(translatedPolygon, t, {color, 0.2f});
}

// Provided, don't touch this!
void drawSelectedPolygon (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t,
        const glm::vec3 color) {

    auto translatedPolygon = getPolygonT(polygon, initialPosition, motionSteps, t);
    drawPolygon(translatedPolygon, t, {color, 0.8f});
    drawPolygonOutline(translatedPolygon, t, {1.0f, 1.0f, 1.0f, 1.0f});
}


/*
 * GENERATE HULL GEOMETRY
 * Given
 *   a polygon defined by a span of 2D vertices (e.g. the shape of pacman or a ghost),
 *   the initialPosition of that shape provided as a 2D displacement in the xy-plane,
 *   and the motionSteps provided as a span of 2D displacements over time
 *
 * Generate a hull representing the moving shape in 3D spacetime.
 * The hull should consist of HullSegments representing constant motion
 * A new segment should only be started when a direction change happens
 * the HullSegments in Hull should be in the order of the motion steps
 *
 * Each segment consists of the faces defining its outer hull, where faces are simply triangles.
 * We are only interested in the surrounding hull, i.e. we discard triangles for the front and back caps of the segments
 *
 * The triangles are defined by the three indices of their three vertices in the vertex list.
 * The Vertex list should contain the vertices. You should reuse the vertices of neighbouring segments/faces.
 *
 * Return the Hull and the Vertices
 *
 * Once again, watch out: Positive time should develop in negative z direction
 */

// Aliases for the datatypes of Hull and Vertices for easier usage
using Face = glm::uvec3;
using HullSegment = std::vector<Face>;
using Hull = std::vector<HullSegment>;
using Vertices = std::vector<glm::vec3>;

std::tuple<Hull, Vertices> generateHullGeometry(
        std::span<const glm::vec2> polygon,
        const glm::vec2 initialPosition,
        std::span<const glm::vec2> motionSteps) {
    Hull hull {};
    Vertices vertices {};

    const size_t polygonN = polygon.size();
    const size_t sequenceLength = motionSteps.size();


    return {hull, vertices};
}


/*
 * DRAW HULL MESH
 * Given the previously generated vertices and hull, draw the hull using GL_TRIANGLES
 * use the provided vec4 color as color for the hull
 */
void drawHullMesh(const Vertices& vertices, const Hull& hull, glm::vec4 color)
{

}

/*
 * GENERATE PACMAN RAYS
 * Generate rays for the pacman collision tests
 *
 * Given the time of interest t
 * as well as the polygon, its initial position, and the motions, defined as before;
 * Generate rays from its vertices in its current motion direction
 *
 * Return the rays as a vector of rays.
 */
std::vector<Ray> generatePacmanRays (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t) {

    if (t >= (float) motionSteps.size()) { return {}; }
    std::vector<Ray> rays {};




    return rays;
}
