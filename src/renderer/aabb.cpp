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

AABB::AABB()
{
}

AABB::AABB(
    const vec3& min,
    const vec3& max)
  : min(min)
  , max(max)
{
}

AABB::AABB(
    const vec3& point)
  : min(point)
  , max(point)
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

bool AABB::contains(const vec3& point) const
{
    for (size_t i = 0; i < 3; ++i)
    {
        if (point[i] < min[i] || point[i] > max[i])
            return false;
    }

    return true;
}

void AABB::add_point(const vec3& point)
{
    min = glm::min(min, point);
    max = glm::max(max, point);
}

size_t AABB::max_extent() const
{
    float max_extent = max[0] - min[0];
    size_t max_extent_index = 0;

    for (size_t i = 0; i < 3; ++i)
    {
        const float extent = max[i] - min[i];
        if (extent > max_extent)
        {
            max_extent = extent;
            max_extent_index = i;
        }
    }

    return max_extent_index;
}


//
// Artihmetic implementation
//

AABB operator+(const AABB& lhs, const AABB& rhs)
{
    AABB res = lhs;
    res.add_point(rhs.min);
    res.add_point(rhs.max);
    return res;
}

AABB& operator+=(AABB& lhs, const AABB& rhs)
{
    lhs.add_point(rhs.min);
    lhs.add_point(rhs.max);
    return lhs;
}
