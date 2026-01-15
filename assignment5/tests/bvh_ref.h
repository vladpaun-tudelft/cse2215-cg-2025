#pragma once

#include "../src/bvh_interface.h"
#include "../src/common.h"
#include "../src/intersect.h"
#include "../src/scene.h"
#include <array>
#include <functional>
#include <glm/gtx/component_wise.hpp>
#include <limits>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <algorithm>

namespace ref {
    namespace detail {
        static constexpr AxisAlignedBox aabbUnion(const AxisAlignedBox &lhs, const AxisAlignedBox &rhs);
        static constexpr AxisAlignedBox aabbInfinite();
        static constexpr float aabbSurfaceArea(const AxisAlignedBox &lhs);

        template<typename T, size_t size>
        class FixedStack {
            static_assert(std::is_trivially_constructible_v<T>);
            std::array<T, size> m_stack;
            size_t m_stackIdx = 0;

        public:
            void push(T v) {
                if (m_stackIdx + 1 == size) { throw std::runtime_error("Maximum stack size exceeded"); }
                m_stack[m_stackIdx++] = v;
            }

            T pop() { return m_stack[--m_stackIdx]; }
            bool empty() { return m_stackIdx == 0; }
        };

        inline bool intersectRayWithAABBVolume(const AxisAlignedBox &box, const Ray &ray, float &intersectionDistance);
        void updateHitInfo(const Scene &scene, const BVHInterface::Primitive &primitive, const Ray &ray, HitData &hitInfo);
    }

    AxisAlignedBox computePrimitiveAABB(BVHInterface::Primitive primitive);
    AxisAlignedBox computeSpanAABB(std::span<const BVHInterface::Primitive> primitives);
    glm::vec3 computePrimitiveCentroid(BVHInterface::Primitive primitive);
    uint32_t computeAABBLongestAxis(const AxisAlignedBox &aabb);
    size_t splitPrimitivesByMedian(const AxisAlignedBox &aabb, uint32_t axis, std::span<BVHInterface::Primitive> primitives);
    bool intersectRayWithBVH(const Scene &scene, const BVHInterface &bvh, Ray &ray, HitData &hitInfo);

    struct BVH : public BVHInterface {
        static constexpr uint32_t LeafSize = 4;
        static constexpr uint32_t RootIndex = 0;

        BVH(const Scene &scene);
        bool intersect(const Scene &scene, Ray &ray, HitData &hitInfo) const override;
        std::vector<Node> &nodes_v();
        const std::vector<Node> &nodes_v() const;
        std::vector<Primitive> &primitives_v();
        const std::vector<Primitive> &primitives_v() const;

    private:
        int m_numLevels;
        int m_numLeaves;
        std::vector<Node> m_nodes;
        std::vector<Primitive> m_primitives;

        uint32_t nextNodeIdx();
        Node buildLeafData(const Scene &scene, const AxisAlignedBox &aabb, std::span<Primitive> primitives);
        Node buildNodeData(const Scene &scene, const AxisAlignedBox &aabb, uint32_t leftChildIndex, uint32_t rightChildIndex);
        void buildRecursive(const Scene &scene, std::span<Primitive> primitives, uint32_t nodeIndex);
        void buildNumLevels();
        void buildNumLeaves();

    public:
        std::span<const Node> nodes() const override;
        std::span<Node> nodes() override;
        std::span<const Primitive> primitives() const override;
        std::span<Primitive> primitives() override;

        uint32_t numLevels() const override;
        uint32_t numLeaves() const override;
    };
} // namespace ref
