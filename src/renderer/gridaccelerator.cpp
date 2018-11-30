// Interface.
#include "renderer/gridaccelerator.h"

// couscous includes.
#include "common/logger.h"
#include "renderer/ray.h"
#include "renderer/visualobject.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <cmath>

// Uncomment this if you don't want to use the grid
// accelerator while rendering.
// #define DEBUG_DISABLE_ACCELERATOR

using namespace glm;
using namespace std;

bool Voxel::hit(
    const Ray&                          r,
    const float                         tmin,
    HitRecord&                          rec) const
{
    if (objects.empty())
        return false;

    bool hit_something = false;

    // Test intersection will all the shapes in the voxel.
    for (size_t i = 0, e = objects.size(); i < e; ++i)
    {
        const shared_ptr<VisualObject>& shape = objects[i];
        hit_something |= shape->hit(r, tmin, rec.t, rec);
    }

    return hit_something;
}

VoxelGridAccelerator::VoxelGridAccelerator(
    const MeshGroup& world)
  : m_world(world)
{
    assert(world.size());

    Logger::log_info("building a grid accelerator...");

    m_bounds = world[0]->bbox();

    // Create bbox and choose a resolution.
    for (size_t i = 1; i < world.size(); ++i)
    {
        const shared_ptr<VisualObject> shape = world[i];
        m_bounds += shape->bbox();
    }

    // Compute the grid size.
    const vec3 grid_size = m_bounds.max - m_bounds.min;

    // Compute the ideal size of one voxel.
    // The goal is to have a cubic voxel if possible.
    // If meshes are uniform and uniformly spreaded,
    // using the number of meshes to deduce the number
    // of voxels is a good idea.
    const size_t max_extent = m_bounds.max_extent();
    const float inv_max_width = 1.0f / grid_size[max_extent];
    const float cube_root = 3.0f * pow(
        static_cast<float>(world.size()), 1.0f / 3.0f);
    m_best_voxel_size = cube_root * inv_max_width;

    // Compute the number of voxels to create on each axis.
    for (size_t i = 0; i < 3; ++i)
    {
        m_voxels_per_axis[i] =
            static_cast<int>(
                round(grid_size[i] * m_best_voxel_size));
        m_voxels_per_axis[i] = clamp(m_voxels_per_axis[i], 1, 64);
    }

    // Compute the real size of voxels on each axis.
    for (size_t i = 0; i < 3; ++i)
    {
        m_voxel_size[i] = grid_size[i] / static_cast<float>(m_voxels_per_axis[i]);
        m_inv_voxel_size[i] = (m_voxel_size[i] == 0.0f) ? 0.0f : 1.0f / m_voxel_size[i];
    }

    m_best_voxel_size = length(m_voxel_size) / 3.0f;

    const size_t resolution = m_voxels_per_axis[0] * m_voxels_per_axis[1] * m_voxels_per_axis[2];
    m_voxels.reset(new Voxel[resolution]);

    Logger::log_debug("grid voxel count: " + to_string(resolution) + ".");
    Logger::log_debug(
        "grid resolution: ("
        + to_string(m_voxels_per_axis[0]) + ","
        + to_string(m_voxels_per_axis[1]) + ","
        + to_string(m_voxels_per_axis[2]) + ").");

    // Link shapes to voxels.
    for (size_t i = 0, e = world.size(); i < e; ++i)
    {
        // Find min and max voxels the shape is goind through.
        const shared_ptr<VisualObject> shape = world[i];
        const AABB& bbox = shape->bbox();
        ivec3 index_min, index_max;
        for (size_t a = 0; a < 3; ++a)
        {
            index_min[a] = voxel(bbox.min, a);
            index_max[a] = voxel(bbox.max, a);
        }

        // Link the shape to all the voxels
        // from the minimum voxel to the maximal voxel.
        const size_t z_min = static_cast<size_t>(index_min[2]);
        const size_t z_max = static_cast<size_t>(index_max[2]);
        const size_t y_min = static_cast<size_t>(index_min[1]);
        const size_t y_max = static_cast<size_t>(index_max[1]);
        const size_t x_min = static_cast<size_t>(index_min[0]);
        const size_t x_max = static_cast<size_t>(index_max[0]);
        for (size_t z = z_min; z <= z_max; ++z)
        {
            for (size_t y = y_min; y <= y_max; ++y)
            {
                for (size_t x = x_min; x <= x_max; ++x)
                {
                    const size_t o = offset(x, y, z);
                    m_voxels[o].objects.push_back(shape);
                }
            }
        }
    }
}

bool VoxelGridAccelerator::hit(
    const Ray&                          r,
    const float                         tmin,
    float                               tmax,
    HitRecord&                          rec) const
{
#ifdef DEBUG_DISABLE_ACCELERATOR
    return hit_world(m_world, r, tmin, tmax, rec);
#else
    // Parameter for the point where the ray enters the grid.
    float t = 0.0f;

    // Check if the ray starts in the grid.
    // This will happen a lot with rebounds.
    if (m_bounds.contains(r.point(tmin)))
    {
        t = tmin;
    }
    // Otherwise check if the ray intersects the grid.
    else if (!m_bounds.intersect(r, tmin, tmax, &t))
    {
        return false;
    }

    rec.t = tmax;

    // Digital Differental Analyser.
    // Compute next voxel entry points.

    // What we need to store:
    // - Position of the current voxel in the grid space (pos).
    // - Ray parameter t that intersects the next voxel (next_t).
    // - Direction to the next voxel in the grid space (step).
    // - Distance to the next voxel in each direction (delta_t).
    // - Coordinates of the voxel where the ray leaves the grid (out).

    // Setup variables.
    const vec3 entry = r.point(t);
    vec3 next_t(0.0f), delta_t(0.0f);
    ivec3 step(0), out(0), pos(0);

    for (size_t i = 0; i < 3; ++i)
    {
        pos[i] = voxel(entry, i);

        if (r.dir[i] >= 0.0f)
        {
            // Ray with positive direction.
            // Next parameter t is simply
            // the entry parameter t plus the axis distance to the next voxel
            // and divided by the axis direction.

            // If the ray direction is 0, then next_t will be INF.
            // We use this to never go in this direction later.
            next_t[i] = t + (position(pos[i] + 1, i) - entry[i]) / r.dir[i];
            delta_t[i] = m_voxel_size[i] / r.dir[i];
            step[i] = 1;
            out[i] = m_voxels_per_axis[i];
        }
        else
        {
            // Ray with negative direction.
            next_t[i] = t + (position(pos[i], i) - entry[i]) / r.dir[i];
            delta_t[i] = -m_voxel_size[i] / r.dir[i];
            step[i] = -1;
            out[i] = -1;
        }
    }

    // Test intersection with shapes.
    bool hit_something = false;

    while (true)
    {
        // Check intersection with the current voxel.
        const Voxel& voxel = m_voxels[offset(pos[0], pos[1], pos[2])];
        hit_something |= voxel.hit(r, tmin, rec);

        // Move to the next voxel.
        // We choose the best axis.
        // It's the one getting us to the closest voxel.
        size_t axis = (next_t[1] < next_t[0]) ? 1 : 0;
        axis = (next_t[2] < next_t[axis]) ? 2 : axis;

        // We stop if the current intersection point is
        // closer than the next voxel.
        if (rec.t < next_t[axis])
            break;

        pos[axis] += step[axis];

        // Check if we are getting out of the grid.
        if (pos[axis] == out[axis])
            break;

        next_t[axis] += delta_t[axis];
    }

    return hit_something;
#endif
}

float VoxelGridAccelerator::voxel_size() const
{
    return m_best_voxel_size;
}

size_t VoxelGridAccelerator::voxel(const vec3& position, const size_t axis) const
{
    const int index = static_cast<int>(
        (position[axis] - m_bounds.min[axis])
        * m_inv_voxel_size[axis]);

    return clamp(index, 0, m_voxels_per_axis[axis] - 1);
}

float VoxelGridAccelerator::position(const size_t voxel, const size_t axis) const
{
    return m_bounds.min[axis] + voxel * m_voxel_size[axis];
}

