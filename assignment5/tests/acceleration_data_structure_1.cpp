#include <algorithm>
#include <list>
#include <vector>
#include <array>
#include <span>
#include <random>

#include "../src/bvh.h" // Include the student's BVH
#include "util.h"
#include "bvh_ref.h"
#include "../src/intersect.h"
#include <numeric>
#include <iostream>
#include "framework/glm_ostream.h"

std::ostream &operator<<(std::ostream &stream, const AxisAlignedBox &aabb) {
    stream << "AxisAlignedBox {";
    stream << "lower: " << aabb.lower;
    stream << ", upper: " << aabb.upper;
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const Vertex &vertex) {
    stream << "Vertex {";
    stream << "position: " << vertex.position;
    stream << " [...] }";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface::Primitive &prim) {
    stream << "Primitive {";
    stream << "meshID: " << prim.meshID;
    stream << ", v1: " << prim.v1;
    stream << ", v2: " << prim.v1;
    stream << ", v3: " << prim.v1;
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface::Node &node) {
    stream << "Node {";
    stream << "leftChild: " << node.leftChild();
    stream << ", rightChild: " << node.rightChild();
    stream << "}";
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const BVHInterface &bvh) {
    stream << "BVH {";
    stream << "Nodes: {";
    for (const auto &node : bvh.nodes()) {
        stream << node;
        stream << ",";
    }
    stream << "} [...] }";
    return stream;
}

namespace test {

// Test settings
constexpr uint32_t num_samples = 16; // Number of random samples taken in certain tests
constexpr uint32_t allowed_time_multiplier = 10; // >= 10x reference intersect() time throws a warning for now

std::tuple<ref::Sampler, Scene, ref::BVH, BVHInterface::Primitive, BVHInterface::Primitive>
getPreambel() {
    // Instantiate reference objects
    static ref::Sampler sampler(4);

    // Load test scene
    static Scene scene = loadScenePrebuilt(SceneType::Teapot, DATA_DIR);

    // Instantiate reference bvh, not student's bvh
    static ref::BVH refr_bvh(scene);

    // Instantiate collapsed primitive, should be handled correctly either way
    static BVHInterface::Primitive good_prim = refr_bvh.primitives()[0];
    static BVHInterface::Primitive empty_prim = {
            .meshID = 16,
            .v0 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)},
            .v1 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)},
            .v2 = Vertex{.position = glm::vec3(32), .normal = glm::vec3(0, 1, 0), .texCoord = glm::vec3(0)}
    };

    return {sampler, scene, refr_bvh, good_prim, empty_prim};
}

TEST_CASE("BVH Compute Bounding Boxes: computePrimitiveAABB")
{
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    SECTION("")
    {
        BVHInterface::Primitive prim = refr_bvh.primitives()[0];
        auto refr = ref::computePrimitiveAABB(prim);
        auto stud = ::computePrimitiveAABB(prim);

        // Simply test comparison on a regular primitive
        // We expect a regular AABB equal to the reference implementation
        CAPTURE(prim, refr, stud);
        REQUIRE((epsEqual(refr, stud) && !epsEqual(stud.lower, stud.upper)));
    }
};

TEST_CASE("BVH Compute Bounding Boxes: computeSpanAABB")
{
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    SECTION("")
    {
        auto prims = refr_bvh.primitives();
        std::vector<BVHInterface::Primitive> shuffle(prims.begin(), prims.end());
        rng::shuffle(shuffle.begin(), shuffle.end(), sampler);

        // Comparative reference
        auto refr = ref::computeSpanAABB(prims);

        // Simply test comparison on the full set of primitives
        // We expect a regular AABB equal to the reference implementation
        auto stud = ::computeSpanAABB(prims);
        INFO("Testing on all primitives of teapot scene");
        CAPTURE(stud, refr);
        REQUIRE((epsEqual(refr, stud) && !epsEqual(stud.lower, stud.upper)));

        // Rerun test on shuffled set of primitives
        // We expect the same results
        stud = ::computeSpanAABB(shuffle);
        CAPTURE(stud, refr);
        REQUIRE((epsEqual(refr, stud) && !epsEqual(stud.lower, stud.upper)));
    }
};

TEST_CASE("BVH Bounding Volume Split: computePrimitiveCentroid") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    SECTION("")
    {
        // Simply test comparison between centroids
        // We expect the average of the vertices, as per the reference implementation
        auto ref_good = ref::computePrimitiveCentroid(good_prim);
        auto stud_good = ::computePrimitiveCentroid(good_prim);
        CAPTURE(good_prim, ref_good, stud_good);
        REQUIRE(epsEqual(ref_good, stud_good));

        // Test comparison on a collapsed centroid
        // We expect the centroid to be equal to all vertices, as per reference implementation
        auto ref_empty = ref::computePrimitiveCentroid(empty_prim);
        auto stud_empty = ::computePrimitiveCentroid(empty_prim);
        CAPTURE(empty_prim, ref_empty, stud_empty);
        REQUIRE(epsEqual(ref_empty, stud_empty));
    }
};

TEST_CASE("BVH Bounding Volume Split: computeAABBLongestAxis") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    // Test longest axis in the general case, where there
        // is a well-defined longest axis
    SECTION("computeAABBLongestAxis [Axes are randomized, should return longest]")
    {
        auto aabb = ref::computeSpanAABB(refr_bvh.primitives());
        auto refr = ref::computeAABBLongestAxis(aabb);
        auto stud = ::computeAABBLongestAxis(aabb);

        // Then compare output of both implementations
        CAPTURE(aabb, refr, stud);
        REQUIRE(refr == stud);
    }

        // Then, in the case x, y and z are all longest axes, test that x is returned
    SECTION("computeAABBLongestAxis [All axes are equal, should return first]")
    {
        AxisAlignedBox aabb = {.lower = glm::vec3(0), .upper = glm::vec3(3.5)};
        CAPTURE(aabb);
        REQUIRE(::computeAABBLongestAxis(aabb) == 0);
    }

        // Then, in the case y and z are both longest axes, test that y is returned
    SECTION("computeAABBLongestAxis [First axis is shortest, should return second]")
    {
        AxisAlignedBox aabb = {.lower = glm::vec3(0), .upper = glm::vec3(1.5, 2.3, 2.3)};
        CAPTURE(aabb);
        REQUIRE(::computeAABBLongestAxis(aabb) == 1);
    }

        // Finally, in a collapsed case, test that x is simply returned
    SECTION("computeAABBLongestAxis [All axes are collapsed, should return first]")
    {
        AxisAlignedBox aabb = {.lower = glm::vec3(16), .upper = glm::vec3(16)};
        CAPTURE(aabb);
        REQUIRE(::computeAABBLongestAxis(aabb) == 0);
    }

};

TEST_CASE("BVH Bounding Volume Split: splitPrimitivesByMedian") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();

    // splitPrimitivesByMedian
    // NOTE: No empty case is tested, as the minimum number of nodes bottoms out at 4
    // as explicitly stated in the assignment
    using Primitive = BVHInterface::Primitive;

    // Construct a large bucket of primitives from the scene for use during tests
    std::vector<Primitive> prim_base;
    {
        // Count the total nr. of triangles in the scene
        size_t numTriangles = 0;
        for (const auto &mesh: scene.meshes)
            numTriangles += mesh.triangles.size();
        prim_base.reserve(numTriangles);

        // Pass in all triangles
        for (uint32_t meshID = 0; meshID < scene.meshes.size(); meshID++) {
            const auto &mesh = scene.meshes[meshID];
            for (const auto &triangle: mesh.triangles) {
                prim_base.push_back(Primitive{
                        .meshID = meshID,
                        .v0 = mesh.vertices[triangle.x],
                        .v1 = mesh.vertices[triangle.y],
                        .v2 = mesh.vertices[triangle.z]});
            }
        }
    }

    // General test; split a large set of primitives. Independent of the split position,
    // we expect all primitives to be sorted along their centroids on the specified axis
    SECTION("splitPrimitivesByMedian [Primitives are partially sorted around split position]")
    {
        AxisAlignedBox aabb = ref::computeSpanAABB(prim_base);

        // Test check values
        bool is_stud_sort_correct_all = true;

        // Test all three axes
        for (uint32_t axis = 0; axis < 3; ++axis) {
            // Generate copies of unsorted primitive spans
            auto prim_refr = prim_base, prim_stud = prim_base;

            // Sort both unsorted spans over the axis
            // Return split position as signed integer for difference test
            auto split_refr = static_cast<int>(ref::splitPrimitivesByMedian(aabb, axis, prim_refr));
            auto split_stud = static_cast<int>(::splitPrimitivesByMedian(aabb, axis, prim_stud));

            // Check that both subspans are equivalent sets, independent of their actual sorting
            // (as some students may do a partial sort using e.g. std::nth_element)
            auto vws_centroid =
                    [axis](const BVHInterface::Primitive &prim) {
                        return ref::computePrimitiveCentroid(prim)[axis];
                    };
//                auto sub0_refr = prim_refr | vws::take(split_refr) | vws::transform(vws_centroid) | view_to<std::vector>();
//                auto sub1_refr = prim_refr | vws::drop(split_refr) | vws::transform(vws_centroid) | view_to<std::vector>();
//                auto sub0_stud = prim_stud | vws::take(split_stud) | vws::transform(vws_centroid) | view_to<std::vector>();
//                auto sub1_stud = prim_stud | vws::drop(split_stud) | vws::transform(vws_centroid) | view_to<std::vector>();

            auto sub0_refr_ = prim_refr | vws::take(split_refr) | vws::transform(vws_centroid);
            auto sub1_refr_ = prim_refr | vws::drop(split_refr) | vws::transform(vws_centroid);
            auto sub0_stud_ = prim_stud | vws::take(split_stud) | vws::transform(vws_centroid);
            auto sub1_stud_ = prim_stud | vws::drop(split_stud) | vws::transform(vws_centroid);
            std::vector sub0_refr = std::ranges::to<std::vector>(sub0_refr_);
            std::vector sub1_refr = std::ranges::to<std::vector>(sub1_refr_);
            std::vector sub0_stud = std::ranges::to<std::vector>(sub0_stud_);
            std::vector sub1_stud = std::ranges::to<std::vector>(sub1_stud_);

            rng::sort(sub0_refr);
            rng::sort(sub1_refr);
            rng::sort(sub0_stud);
            rng::sort(sub1_stud);
            std::vector<float> sub0_diff, sub1_diff;
            rng::set_difference(sub0_refr, sub0_stud, std::back_inserter(sub0_diff));
            rng::set_difference(sub1_refr, sub1_stud, std::back_inserter(sub1_diff));

            bool is_stud_sort_correct = sub0_diff.size() <= 1 && sub1_diff.size() <= 1;
            if (sub0_diff.size() >= 2) {
                auto elem_on_other_side_1 = sub0_diff[0];
                auto elem_on_other_side_2 = sub0_diff[1];
                CAPTURE(axis, elem_on_other_side_1, elem_on_other_side_2, split_refr, split_stud);
                REQUIRE(is_stud_sort_correct);
            }
            if (sub1_diff.size() >= 2) {
                auto elem_on_other_side_1 = sub1_diff[0];
                auto elem_on_other_side_2 = sub1_diff[1];
                CAPTURE(axis, elem_on_other_side_1, elem_on_other_side_2, split_refr, split_stud);
                REQUIRE(is_stud_sort_correct);
            }
            is_stud_sort_correct_all &= is_stud_sort_correct;
        } // for (axis)

        // Verify student solution
        CAPTURE(aabb, prim_base);
        REQUIRE(is_stud_sort_correct_all); // Is the sort performed along the right axis?
    }

    // Even test; split an even nr. of primitives and ensure correctness
    SECTION("splitPrimitivesByMedian [Split position is correct for even nr. of primitives]")
    {
        // Generate an even nr. of primitives s.t. the split should be off
        std::vector<Primitive> prim_even = {prim_base[0], prim_base[3], prim_base[7], prim_base[11]};
        AxisAlignedBox aabb = ref::computeSpanAABB(prim_even);

        // Test check values
        bool is_stud_posi_correct_all = true;

        // Test all three axes
        for (uint32_t axis = 0; axis < 3; ++axis) {
            // Generate copies of unsorted primitive spans
            auto prim_refr = prim_even, prim_stud = prim_even;

            // Sort both unsorted spans over the axis
            // Return split position as signed integer for difference test
            auto split_refr = static_cast<int>(ref::splitPrimitivesByMedian(aabb, axis, prim_refr));
            auto split_stud = static_cast<int>(::splitPrimitivesByMedian(aabb, axis, prim_stud));

            // Check that difference between both split positions is at most 1
            bool is_stud_posi_correct = std::abs(split_refr - split_stud) <= 1;
            is_stud_posi_correct_all &= is_stud_posi_correct;
            CAPTURE(split_refr, split_stud, aabb, prim_even);
            REQUIRE(is_stud_posi_correct);
        } // for (axis)

        // Verify student solution
        CAPTURE(aabb, prim_even);
        REQUIRE(is_stud_posi_correct_all); // Is the split position on the median?
    }

    // Even test; split an uneven nr. of primitives and ensure correctness
    SECTION("splitPrimitivesByMedian [Split position is correct for uneven nr. of primitives]")
    {
        std::vector<Primitive> prim_uneven = {prim_base[13], prim_base[12], prim_base[15], prim_base[14],
                                              prim_base[16]};
        AxisAlignedBox aabb = ref::computeSpanAABB(prim_uneven);

        // Test check values
        bool is_stud_posi_correct_all = true;

        // Test all three axes
        for (uint32_t axis = 0; axis < 3; ++axis) {
            // Generate copies of unsorted primitive spans
            auto prim_refr = prim_uneven, prim_stud = prim_uneven;

            // Sort both unsorted spans over the axis
            // Return split position as signed integer for difference test
            auto split_refr = static_cast<int>(ref::splitPrimitivesByMedian(aabb, axis, prim_refr));
            auto split_stud = static_cast<int>(::splitPrimitivesByMedian(aabb, axis, prim_stud));

            // Check that difference between both split positions is at most 1
            bool is_stud_posi_correct = (std::abs(split_refr - split_stud) <= 1);
            is_stud_posi_correct_all &= is_stud_posi_correct;
            CAPTURE(split_refr, split_stud, aabb, prim_uneven);
            REQUIRE(is_stud_posi_correct);
        } // for (axis)

        // Verify student solution
        CAPTURE(aabb, prim_uneven);
        REQUIRE(is_stud_posi_correct_all); // Is the split position on the median?
    }
}

TEST_CASE("BVH Recursive Construction: Node Data") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();
    // The following gets a bit tricky, as it is essentially an "end-to-end" test
    // of the BVH constructor.
    // We simply confirm that no nodes exist which could break traversal badly.
    SECTION("")
    {
        // Generate the student's BVH implementation
        ::BVH stud_bvh(scene);

        // Gather a filtered view over non-leaf nodes
        auto nodes = stud_bvh.nodes()
            | vws::filter([](const auto& node) { return !node.isLeaf(); });

        // Test all node left/right child indices to see if their values are out of bounds,
        // or if left/right children are the same (or both zeroes)
        bool is_stud_out_of_bounds = false;
        bool is_stud_all_equal = false;
        auto bounds = stud_bvh.nodes().size();
        for (const auto& node : nodes) {
            if (node.leftChild() == node.rightChild() && node.leftChild() != 0) {
                is_stud_all_equal = true;
                CAPTURE(node);
                REQUIRE_FALSE(is_stud_all_equal);
                break;
            }

            if (node.leftChild() >= bounds || node.rightChild() >= bounds) {
                is_stud_out_of_bounds = true;
                CAPTURE(node, bounds);
                REQUIRE_FALSE(is_stud_out_of_bounds);
                break;
            }
        }

        REQUIRE_FALSE(is_stud_out_of_bounds);
        REQUIRE_FALSE(is_stud_all_equal);
    }

}

TEST_CASE("BVH Recursive Construction: Leaf Data") {
    auto [sampler, scene, refr_bvh, good_prim, empty_prim] = getPreambel();
    // The following get a bit tricky, as it is essentially an "end-to-end" test
    // of the BVH constructor.
    // We simply confirm that leaf nodes adhere to the restrictions we set in
    // the assignment:
    // - leaf nodes contain at most 4 primitives
    // - given a good BVH structure, not all leaf nodes contain 1 primitive
    // - all leaf nodes together allow you to recover all primitives
    SECTION("")
    {
        // Generate the student's BVH implementation
        ::BVH stud_bvh(scene);

        // Gather a filtered view over non-empty leaf nodes
        auto leaves_ = stud_bvh.nodes() | vws::filter([](const auto& node) { return node.isLeaf(); });
        std::vector leaves = std::ranges::to<std::vector>(leaves_);

        // Test that the nr. of leaf nodes does not equal or exceed the nr. of primitives
        CAPTURE(leaves.size(), refr_bvh.primitives().size());
        REQUIRE((!leaves.empty() && leaves.size() < refr_bvh.primitives().size()));

        // Sum together all leaf primitive counts, and confirm this equals the nr. of primitives
        auto prim_count = rng::fold_left_first(leaves
            | vws::transform([](const auto& node) { return node.primitiveCount(); }), std::plus {});
        uint32_t prim_count_v = prim_count.has_value() ? prim_count.value() : 0;
        CAPTURE(prim_count_v, refr_bvh.primitives().size());
        REQUIRE(prim_count_v == refr_bvh.primitives().size());

        // Test that no leaf-node holds more than 4 primitives
        auto over_4_leaves_ = leaves | vws::filter([](const auto& node) { return node.primitiveCount() > 4; });
        std::vector over_4_leaves = std::ranges::to<std::vector>(over_4_leaves_);
        if (!over_4_leaves.empty()) {
            CAPTURE(over_4_leaves[0]);
            REQUIRE(over_4_leaves.empty());
        }
    }
}


} // namespace test