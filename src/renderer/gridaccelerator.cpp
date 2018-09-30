// Interface.
#include "renderer/gridaccelerator.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <cmath>

using namespace glm;
using namespace std;

VoxelGridAccelerator::VoxelGridAccelerator(
    const MeshGroup& world)
  : m_world(world)
{
    assert(world.size());

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
    const float voxel_size = cube_root * inv_max_width;

    // Compute the number of voxels to create on each axis.
    for (size_t i = 0; i < 3; ++i)
    {
        m_voxels_per_axis[i] =
            static_cast<int>(
                round(grid_size[i] * voxel_size));
        m_voxels_per_axis[i] = clamp(m_voxels_per_axis[i], 1, 64);
    }

    // Compute the real size of voxels on each axis.
    for (size_t i = 0; i < 3; ++i)
    {
        m_voxel_size[i] = grid_size[i] / static_cast<float>(m_voxels_per_axis[i]);
        m_inv_voxel_size[i] = (m_voxel_size[i] == 0.0f) ? 0.0f : 1.0f / m_voxel_size[i];
    }

    const size_t resolution = m_voxels_per_axis[0] * m_voxels_per_axis[1] * m_voxels_per_axis[2];
    m_voxels.reset(new Voxel[resolution]);

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

