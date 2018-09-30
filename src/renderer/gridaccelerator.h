#ifndef RENDER_GRID_ACCELERATOR_H
#define RENDER_GRID_ACCELERATOR_H

// couscous includes.
#include "renderer/aabb.h"
#include "renderer/visualobject.h"

// Standard includes.
#include <memory>
#include <vector>

// A voxel contains a list to
// shapes that go through it.
class Voxel
{
  public:
    std::vector<std::shared_ptr<VisualObject>> objects;
};

// A grid accelerator link shapes to a grid of voxel.
class VoxelGridAccelerator
{
  public:
    VoxelGridAccelerator(const MeshGroup& world);

  private:
    const MeshGroup&            m_world;
    glm::ivec3                  m_voxels_per_axis;
    glm::vec3                   m_voxel_size;
    glm::vec3                   m_inv_voxel_size;
    AABB                        m_bounds;
    std::unique_ptr<Voxel[]>    m_voxels;

    // Given a 3D position and an axis, return the index of
    // the voxel where the point is.
    size_t voxel(const glm::vec3& position, const size_t axis) const;

    // Given a voxel index and an axis, return the
    // coordinate of the voxel.
    float position(const size_t voxel, const size_t axis) const;

    // Return the memory index of a given indexed voxel.
    inline size_t offset(const size_t x, const size_t y, const size_t z) const;
};

size_t VoxelGridAccelerator::offset(const size_t x, const size_t y, const size_t z) const
{
    return z * m_voxels_per_axis[0] * m_voxels_per_axis[1] + y * m_voxels_per_axis[0] + x;
}

#endif // RENDER_GRID_ACCELERATOR_H
