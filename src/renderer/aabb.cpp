// Interface.
#include "renderer/aabb.h"

// couscous includes.
#include "renderer/ray.h"

// Standard includes.
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace glm;
using namespace std;

AABB::AABB(
    const vec3& min,
    const vec3& max)
  : min(min)
  , max(max)
{
}

bool AABB::is_valid() const
{
    for (size_t i = 0; i < 3; ++i)
    {
        // Detect NaN values.
        if (!(min[i] <= max[i]))
            return false;
    }

    return true;
}

// Use the slab algorithm.
bool AABB::intersect(
    const Ray&  ray,
    const float tmin,
    const float tmax,
    float*      t_in,
    float*      t_out) const
{
    assert(is_valid());
    // Increase far parameter to avoid false positives due to rounding.
    static float error_padding = 1.0f + 2.0f * tgamma(3.0f);

    float t0 = tmin;
    float t1 = tmax;

    for (size_t i = 0; i < 3; ++i)
    {
        const float inv_dir = 1.0f / ray.dir[i];
        float near = (min[i] - ray.origin[i]) * inv_dir;
        float far = (max[i] - ray.origin[i]) * inv_dir;
        if (near > far) swap(near, far);
        far *= error_padding;
        t0 = near > t0 ? near : t0;
        t1 = far < t1 ? far : t1;
        if (t0 > t1) return false;
    }

    if (t_in) *t_in = t0;
    if (t_out) *t_out = t1;

    return true;
}

