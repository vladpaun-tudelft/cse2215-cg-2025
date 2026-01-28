#include "bvh.h"

// This method returns `true` if geometry was hit, and `false` otherwise. On first/closest hit, the
// distance `t` in the `ray` object is updated, and information is updated in the `hitInfo` object.
// - state;    current render state (containing scene, features, ...)
// - bvh;      the actual bvh which should be traversed for faster intersection
// - ray;      the ray intersecting the scene's geometry
// - hitInfo;  the return object, with info regarding the hit geometry
// - return;   boolean, if geometry was hit or not
bool BVH::intersect(RenderState& state, Ray& ray, HitInfo& hitInfo) const
{
    return intersectRayWithBVH(state, *this, ray, hitInfo);
}
