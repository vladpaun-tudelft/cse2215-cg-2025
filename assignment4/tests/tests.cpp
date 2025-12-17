#include "../src/ray_tracing.h"
// Include ray_tracing.h first like in the student code (no extra include headers for them).
#include "./intersect_scene.cpp"
#include "../src/pacman.h"

// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/glm_ostream.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <span>
#include <filesystem>
#include <set>
#include <ranges>
#include <random>
//#include <glm/gtx/string_cast.hpp>

#include "reference_pacman.h"

// very generous threshold
const float epsilon = 1e-2f;

std::ostream& operator<<(std::ostream& stream, const MazeTile &maze_tile)
{
    stream << "MazeTile {";
    stream << "v1: " << maze_tile.v1;
    stream << ", v2: " << maze_tile.v2;
    stream << ", v3: " << maze_tile.v3;
    stream << ", v4: " << maze_tile.v4;
    stream << ", color: " << maze_tile.color;
    stream << "}";
    return stream;
}

std::ostream& operator<<(std::ostream &stream, const Ray &ray) {
    stream << "Ray {";
    stream << "origin: " << ray.origin;
    stream << ", direction: " << ray.direction;
    stream << ", t: " << ray.t;
    stream << "}";
    return stream;
}

static Scene loadScene(SceneType type, const std::filesystem::path& dataDir)
{
    std::optional<Ray> dummy;
    return loadTestScene(type, dataDir, dummy);
}

// test a list of rays from a file against a testscene (indicated in file)
void testIntersections(const std::filesystem::path& testfilePath)
{
    std::ifstream ifs(testfilePath);
    assert(ifs.is_open());

    Ray ray;

    // load scene
    int scene_id;
    ifs >> scene_id;
    const Scene scene = loadScene(static_cast<SceneType>(scene_id), DATA_DIR);

    float solution_t = 0.0f;
    bool solution_hit;
    glm::vec3 solution_hit_point;

    // check all rays in file
    while (ifs >> solution_hit) {
        ifs >> ray.origin.x >> ray.origin.y >> ray.origin.z;
        ifs >> ray.direction.x >> ray.direction.y >> ray.direction.z;
        ifs >> solution_t;
        ifs >> solution_hit_point.x >> solution_hit_point.y >> solution_hit_point.z;

        // reset ray t value
        ray.t = std::numeric_limits<float>::max();

        // student's answer
        bool hit = intersectRayWithScene(scene, ray);

        CAPTURE(scene_id, ray.origin, ray.direction, solution_hit, solution_t, hit, ray.t);
        REQUIRE(solution_hit == hit);
        if (solution_hit) {
            REQUIRE(solution_t == Catch::Approx(ray.t).margin(epsilon));
        }
    }
}

void testSingleTriangleIntersections(const std::filesystem::path& testfilePath)
{
    std::ifstream ifs(testfilePath);
    assert(ifs.is_open());

    Ray ray;

    // load scene
    int scene_id;
    ifs >> scene_id;
    const Scene scene = loadScene(static_cast<SceneType>(scene_id), DATA_DIR);

    float solution_t, planeD;
    bool solution_hit, hitPlane, insideTriangle;
    glm::vec3 solution_hit_point, planeNormal, v0, v1, v2;

    // check all rays in file
    while (ifs >> solution_hit) {
        ifs >> v0.x >> v0.y >> v0.z;
        ifs >> v1.x >> v1.y >> v1.z;
        ifs >> v2.x >> v2.y >> v2.z;
        ifs >> ray.origin.x >> ray.origin.y >> ray.origin.z;
        ifs >> ray.direction.x >> ray.direction.y >> ray.direction.z;
        ifs >> solution_t;
        ifs >> hitPlane;
        ifs >> planeD;
        ifs >> planeNormal.x >> planeNormal.y >> planeNormal.z;
        ifs >> insideTriangle;
        ifs >> solution_hit_point.x >> solution_hit_point.y >> solution_hit_point.z;

        // reset ray t value
        ray.t = std::numeric_limits<float>::max();

        // student's answer
        bool hit = intersectRayWithTriangle(v0, v1, v2, ray);

        CAPTURE(scene_id, ray.origin, ray.direction, solution_hit, solution_t, hit, ray.t);
        REQUIRE(solution_hit == hit);
        if (solution_hit) {
            REQUIRE(solution_t == Catch::Approx(ray.t).margin(epsilon));
        }
    }
}

// test a list of rays from a file against a testscene (indicated in file)
void testPlaneCreation(const std::filesystem::path& testfilePath)
{
    std::ifstream ifs(testfilePath);
    assert(ifs.is_open());

    Ray ray;

    // load scene
    int scene_id;
    ifs >> scene_id;
    const Scene scene = loadScene(static_cast<SceneType>(scene_id), DATA_DIR);

    float solution_t = 0.0f;
    bool solution_hit;
    bool solution_inside_triangle;
    bool solution_hit_plane;
    Plane solution_plane;
    glm::vec3 solution_hit_point;

    auto v0 = scene.meshes[0].vertices[0].position;
    auto v1 = scene.meshes[0].vertices[1].position;
    auto v2 = scene.meshes[0].vertices[2].position;
    //    hit |= intersectRayWithTriangle(v0.p, v1.p, v2.p, ray);

    // check all rays in file
    while (ifs >> solution_hit) {
        ifs >> v0.x >> v0.y >> v0.z;
        ifs >> v1.x >> v1.y >> v1.z;
        ifs >> v2.x >> v2.y >> v2.z;
        ifs >> ray.origin.x >> ray.origin.y >> ray.origin.z;
        ifs >> ray.direction.x >> ray.direction.y >> ray.direction.z;
        ifs >> solution_t;
        ifs >> solution_hit_plane;
        ifs >> solution_plane.D;
        ifs >> solution_plane.normal.x >> solution_plane.normal.y >> solution_plane.normal.z;
        ifs >> solution_inside_triangle;
        ifs >> solution_hit_point.x >> solution_hit_point.y >> solution_hit_point.z;

        // first test plane creation
        Plane plane = trianglePlane(v0, v1, v2);

        CAPTURE(v0, v1, v2, solution_plane.D, solution_plane.normal, plane.D, plane.normal);

        REQUIRE(std::abs(solution_plane.D) == Catch::Approx(std::abs(plane.D)).margin(epsilon));
        REQUIRE(glm::length(plane.normal) == Catch::Approx(1.0).margin(epsilon));
        REQUIRE(fabs(glm::dot(solution_plane.normal, glm::normalize(plane.normal))) == Catch::Approx(1.0).margin(epsilon));
    }
}

// test a list of rays from a file against a testscene (indicated in file)
// tests only for the plane intersection part of the code
void testPlaneIntersection(const std::filesystem::path& testfilePath)
{
    std::ifstream ifs(testfilePath);
    assert(ifs.is_open());

    Ray ray;

    // load scene
    int scene_id;
    ifs >> scene_id;
    const Scene scene = loadScene(static_cast<SceneType>(scene_id), DATA_DIR);

    float solution_t = 0.0f;
    bool solution_hit;
    bool solution_inside_triangle;
    bool solution_hit_plane;
    Plane solution_plane;
    glm::vec3 solution_hit_point;

    auto v0 = scene.meshes[0].vertices[0].position;
    auto v1 = scene.meshes[0].vertices[1].position;
    auto v2 = scene.meshes[0].vertices[2].position;

    // check all rays in file
    while (ifs >> solution_hit) {
        ifs >> v0.x >> v0.y >> v0.z;
        ifs >> v1.x >> v1.y >> v1.z;
        ifs >> v2.x >> v2.y >> v2.z;
        ifs >> ray.origin.x >> ray.origin.y >> ray.origin.z;
        ifs >> ray.direction.x >> ray.direction.y >> ray.direction.z;
        ifs >> solution_t;
        ifs >> solution_hit_plane;
        ifs >> solution_plane.D;
        ifs >> solution_plane.normal.x >> solution_plane.normal.y >> solution_plane.normal.z;
        ifs >> solution_inside_triangle;
        ifs >> solution_hit_point.x >> solution_hit_point.y >> solution_hit_point.z;

        // student's answer
        ray.t = std::numeric_limits<float>::max();
        bool hit = intersectRayWithPlane(solution_plane, ray);

        CAPTURE(solution_plane.D, solution_plane.normal, ray.origin, ray.direction, ray.t, solution_hit_plane, solution_t, hit, ray.t);
        REQUIRE(solution_hit_plane == hit);
        if (solution_hit_plane) {
            REQUIRE(solution_t == Catch::Approx(ray.t).margin(epsilon));
        }
    }
}

// test a list of rays from a file against a testscene (indicated in file)
// tests only for the point in triangle part of the code
void testPointInTriangle(const std::filesystem::path& testfilePath)
{
    std::ifstream ifs(testfilePath);
    assert(ifs.is_open());

    Ray ray;

    // load scene
    int scene_id;
    ifs >> scene_id;
    const Scene scene = loadScene(static_cast<SceneType>(scene_id), DATA_DIR);

    float solution_t = 0.0f;
    bool solution_hit;
    bool solution_inside_triangle;
    bool solution_hit_plane;
    Plane solution_plane;
    glm::vec3 solution_hit_point;

    auto v0 = scene.meshes[0].vertices[0].position;
    auto v1 = scene.meshes[0].vertices[1].position;
    auto v2 = scene.meshes[0].vertices[2].position;
//        hit |= intersectRayWithTriangle(v0.p, v1.p, v2.p, ray);

    // check all rays in file
    while (ifs >> solution_hit) {
        ifs >> v0.x >> v0.y >> v0.z;
        ifs >> v1.x >> v1.y >> v1.z;
        ifs >> v2.x >> v2.y >> v2.z;
        ifs >> ray.origin.x >> ray.origin.y >> ray.origin.z;
        ifs >> ray.direction.x >> ray.direction.y >> ray.direction.z;
        ifs >> solution_t;
        ifs >> solution_hit_plane;
        ifs >> solution_plane.D;
        ifs >> solution_plane.normal.x >> solution_plane.normal.y >> solution_plane.normal.z;
        ifs >> solution_inside_triangle;
        ifs >> solution_hit_point.x >> solution_hit_point.y >> solution_hit_point.z;

        bool inside = pointInTriangle(v0, v1, v2, solution_plane.normal, solution_hit_point);

        CAPTURE(v0, v1, v2, solution_plane.normal, solution_hit_point, solution_inside_triangle, inside);
        REQUIRE(solution_inside_triangle == inside);
    }
}

bool vec4equal (const glm::vec4 &v1, const glm::vec4 &v2) {
    constexpr float margin = 1e-5f;
    return glm::length(v1 - v2) < margin;
}

bool vec3equal (const glm::vec3 &v1, const glm::vec3 &v2) {
    constexpr float margin = 1e-5f;
    return glm::length(v1 - v2) < margin;
}

bool mazeTileEqual (const std::array<glm::vec3, 4> &q1, const MazeTile &t2) {
    const std::array<glm::vec3, 4> q2 {t2.v1, t2.v2, t2.v3, t2.v4};
    return std::ranges::is_permutation(q1, q2, vec3equal);
}

void testMazeGeneration() {
    std::vector<std::vector<MazeTile>> ref_boards = {
        reference::generateMaze(Game::mazeCenter, Game::maze, 0),
        reference::generateMaze(Game::mazeCenter, Game::maze, 3),
        reference::generateMaze(Game::mazeCenter, Game::maze, 2.7f),
    };

    std::vector<std::vector<MazeTile>> student_boards = {
            generateMaze(Game::mazeCenter, Game::maze, 0),
            generateMaze(Game::mazeCenter, Game::maze, 3),
            generateMaze(Game::mazeCenter, Game::maze, 2.7f),
    };

    for (size_t b=0; b < ref_boards.size(); b++) {
        const auto &ref_board = ref_boards[b];
        const auto &student_board = student_boards[b];

        CAPTURE(ref_board.size(), student_board.size());
        REQUIRE(ref_board.size() == student_board.size());

        int quad_counter = 0;
        int color_counter = 0;
        for (const auto &ref_tile : ref_board) {
            const std::array<glm::vec3, 4> ref_quad {ref_tile.v1, ref_tile.v2, ref_tile.v3, ref_tile.v4};
            auto predicate = [ref_quad] (const MazeTile &student_tile){ return mazeTileEqual(ref_quad, student_tile); };
            auto found_pos = std::ranges::find_if(student_board, predicate);
            if (found_pos != student_board.end()) {
                quad_counter++;
                if (vec4equal(ref_tile.color, found_pos->color)) {
                    color_counter++;
                }
                // Note: Feel free to comment out both of the else blocks if you want to see the entire boards in the console
                else {
                    INFO("Color of the following tile does not match: Reference:" << ref_tile << "Student: " << *found_pos);
                    REQUIRE(false);
                }
            }
            else {
                INFO("Could not find the following tile in your maze: " << ref_tile);
                REQUIRE(false);
            }
        }

        CAPTURE(ref_board, student_board);
        int c = 0;
        for (; c < quad_counter; c++) REQUIRE(true);
        for (; c < ref_board.size(); c++) REQUIRE(false);
        for (c = 0; c < color_counter; c++) REQUIRE(true);
        for(; c < ref_board.size(); c++) REQUIRE(false);
    }
}

struct circleData {
    size_t n_points;
    float radius;
    glm::vec2 center;
};

void testSingleCirclePoints(std::span<const glm::vec2> circle, circleData cd) {
    constexpr float margin = 1e-5f;

    REQUIRE(cd.n_points == circle.size());

    // Check if points are on circle
    bool all_points_on_circle = true;

    for (const glm::vec2 &p : circle) {
        float distance_to_center = glm::length(p - cd.center);
        if (distance_to_center - cd.radius > margin) {
            all_points_on_circle = false;
            break;
        }
    }

    // Check if points are uniformly distributed
    bool all_points_evenly_spaced = true;
    float initial_distance_points = glm::length(circle[0] - circle[1]);
    for (int i = 1; i < cd.n_points; i++) {
        float current_distance_points = glm::length(circle[i] - circle[(i+1) % cd.n_points]);
        if (current_distance_points - initial_distance_points > margin) {
            all_points_evenly_spaced = false;
            break;
        }
    }
    bool all_points_on_circle_and_evenly_spaced = (all_points_on_circle && all_points_evenly_spaced);

    CAPTURE(circle, cd.n_points, cd.radius, cd.center, all_points_on_circle, all_points_evenly_spaced);
    REQUIRE(all_points_on_circle_and_evenly_spaced);
//    return true;
}

void testCircePoints() {
    std::mt19937 engine {12345};
    std::uniform_int_distribution<size_t> dist_npoints {3, 30};
    std::uniform_real_distribution<float> dist_radii {0.3f, 10.0f};
    std::uniform_real_distribution<float> dist_center {-10.0, 10.0};

    for (int i=0; i < 50; i++) {
        const circleData c3 {dist_npoints(engine), dist_radii(engine), {dist_center(engine), dist_center(engine)}};
        testSingleCirclePoints(generateCirclePoints(c3.radius, c3.center, c3.n_points), c3);
    }
}

struct RandomPolygonGenerator{
    std::vector<glm::vec2> generate(size_t N) {
        std::vector<glm::vec2> polygon (N);
        for (size_t i = 0; i < N; i++) {
            polygon[i] = {dist(engine), dist(engine)};
        }
        return polygon;
    }
    glm::vec2 generateInitialPosition() {
        return {dist(engine), dist(engine)};
    }

    std::vector<glm::vec2> generateSequence(size_t sequenceLength) {
        std::vector<glm::vec2> sequence (sequenceLength);
        for (size_t i = 0; i < sequenceLength; i++) {
            switch(dist_sequence(engine)) {
                case 0:
                    sequence[i] = Game::UP;
                    break;
                case 1:
                    sequence[i] = Game::DOWN;
                    break;
                case 2:
                    sequence[i] = Game::LEFT;
                    break;
                default:
                    sequence[i] = Game::RIGHT;
                    break;
            }
        }
        return sequence;
    }

    float generateTime(int sequenceLength, bool discrete=false) {
        float random_t = dist_t(engine) * (float) sequenceLength;
        if (discrete) return std::roundf(random_t);
        return random_t;
    }

    private:
        std::mt19937 engine {12345};
        std::uniform_real_distribution<float> dist {-10.0, 10.0};
        std::uniform_int_distribution<int> dist_sequence {0, 3};
        std::uniform_real_distribution<float> dist_t {0.0f, 1.0f};
    };

void testInitialPositionSinglePolygon(std::vector<glm::vec2> polygon, glm::vec2 initialPosition) {
    auto polygon_reference = reference::applyInitialPosition(polygon, initialPosition);
    auto polygon_student = applyInitialPosition(polygon, initialPosition);

    constexpr float margin = 1e-5f;
    bool all_correctly_displaced = true;
    for (size_t i = 0; i < polygon.size(); i++) {
        if (glm::length(polygon_reference[i] - polygon_student[i]) > margin) {
            all_correctly_displaced = false;
            break;
        }
    }
    CAPTURE(polygon, initialPosition, polygon_student, polygon_reference);
    REQUIRE(all_correctly_displaced);
}

void testMovementSinglePolygon(
        std::span<const glm::vec2> polygon,
        std::span<const glm::vec2> motionSteps,
        const float t) {
    auto polygon_reference = reference::applyMovementInTime(polygon, motionSteps, t);
    auto polygon_student = applyMovementInTime(polygon, motionSteps, t);

    constexpr float margin = 1e-5f;
    bool all_correctly_moved_in_time = true;
    for (size_t i = 0; i < polygon.size(); i++) {
        if (glm::length(polygon_reference[i] - polygon_student[i]) > margin) {
            all_correctly_moved_in_time = false;
            break;
        }
    }

    CAPTURE(polygon, motionSteps, t, polygon_reference, polygon_student);
    REQUIRE(all_correctly_moved_in_time);
}


void testGameEvolution() {
    RandomPolygonGenerator rpg {};

    for (int i = 0; i < 10; i++) {
        testInitialPositionSinglePolygon(rpg.generate(4), rpg.generateInitialPosition());
        testInitialPositionSinglePolygon(rpg.generate(7), rpg.generateInitialPosition());
        testInitialPositionSinglePolygon(rpg.generate(10), rpg.generateInitialPosition());
        testInitialPositionSinglePolygon(rpg.generate(11), rpg.generateInitialPosition());

        testMovementSinglePolygon(rpg.generate(4), rpg.generateSequence(8), rpg.generateTime(8));
        testMovementSinglePolygon(rpg.generate(7), rpg.generateSequence(8), rpg.generateTime(8));
        testMovementSinglePolygon(rpg.generate(10), rpg.generateSequence(8), rpg.generateTime(8));
        testMovementSinglePolygon(rpg.generate(11), rpg.generateSequence(8), rpg.generateTime(8));

        testMovementSinglePolygon(rpg.generate(4), rpg.generateSequence(5), rpg.generateTime(5));
        testMovementSinglePolygon(rpg.generate(7), rpg.generateSequence(5), rpg.generateTime(5));
        testMovementSinglePolygon(rpg.generate(10), rpg.generateSequence(5), rpg.generateTime(5));
        testMovementSinglePolygon(rpg.generate(11), rpg.generateSequence(5), rpg.generateTime(5));

        testMovementSinglePolygon(rpg.generate(4), rpg.generateSequence(8), rpg.generateTime(8, true));
        testMovementSinglePolygon(rpg.generate(7), rpg.generateSequence(8), rpg.generateTime(8, true));
        testMovementSinglePolygon(rpg.generate(10), rpg.generateSequence(8), rpg.generateTime(8, true));
        testMovementSinglePolygon(rpg.generate(11), rpg.generateSequence(8), rpg.generateTime(8, true));

        testMovementSinglePolygon(rpg.generate(4), rpg.generateSequence(5), rpg.generateTime(5, true));
        testMovementSinglePolygon(rpg.generate(7), rpg.generateSequence(5), rpg.generateTime(5, true));
        testMovementSinglePolygon(rpg.generate(10), rpg.generateSequence(5), rpg.generateTime(5, true));
        testMovementSinglePolygon(rpg.generate(11), rpg.generateSequence(5), rpg.generateTime(5, true));
    }
}


using Face = glm::uvec3;
using HullSegment = std::vector<Face>;
using Hull = std::vector<HullSegment>;
using Vertices = std::vector<glm::vec3>;

struct HullTestResult {
    bool correct_n_vertices = true;
    bool correct_n_hull_segments =  true;
    bool correct_n_faces_per_segment = true;

    int correct_hs = 0;
    int total_hs = 0;

    Hull student_hull {};
    Vertices student_vertices {};

    Hull reference_hull {};
    Vertices reference_vertices {};
};

HullTestResult testSingleHull(
        std::span<const glm::vec2> polygon,
        const glm::vec2 initialPosition,
        std::span<const glm::vec2> motionSteps) {

    HullTestResult htr {};

    auto [student_hull, student_vertices] = generateHullGeometry(polygon, initialPosition, motionSteps);
    auto [reference_hull, reference_vertices] = reference::generateHullGeometry(polygon, initialPosition, motionSteps);

    htr.student_vertices = student_vertices;
    htr.student_hull = student_hull;
    htr.reference_vertices = reference_vertices;
    htr.reference_hull = reference_hull;
    htr.total_hs = (int) reference_hull.size();

    constexpr float margin = 1e-5f;

//    CAPTURE((student_hull), (student_vertices), (reference_hull), (reference_vertices));

    // Check all shapes first
    if (student_vertices.size() != reference_vertices.size()) htr.correct_n_vertices = false;
    if (student_hull.size() != reference_hull.size()) htr.correct_n_hull_segments = false;
    for (const HullSegment& hs : student_hull) {
        if (hs.size() != polygon.size() * 2) {
            htr.correct_n_faces_per_segment = false;
            break;
        }
    }

    // Shape correct so we actually have to compare now
    // We told them the hull segments have to be ordered so luckily no invariance there

    for (int i = 0; i < (int) std::min(reference_hull.size(), student_hull.size()); i++) {
        const HullSegment &ref_hs = reference_hull[i];
        const HullSegment &stu_hs = student_hull[i];
        bool segment_valid = true;

        // Dereference all vertices
        std::vector<std::array<glm::vec3, 3>> stu_deref {};
        for (const Face& f : stu_hs) {
            if (f.x >= student_vertices.size() ||
                f.y >= student_vertices.size() ||
                f.z >= student_vertices.size()) continue;
            stu_deref.push_back({student_vertices[f.x], student_vertices[f.y], student_vertices[f.z]});
        }

        // We assume that the two triangles of a quad in the reference are actually adjacent
        // That is not checked, but we know the implementation behaves that way
        // This allows to easily respect the two possible invariants of cutting the quad into triangles for the students
        for (size_t i=0; i < ref_hs.size(); i+=2) {
            // Also assume that there are 2n primitives
            const auto &f1 = ref_hs[i];
            const auto &f2 = ref_hs[i+1];

            // Also assume the order of the vertices in there
            const glm::vec3 &v0 = reference_vertices[f1.x];
            const glm::vec3 &v1 = reference_vertices[f1.y];
            const glm::vec3 &v2 = reference_vertices[f1.z];
            const glm::vec3 &v3 = reference_vertices[f2.y];

            const std::array<std::array<glm::vec3, 3>, 4> ref_triangles {{
                     // Valid triangle combo 1
                     {v0, v1, v2},
                     {v1, v3, v2},

                     // Valid triangle combo 2
                     {v0, v1, v3},
                     {v0, v2, v3}
                }};

            bool combo_found = false;
            // iterate combos
            for (int j=0; j<3; j+=2) {
                bool triangles_found = true;
                // iterate triangles in combo
                for (int k=0; k<2; k++) {
                    const auto ref_tri = ref_triangles[j + k];
                    auto predicate = [ref_tri](const std::array<glm::vec3, 3> &stu_tri){
                        return std::ranges::is_permutation(stu_tri, ref_tri, vec3equal);
                    };

                    auto found = std::ranges::find_if(stu_deref, predicate);
                    if (found == stu_deref.end()) {
                        triangles_found = false;
                        break;
                    }
                }
                if (triangles_found) {
                    combo_found = true;
                    break;
                }
            }

            if (!combo_found) {
                segment_valid = false;
                break;
            }
        }
        if (segment_valid) {
            htr.correct_hs++;
        }
    }
    return htr;
}

void evaluateHullTest(const HullTestResult &hull_test_result) {
    float correct_hull_segments_percentage = (float) hull_test_result.correct_hs / (float) hull_test_result.total_hs;

    bool shapes_correct = (
        hull_test_result.correct_n_vertices &&
        hull_test_result.correct_n_faces_per_segment &&
        hull_test_result.correct_n_hull_segments
        );

    bool hull_completely_correct = (
        shapes_correct &&
        (hull_test_result.correct_hs == hull_test_result.total_hs)
        );

    CAPTURE(hull_test_result.student_hull,
            hull_test_result.student_vertices,
            hull_test_result.reference_hull,
            hull_test_result.reference_vertices,
            hull_test_result.correct_n_vertices,
            hull_test_result.correct_n_hull_segments,
            hull_test_result.correct_n_faces_per_segment,
            correct_hull_segments_percentage);

    REQUIRE(hull_completely_correct);
}

void testHullGeometry() {
    RandomPolygonGenerator rpg {};
    std::vector<glm::vec2> sequence, sequence2, sequence3, sequence4;

    // ****** BASIC (20 * 4 = 80) ******
    for (int i=0; i < 20; i++) {
        // Check Hull generation for length 1 sequence (basic are triangles correct test)
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), rpg.generateSequence(1)));
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), rpg.generateSequence(1)));

        // Check Hull generation for length two with changing direction (no stretch over multiple required, but vertex reuse)
        do sequence = rpg.generateSequence(2); while (sequence[0] == sequence[1]);
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence));
        do sequence = rpg.generateSequence(2); while (sequence[0] == sequence[1]);
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence));
    }

    // ****** LENGTH TWO CONSTANT (20 * 2 = 40) ******
    for (int i=0; i < 20; i++) {
        // Check Hull generation for length two with same direction (basic extends over two with no vertices in between test)
        sequence = rpg.generateSequence(2);
        sequence[1] = sequence[0];
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence));
        sequence = rpg.generateSequence(2);
        sequence[1] = sequence[0];
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence));
    }

    // ****** Combined Test Including Constant Motion Parts (10 * 12 = 120) ******
    for (int i=0; i < 10; i++) {
        // Check Hull generation for length n with start zigzack
        do sequence2 = rpg.generateSequence(7); while (sequence2[0] == sequence2[1]);
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence2));

        do sequence3 = rpg.generateSequence(17); while (sequence3[0] == sequence3[1]);
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence3));

        do sequence4 = rpg.generateSequence(20); while (sequence4[0] == sequence4[1]);
        evaluateHullTest(testSingleHull(rpg.generate(17), rpg.generateInitialPosition(), sequence4));

        // Check Hull generation for length n with end zigzak
        do sequence2 = rpg.generateSequence(7); while (sequence2.rbegin()[0] == sequence2.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence2));

        do sequence3 = rpg.generateSequence(17); while (sequence3.rbegin()[0] == sequence3.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence3));

        do sequence4 = rpg.generateSequence(20); while (sequence4.rbegin()[0] == sequence4.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(17), rpg.generateInitialPosition(), sequence4));

        // Check Hull generation for length n with start straight
        do sequence2 = rpg.generateSequence(7); while (sequence2[0] != sequence2[1]);
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence2));

        do sequence3 = rpg.generateSequence(17); while (sequence3[0] != sequence3[1]);
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence3));

        do sequence4 = rpg.generateSequence(20); while (sequence4[0] != sequence4[1]);
        evaluateHullTest(testSingleHull(rpg.generate(17), rpg.generateInitialPosition(), sequence4));

        // Check Hull generation for length n with end straight
        do sequence2 = rpg.generateSequence(7); while (sequence2.rbegin()[0] != sequence2.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(4), rpg.generateInitialPosition(), sequence2));

        do sequence3 = rpg.generateSequence(17); while (sequence3.rbegin()[0] != sequence3.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(7), rpg.generateInitialPosition(), sequence3));

        do sequence4 = rpg.generateSequence(20); while (sequence4.rbegin()[0] != sequence4.rbegin()[1]);
        evaluateHullTest(testSingleHull(rpg.generate(17), rpg.generateInitialPosition(), sequence4));
    }
}

struct RayTestResult {
    bool correct_n_rays = true;
    int not_directions_normalized_and_nonzero = 0;
    int not_ts_set_to_float_max = 0;
    int not_ray_found = 0;

    std::vector<glm::vec2> polygon {};
    glm::vec2 initialPosition {};
    std::vector<glm::vec2> motionSteps {};
    float t {};
    std::vector<Ray> ref_rays {};
    std::vector<Ray> stu_rays {};
};

RayTestResult testSingleRayBundle (
        std::span<const glm::vec2> polygon,
        const glm::vec2& initialPosition,
        std::span<const glm::vec2> motionSteps,
        const float t) {
    auto ref_rays = reference::generatePacmanRays(polygon, initialPosition, motionSteps, t);
    auto stu_rays = generatePacmanRays(polygon, initialPosition, motionSteps, t);

    constexpr float margin = 1e-5f;

    RayTestResult test_result {};

//    test_result.polygon = polygon;
    test_result.polygon.assign(polygon.begin(), polygon.end());
    test_result.initialPosition = initialPosition;
//    test_result.motionSteps = motionSteps;
    test_result.motionSteps.assign(motionSteps.begin(), motionSteps.end());
    test_result.t = t;
    test_result.ref_rays = ref_rays;
    test_result.stu_rays = stu_rays;
//    CAPTURE(polygon, initialPosition, motionSteps, t, ref_rays, stu_rays);

    if (stu_rays.size() != ref_rays.size()) {
        test_result.correct_n_rays = false;
        CAPTURE(polygon.size());
        REQUIRE(stu_rays.size() == ref_rays.size());
    }

    // Check if all directions normalized and nonzero, and all ts fp max
    for (Ray& r : stu_rays) {
        if (glm::length(r.direction) < margin) {
            test_result.not_directions_normalized_and_nonzero++;
            INFO("Direction vector should indicate some direction (nonzero): " << r);
            REQUIRE(false);
        }
        else if ((glm::length(r.direction) - 1.0f) > margin) {
            test_result.not_directions_normalized_and_nonzero++;
            INFO("Direction vector should be normalized: " << r);
            REQUIRE(false);
            r.direction = r.direction / glm::length(r.direction);
        }
        if (r.t != std::numeric_limits<float>::max()) {
            test_result.not_ts_set_to_float_max++;
            INFO("The t value should be set to float max (default value)");
            REQUIRE(false);
        }
    }

    for (const Ray &ref_ray : ref_rays) {
        auto predicate = [ref_ray] (const Ray & stu_ray) {
            if (glm::length(stu_ray.direction - ref_ray.direction) > margin) return false;
            if (glm::length(stu_ray.origin - ref_ray.origin) > margin) return false;
            return true;
        };

        auto found = std::ranges::find_if(stu_rays, predicate);
        if (found == stu_rays.end()) {
            test_result.not_ray_found++;
            INFO("Could not find reference ray in student rays: Reference " << ref_ray);
            REQUIRE(false);
        }
    }

    return test_result;
}

void check_ray_result(int n_rays, const RayTestResult &test_result) {
    float percentage_incorrect_rays = (float) test_result.not_ray_found / (float) n_rays;

    CAPTURE(test_result.polygon,
            test_result.initialPosition,
            test_result.motionSteps,
            test_result.t,
            test_result.ref_rays,
            test_result.stu_rays,
            test_result.correct_n_rays,
            test_result.not_directions_normalized_and_nonzero,
            test_result.not_ts_set_to_float_max,
            percentage_incorrect_rays);

    REQUIRE((test_result.correct_n_rays &&
          test_result.not_directions_normalized_and_nonzero == 0 &&
          test_result.not_ts_set_to_float_max == 0));

    REQUIRE(test_result.not_ray_found == 0);
}

struct IntersectionGhostTestResult{
    int incorrect_intersection = 0;
    int incorrect_t = 0;

    std::vector<Hull> ghostHulls {};
    std::vector<Vertices> ghostVertices {};

    std::vector<Ray> ref_rays {};
    std::vector<Ray> stu_rays {};
};

IntersectionGhostTestResult testIntersectRaysWithGhosts(
        std::span<const glm::vec2> polygonPacman,
        std::span<const glm::vec2> polygonGhost1,
        std::span<const glm::vec2> polygonGhost2,
        const glm::vec2& initialPositionPacman,
        const glm::vec2& initialPositionGhost1,
        const glm::vec2& initialPositionGhost2,
        std::span<const glm::vec2> motionStepsPacman,
        std::span<const glm::vec2> motionStepsGhost1,
        std::span<const glm::vec2> motionStepsGhost2,
        const float t) {

    IntersectionGhostTestResult test_result {};

    const auto [reference_hull1, reference_vertices1] = reference::generateHullGeometry(polygonGhost1, initialPositionGhost1, motionStepsGhost1);
    const auto [reference_hull2, reference_vertices2] = reference::generateHullGeometry(polygonGhost2, initialPositionGhost2, motionStepsGhost2);
    std::vector<Hull> ghost_hulls {reference_hull1, reference_hull2};
    std::vector<Vertices> ghost_vertices {reference_vertices1, reference_vertices2};

    auto ref_rays = reference::generatePacmanRays(polygonPacman, initialPositionPacman, motionStepsPacman, t);
    auto stu_rays = ref_rays;

    test_result.ghostHulls = ghost_hulls;
    test_result.ghostVertices = ghost_vertices;
    test_result.ref_rays = ref_rays;

    constexpr float margin = 1e-5f;

    for (size_t i=0; i < ref_rays.size(); i++) {
        auto &ref_ray = ref_rays[i];
        auto &stu_ray = stu_rays[i];
        bool ref_result = reference::intersectRayWithGhosts(ghost_vertices, ghost_hulls, ref_ray);
        bool stu_result = intersectRayWithGhosts(ghost_vertices, ghost_hulls, stu_ray);
        if (stu_result != ref_result) {
            test_result.incorrect_intersection++;
            CAPTURE(ref_ray, stu_ray, ghost_hulls, ghost_vertices);
            REQUIRE(stu_result == ref_result);
        }
        if ((stu_ray.t - ref_ray.t) > margin) {
            test_result.incorrect_t++;
            CAPTURE(ref_ray, stu_ray, ghost_hulls, ghost_vertices);
            REQUIRE((stu_ray.t - ref_ray.t) < margin);
        }
    }

    test_result.stu_rays = stu_rays;

    return test_result;
}

void check_hull_intersect_result(int n_rays, const IntersectionGhostTestResult &test_result) {
    float incorrect_intersections = (float) test_result.incorrect_intersection / (float) n_rays;
    float incorrect_t = (float) test_result.incorrect_t / (float) n_rays;

    CAPTURE(test_result.ghostHulls, test_result.ghostVertices, test_result.stu_rays, test_result.ref_rays, incorrect_intersections, incorrect_t);
    REQUIRE(test_result.incorrect_intersection == 0);
    REQUIRE(test_result.incorrect_t == 0);
}

void testRayGeneration() {
    // Test Ray Generation
    RandomPolygonGenerator rpg;

    for (int i = 0; i < 10; i++) {
        check_ray_result(7, testSingleRayBundle(rpg.generate(7), rpg.generateInitialPosition(),
                                                rpg.generateSequence(5), rpg.generateTime(5)));
        check_ray_result(10, testSingleRayBundle(rpg.generate(10), rpg.generateInitialPosition(),
                                                 rpg.generateSequence(7), rpg.generateTime(7, true)));
        check_ray_result(15, testSingleRayBundle(rpg.generate(15), rpg.generateInitialPosition(),
                                                 rpg.generateSequence(9), rpg.generateTime(9, true)));
        check_ray_result(20, testSingleRayBundle(rpg.generate(20), rpg.generateInitialPosition(),
                                                 rpg.generateSequence(11), rpg.generateTime(11)));
    }
}

void testCollisionDetection() {
    RandomPolygonGenerator rpg;

    for(int i = 0; i < 10; i++) {
        check_hull_intersect_result(3, testIntersectRaysWithGhosts(
                rpg.generate(3), rpg.generate(3), rpg.generate(3),
                rpg.generateInitialPosition(), rpg.generateInitialPosition(), rpg.generateInitialPosition(),
                rpg.generateSequence(4), rpg.generateSequence(4), rpg.generateSequence(4),
                rpg.generateTime(4)));

        check_hull_intersect_result(7, testIntersectRaysWithGhosts(
                rpg.generate(7), rpg.generate(7), rpg.generate(7),
                rpg.generateInitialPosition(), rpg.generateInitialPosition(), rpg.generateInitialPosition(),
                rpg.generateSequence(5), rpg.generateSequence(5), rpg.generateSequence(5),
                rpg.generateTime(5)));

        check_hull_intersect_result(10, testIntersectRaysWithGhosts(
                rpg.generate(10), rpg.generate(10), rpg.generate(10),
                rpg.generateInitialPosition(), rpg.generateInitialPosition(), rpg.generateInitialPosition(),
                rpg.generateSequence(7), rpg.generateSequence(7), rpg.generateSequence(7),
                rpg.generateTime(7)));

        check_hull_intersect_result(15, testIntersectRaysWithGhosts(
                rpg.generate(15), rpg.generate(15), rpg.generate(15),
                rpg.generateInitialPosition(), rpg.generateInitialPosition(), rpg.generateInitialPosition(),
                rpg.generateSequence(11), rpg.generateSequence(11), rpg.generateSequence(11),
                rpg.generateTime(11)));

        check_hull_intersect_result(20, testIntersectRaysWithGhosts(
                rpg.generate(20), rpg.generate(20), rpg.generate(20),
                rpg.generateInitialPosition(), rpg.generateInitialPosition(), rpg.generateInitialPosition(),
                rpg.generateSequence(10), rpg.generateSequence(10), rpg.generateSequence(10),
                rpg.generateTime(10)));
    }
}

TEST_CASE("Generate and Draw Maze") {
    testMazeGeneration();
}


TEST_CASE("Geometric Approximation") {
    testCircePoints();
}

TEST_CASE("Game Evolution") {
    testGameEvolution();
}

TEST_CASE("Motion Behavior (Hull Geometry)") {
    testHullGeometry();
}

TEST_CASE("Triangle Plane Creation Test")
{
    testPlaneCreation(std::filesystem::path(TESTS_DIR) / "test-triangle");
}

TEST_CASE("Triangle Plane Intersection Test")
{
    testPlaneIntersection(std::filesystem::path(TESTS_DIR) / "test-triangle");
}

TEST_CASE("Point in Triangle Test")
{
    testPointInTriangle(std::filesystem::path(TESTS_DIR) / "test-triangle");
}

TEST_CASE("Complete Triangle Intersection")
{
    testSingleTriangleIntersections(std::filesystem::path(TESTS_DIR) / "test-triangle");
}

TEST_CASE("Cube Intersection Test")
{
    testIntersections(std::filesystem::path(TESTS_DIR) / "test-cube");
}

TEST_CASE("Ray Generation")
{
    testRayGeneration();
}

TEST_CASE("Collision Detection (Hull Intersection) Test")
{
    testCollisionDetection();
}
