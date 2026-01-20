#include "bvh.h"
#include "bvh_interface.h"
#include "common.h"
#include "draw.h"
#include "intersect.h"
#include "scene.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <framework/opengl_includes.h>
#include <iostream>

#include <functional>
#include <glm/gtx/component_wise.hpp>
#include <limits>
#include <numeric>
#include <type_traits>


// Helper method to fill in hitData object.
// You should not have to touch this, just call it in the traversal method.
void updateHitInfo(const Scene& scene, const BVHInterface::Primitive& primitive, const Ray& ray, HitData& hitInfo)
{
    const auto& [v0, v1, v2] = std::tie(primitive.v0, primitive.v1, primitive.v2);
    const auto& mesh = scene.meshes[primitive.meshID];
    const auto n = glm::normalize(glm::cross(v1.position - v0.position, v2.position - v0.position));

    hitInfo.material = mesh.material;
    hitInfo.normal = n;

    // Catch flipped normals
    if (glm::dot(ray.direction, n) > 0.0f) {
        hitInfo.normal = -hitInfo.normal;
    }
}

// BVH constructor; can be safely ignored. You should not have to touch this
// NOTE: this constructor is tested, so do not change the function signature.
BVH::BVH(const Scene& scene)
{
#ifndef NDEBUG
    // Store start of bvh build for timing
    using clock = std::chrono::high_resolution_clock;
    const auto start = clock::now();
#endif

    // Count the total nr. of triangles in the scene
    size_t numTriangles = 0;
    for (const auto& mesh : scene.meshes)
        numTriangles += mesh.triangles.size();

    // Given the input scene, gather all triangles over which to build the BVH as a list of Primitives
    std::vector<Primitive> primitives;
    primitives.reserve(numTriangles);
    for (uint32_t meshID = 0; meshID < scene.meshes.size(); meshID++) {
        const auto& mesh = scene.meshes[meshID];
        for (const auto& triangle : mesh.triangles) {
            primitives.push_back(Primitive {
                .meshID = meshID,
                .v0 = mesh.vertices[triangle.x],
                .v1 = mesh.vertices[triangle.y],
                .v2 = mesh.vertices[triangle.z] });
        }
    }

    // Tell underlying vectors how large they should approximately be
    m_primitives.reserve(numTriangles);
    m_nodes.reserve(numTriangles + 1);

    // Recursively build BVH structure; this is where your implementation comes in
    m_nodes.emplace_back(); // Create root node
    m_nodes.emplace_back(); // Create dummy node s.t. children are allocated on the same cache line
    buildRecursive(scene, primitives, RootIndex);

    // Fill in boilerplate data
    buildNumLevels();
    buildNumLeaves();

#ifndef NDEBUG
    // Output end of bvh build for timing
    const auto end = clock::now();
    std::cout << "BVH construction time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms" << std::endl;
#endif
}

// BVH helper method; allocates a new node and returns its index
// You should not have to touch this
uint32_t BVH::nextNodeIdx()
{
    const auto idx = static_cast<uint32_t>(m_nodes.size());
    m_nodes.emplace_back();
    return idx;
}

// TODO: Compute Bounding Boxes
// Given a BVH triangle, compute an axis-aligned bounding box around the primitive
// - primitive; a single triangle to be stored in the BVH
// - return;    an axis-aligned bounding box around the triangle
// This method is unit-tested, so do not change the function signature.
AxisAlignedBox computePrimitiveAABB(const BVHInterface::Primitive primitive)
{
    glm::vec3 p0 = primitive.v0.position;
    glm::vec3 p1 = primitive.v1.position;
    glm::vec3 p2 = primitive.v2.position;

    AxisAlignedBox aabb = {
        .lower = glm::min(p0, glm::min(p1, p2)),
        .upper = glm::max(p0, glm::max(p1, p2))
    };
    return aabb;
}

// TODO: Compute Bounding Boxes
// Given a range of BVH triangles, compute an axis-aligned bounding box around the range.
// - primitives; a contiguous range of triangles to be stored in the BVH
// - return;    a single axis-aligned bounding box around the entire set of triangles
// This method is unit-tested, so do not change the function signature.
AxisAlignedBox computeSpanAABB(std::span<const BVHInterface::Primitive> primitives)
{
    BVHInterface::Primitive p = primitives[0];
    glm::vec3 p0 = p.v0.position;
    glm::vec3 p1 = p.v1.position;
    glm::vec3 p2 = p.v2.position;

    AxisAlignedBox aabb = {
        .lower = glm::min(p0,glm::min(p1,p2)),
        .upper = glm::max(p0,glm::max(p1,p2))
    };

    for (size_t i = 1; i < primitives.size(); i++) {
        p = primitives[i];
        aabb.lower = glm::min(aabb.lower,p.v0.position);
        aabb.lower = glm::min(aabb.lower,p.v1.position);
        aabb.lower = glm::min(aabb.lower, p.v2.position);

        aabb.upper = glm::max(aabb.upper, p.v0.position);
        aabb.upper = glm::max(aabb.upper, p.v1.position);
        aabb.upper = glm::max(aabb.upper, p.v2.position);
    }

    return aabb;
}

// TODO: Bounding Volume Split
// Given a BVH triangle, compute the geometric centroid of the triangle
// - primitive; a single triangle to be stored in the BVH
// - return;    the geometric centroid of the triangle's vertices
// This method is unit-tested, so do not change the function signature.
glm::vec3 computePrimitiveCentroid(const BVHInterface::Primitive primitive)
{
    return (primitive.v0.position + primitive.v1.position + primitive.v2.position) / 3.0f;
}

// TODO: Bounding Volume Split
// Given an axis-aligned bounding box, compute the longest axis; x = 0, y = 1, z = 2.
// - aabb;   the input axis-aligned bounding box
// - return; 0 for the x-axis, 1 for the y-axis, 2 for the z-axis
//           if several axes are equal in length, simply return the first of these
// This method is unit-tested, so do not change the function signature.
uint32_t computeAABBLongestAxis(const AxisAlignedBox& aabb)
{
    float y = std::abs(aabb.upper.y - aabb.lower.y);
    float x = std::abs(aabb.upper.x - aabb.lower.x);
    float z = std::abs(aabb.upper.z - aabb.lower.z);

    return x >= y ? (x >= z ? 0 : 2) : (y >= z ? 1 : 2);
}

// TODO: Bounding Volume Split
// Given a range of BVH triangles, sort these along a specified axis based on their geometric centroid.
// Then, find and return the split index in the range, such that the subrange containing the first element 
// of the list is at least as big as the other, and both differ at most by one element in size.
// Hint: you should probably reuse `computePrimitiveCentroid()`
// - aabb;       the axis-aligned bounding box around the given triangle range
// - axis;       0, 1, or 2, determining on which axis (x, y, or z) the split must happen
// - primitives; the modifiable range of triangles that requires sorting/splitting along an axis
// - return;     the split position of the modified range of triangles
// This method is unit-tested, so do not change the function signature.
size_t splitPrimitivesByMedian(const AxisAlignedBox& aabb, uint32_t axis, std::span<BVHInterface::Primitive> primitives)
{
    using Primitive = BVHInterface::Primitive;

    std::sort(primitives.begin(), primitives.end(), 
    [&] (const Primitive& a, const Primitive& b) {
        return computePrimitiveCentroid(a)[axis] <  computePrimitiveCentroid(b)[axis];
    });

    size_t n = primitives.size();
    return (n + 1) / 2;
}

// TODO: Recursive BVH Construction
// Leaf construction routine; you should reuse this in `buildRecursive()`
// Given an axis-aligned bounding box, and a range of triangles, generate a valid leaf object
// and store the triangles in the `m_primitives` vector.
// - scene;      the active scene
// - aabb;       the axis-aligned bounding box around the primitives beneath this leaf
// - primitives; the range of triangles to be stored for this leaf
BVH::Node BVH::buildLeafData(const Scene& scene, const AxisAlignedBox& aabb, std::span<Primitive> primitives)
{
    Node node;
    // TODO fill in the leaf's data; refer to `bvh_interface.h` for details
    node.aabb = aabb;
    node.data[0] = BVHInterface::Node::LeafBit | (uint32_t)m_primitives.size();
    node.data[1] = (uint32_t) primitives.size();

    // Copy the current set of primitives to the back of the primitives vector
    std::copy(primitives.begin(), primitives.end(), std::back_inserter(m_primitives));

    return node;
}

// TODO: Recursive BVH Construction
// Node construction routine; you should reuse this in in `buildRecursive()`
// Given an axis-aligned bounding box, and left/right child indices, generate a valid node object.
// - scene;           the active scene
// - aabb;            the axis-aligned bounding box around the primitives beneath this node
// - leftChildIndex;  the index of the node's left child in `m_nodes`
// - rightChildIndex; the index of the node's right child in `m_nodes`
BVH::Node BVH::buildNodeData(const Scene& scene, const AxisAlignedBox& aabb, uint32_t leftChildIndex, uint32_t rightChildIndex)
{
    Node node;
    // TODO fill in the node's data; refer to `bvh_interface.h` for details
    node.aabb = aabb;
    node.data[0] = ~(BVHInterface::Node::LeafBit) & leftChildIndex;
    node.data[1] = rightChildIndex;
    return node;
}

// TODO: Recursive BVH Construction
// Hierarchy construction routine; called by the BVH's constructor,
// you must implement this method and implement it carefully!
//
// You should implement the other BVH methods first, and this one last, as you can reuse
// most of the other methods to assemble this part. There are detailed instructions inside the
// method which we recommend you follow.
//
// Arguments:
// - scene;      the active scene
// - primitives; a range of triangles to be stored in the BVH
// - nodeIndex;  index of the node you are currently working on, this is already allocated
void BVH::buildRecursive(const Scene& scene, std::span<Primitive> primitives, uint32_t nodeIndex)
{
    // WARNING: always use nodeIndex to index into the m_nodes array. never hold a reference/pointer,
    // because a push/emplace (in ANY recursive calls) might grow vectors, invalidating the pointers.

    // Compute the AABB of the current node.
    AxisAlignedBox aabb = computeSpanAABB(primitives);

    // As a starting point, we provide an implementation which creates a single leaf, and stores
    // all triangles inside it. You should remove or comment this, and work on your own recursive
    // construction algorithm that implements the following steps. Make sure to reuse the methods
    // you have previously implemented to simplify this process.
    //
    // 1. Determine if the node should be a leaf, when the nr. of triangles is less or equal to 4
    //    (hint; use the `LeafSize` constant)
    // 2. If it is a leaf, fill in the leaf's data, and store its range of triangles in `m_primitives`
    // 3. If it is a node:
    //    3a. Split the range of triangles along the longest axis into left and right subspans,
    //        using median split
    //    3b. Allocate left/right child nodes
    //        (hint: use `nextNodeIdx()`)
    //    3c. Fill in the current node's data; aabb, left/right child indices
    //    3d. Recursively build left/right child nodes over their respective triangles
    //        (hint; use `std::span::subspan()` to split into left/right ranges)

    // Just configure the current node as a giant leaf for now
    // m_nodes[nodeIndex] = buildLeafData(scene, aabb, primitives);

    if (primitives.size() <= LeafSize) {
        m_nodes[nodeIndex] = buildLeafData(scene, aabb, primitives);
    } else {
        uint32_t axis = computeAABBLongestAxis(aabb);
        size_t splitIdx = splitPrimitivesByMedian(aabb, axis, primitives);
        uint32_t leftChildIndex = nextNodeIdx();
        uint32_t rightChildIndex = nextNodeIdx();

        m_nodes[nodeIndex] = buildNodeData(scene, aabb, leftChildIndex,rightChildIndex);

        buildRecursive(scene, primitives.subspan(0,splitIdx), leftChildIndex);
        buildRecursive(scene, primitives.subspan(splitIdx), rightChildIndex);
    }
}

static bool intersectRayWithAABB(const AxisAlignedBox& box, const Ray& ray, float& tminOut)
{
    float tmin = std::numeric_limits<float>::lowest();
    float tmax = std::numeric_limits<float>::max();

    const glm::vec3& o = ray.origin;
    const glm::vec3& d = ray.direction;

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = axis == 0 ? o.x : (axis == 1 ? o.y : o.z);
        const float dir = axis == 0 ? d.x : (axis == 1 ? d.y : d.z);
        const float minb = axis == 0 ? box.lower.x : (axis == 1 ? box.lower.y : box.lower.z);
        const float maxb = axis == 0 ? box.upper.x : (axis == 1 ? box.upper.y : box.upper.z);

        if (dir == 0.0f) {
            if (origin < minb || origin > maxb) {
                return false;
            }
            continue;
        }

        float inv = 1.0f / dir;
        float t0 = (minb - origin) * inv;
        float t1 = (maxb - origin) * inv;
        if (t0 > t1) {
            std::swap(t0, t1);
        }

        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
        if (tmin > tmax) {
            return false;
        }
    }

    tminOut = tmin;
    if (tmax < 0.0f) {
        return false;
    }
    if (tmin > ray.t) {
        return false;
    }
    return true;
}

// TODO: BVH Traversal
// Hierarchy traversal routine; called by the BVH's intersect(),
// you must implement this method and implement it carefully!
//
// The default implementation just iterates the BVH's  underlying primitives (or the scene's geometry).
// You will have to implement the part which actually traverses the BVH for a faster intersect.
//
// This method returns `true` if geometry was hit, and `false` otherwise. On first/closest hit, the
// distance `t` in the `ray` object is updated, and information is updated in the `hitInfo` object.
//
// - scene;    the active scene
// - bvh;      the actual bvh which should be traversed for faster intersection
// - ray;      the ray intersecting the scene's geometry
// - hitInfo;  the return object, with info regarding the hit geometry
// - return;   boolean, if geometry was hit or not
//
// This method is unit-tested, so do not change the function signature.
bool intersectRayWithBVH(const Scene& scene, const BVHInterface& bvh, Ray& ray, HitData& hitInfo)
{
    // Relevant data in the constructed BVH
    std::span<const BVHInterface::Node> nodes = bvh.nodes();
    std::span<const BVHInterface::Primitive> primitives = bvh.primitives();

    // Return value
    bool is_hit = false;

        // TODO: implement here your (probably stack-based) BVH traversal.
        //
        // Some hints (refer to bvh_interface.h either way). BVH nodes are packed, so the
        // data is not easily extracted. Helper methods are available, however:
        // - For a given node, you can test if the node is a leaf with `node.isLeaf()`.
        // - If the node is not a leaf, you can obtain the left/right children with `node.leftChild()` etc.
        // - If the node is a leaf, you can obtain the offset to and nr. of primitives in the bvh's list
        //   of underlying primitives with `node.primitiveOffset()` and `node.primitiveCount()`
        //
        // In short, you will have to step down the bvh, node by node, and intersect your ray
        // with the node's AABB. If this intersection passes, you should:
        // - if the node is a leaf, intersect with the leaf's primitives
        // - if the node is not a leaf, test the left and right children as well!
        //
        // Note that it is entirely possible for a ray to hit a leaf node, but not its primitives,
        // and it is likewise possible for a ray to hit both children of a node.
        //
        // Make sure to update the hitInfo in case of a hit using updateHitInfo(...)

        // Naive implementation; simply iterates over all primitives
        // for (const auto& prim : primitives) {
        //     const auto& [v0, v1, v2] = std::tie(prim.v0, prim.v1, prim.v2);
        //     if (intersectRayWithTriangle(v0.position, v1.position, v2.position, ray)) {
        //         updateHitInfo(scene, prim, ray, hitInfo);
        //         is_hit = true;
        //     }
        // }

    std::vector<uint32_t> stack;
    stack.push_back(BVH::RootIndex);

    while (!stack.empty()) {
        uint32_t idx = stack.back();
        stack.pop_back();

        BVHInterface::Node node = nodes[idx];

        float t;
        if (!intersectRayWithAABB(node.aabb,ray,t)) continue;

        if (node.isLeaf()) {
            uint32_t offset = node.primitiveOffset();
            uint32_t count = node.primitiveCount();
            for (uint32_t i = 0; i < count; i++) {
                BVH::Primitive p = primitives[offset + i];
                if (intersectRayWithTriangle(p.v0.position, p.v1.position,p.v2.position, ray)) {
                    updateHitInfo(scene, p, ray, hitInfo);
                    is_hit = true;
                }
            }
        } else {
            stack.push_back(node.leftChild());
            stack.push_back(node.rightChild());
        }
    }


    return is_hit;
}



// TODO: Visual Debug
// Compute the nr. of levels in your hierarchy after construction; useful for `debugDrawLevel()`
void BVH::buildNumLevels()
{
    if (m_nodes.empty()) {
        m_numLevels = 0;
        return;
    }

    struct StackItem {
        uint32_t idx;
        uint32_t level;
    };

    uint32_t maxLevel = 0;
    std::vector<StackItem> stack;
    stack.push_back({ RootIndex, 0 });

    while (!stack.empty()) {
        StackItem item = stack.back();
        stack.pop_back();

        const Node& node = m_nodes[item.idx];
        maxLevel = std::max(maxLevel, item.level);

        if (!node.isLeaf()) {
            stack.push_back({ node.leftChild(), item.level + 1 });
            stack.push_back({ node.rightChild(), item.level + 1 });
        }
    }

    m_numLevels = maxLevel + 1;
}

// TODO: Visual Debug
// Compute the nr. of leaves in your hierarchy after construction; useful for `debugDrawLeaf()`
void BVH::buildNumLeaves()
{
    if (m_nodes.empty()) {
        m_numLeaves = 0;
        return;
    }

    uint32_t count = 0;
    std::vector<uint32_t> stack;
    stack.push_back(RootIndex);

    while (!stack.empty()) {
        uint32_t idx = stack.back();
        stack.pop_back();

        const Node& node = m_nodes[idx];
        if (node.isLeaf()) {
            ++count;
        } else {
            stack.push_back(node.leftChild());
            stack.push_back(node.rightChild());
        }
    }

    m_numLeaves = count;
}

// TODO: Visual Debug
// Draw the bounding boxes of the nodes at the selected level. Use this function to visualize nodes
// for debugging. You may wish to implement `buildNumLevels()` first. We suggest drawing the AABB
// of all nodes on the selected level.
void BVH::debugDrawLevel(int level)
{
    if (level < 0 || m_nodes.empty()) {
        return;
    }

    struct StackItem {
        uint32_t idx;
        uint32_t level;
    };

    std::vector<StackItem> stack;
    stack.push_back({ RootIndex, 0 });

    while (!stack.empty()) {
        StackItem item = stack.back();
        stack.pop_back();

        const Node& node = m_nodes[item.idx];
        if (static_cast<int>(item.level) == level) {
            drawAABB(node.aabb, DrawMode::Wireframe, glm::vec3(0.05f, 1.0f, 0.05f), 0.25f);
            continue;
        }

        if (!node.isLeaf() && item.level < static_cast<uint32_t>(level)) {
            stack.push_back({ node.leftChild(), item.level + 1 });
            stack.push_back({ node.rightChild(), item.level + 1 });
        }
    }
}

// TODO: Visual Debug
// Draw data of the leaf at the selected index. Use this function to visualize leaf nodes
// for debugging. You may wish to implement `buildNumLeaves()` first. We suggest drawing the AABB
// of the selected leaf, and then its underlying primitives with different colors.
// - leafIndex; index of the selected leaf.
//              (Hint: not the index of the i-th node, but of the i-th leaf!)
void BVH::debugDrawLeaf(int leafIndex)
{
    if (leafIndex < 0 || m_nodes.empty()) {
        return;
    }

    int currentLeaf = 0;
    std::vector<uint32_t> stack;
    stack.push_back(RootIndex);

    while (!stack.empty()) {
        uint32_t idx = stack.back();
        stack.pop_back();

        const Node& node = m_nodes[idx];
        if (node.isLeaf()) {
            if (currentLeaf == leafIndex) {
                drawAABB(node.aabb, DrawMode::Wireframe, glm::vec3(0.05f, 1.0f, 0.05f), 0.25f);
                uint32_t offset = node.primitiveOffset();
                uint32_t count = node.primitiveCount();
                for (uint32_t i = 0; i < count; ++i) {
                    const Primitive& prim = m_primitives[offset + i];
                    drawTriangle(prim.v0, prim.v1, prim.v2);
                }
                return;
            }
            ++currentLeaf;
        } else {
            stack.push_back(node.leftChild());
            stack.push_back(node.rightChild());
        }
    }
}
