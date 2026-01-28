#pragma once

#include "bvh_interface.h"
#include "common.h"
#include "interpolate.h"
#include "intersect.h"
#include "render.h"
#include "scene.h"
#include "unit_sphere.h"
#include <array>
#include <functional>
#include <glm/gtx/component_wise.hpp>
#include <limits>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <bit>

namespace ref {

// Helper class: a fake BVH that masquerades over a scene, but does not actually intersect
// it. Instead, the BVH stores or generates an array of intersection data that is returned
// in the specified order whenever the BVH is intersected.
struct FakeBVH : public BVHInterface { // Note, FakeBVH is in no way affiliated with a BVH
    struct FakeHit {
        float t;
        HitInfo hit;
    };

private:
    std::vector<FakeHit> m_hits;
    mutable uint32_t m_hit_index = 0;

public:
    FakeBVH(ref::Sampler& sampler, uint32_t n_hits)
    {
        m_hits.resize(n_hits);
        std::ranges::generate(m_hits, [&sampler]() {
            auto bary = sampler.next_3d();
            bary /= (bary.x + bary.y + bary.z);
            return FakeHit {
                .t = sampler.next_1d(),
                .hit = {
                    .normal = test::detail::pointOnSphere(sampler),
                    .barycentricCoord = bary,
                    .texCoord = sampler.next_2d(),
                    .material = {
                        .kd = sampler.next_3d(),
                        .ks = sampler.next_3d(),
                        .shininess = 64.f * sampler.next_1d(),
                        .transparency = sampler.next_1d(),
                    } }
            };
        });
        m_hits.push_back({ .t = std::numeric_limits<float>::max() });
    }

    FakeBVH(std::initializer_list<FakeHit> hits)
        : m_hits(hits)
    {
        m_hits.push_back({ .t = std::numeric_limits<float>::max() });
    }

    bool intersect(RenderState& state, Ray& ray, HitInfo& hitInfo) const override
    {
        if (m_hit_index < m_hits.size()) {
            ray.t = m_hits[m_hit_index].t;
            hitInfo = m_hits[m_hit_index].hit;
            m_hit_index++;
            return true;
        }
        return false;
    }

    void reset_hit_index()
    {
        m_hit_index = 0;
    }

    void set_hit_index(uint32_t i)
    {
        m_hit_index = i;
    }

    std::span<const FakeHit> hits() const
    {
        return m_hits;
    }

    std::span<FakeHit> hits()
    {
        return m_hits;
    }

    std::span<const Node> nodes() const override { return {}; }
    std::span<Node> nodes() override { return {}; }
    std::span<const Primitive> primitives() const override { return {}; }
    std::span<Primitive> primitives() override { return {}; }
    uint32_t numLevels() const override { return 0; }
    uint32_t numLeaves() const override { return 0; }
};
} // namespace ref
