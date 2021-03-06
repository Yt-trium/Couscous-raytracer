#ifndef RENDERER_PHOTONMAPPING_H
#define RENDERER_PHOTONMAPPING_H

// couscous includes.
#include "renderer/material.h"
#include "renderer/gridaccelerator.h"
#include "renderer/utility.h"
#include "renderer/visualobject.h"

// QT includes
#include <QMutex>

// glm includes.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// nanoflann inclues.
#include <nanoflann/nanoflann.hpp>

// Standard library includes
#include <map>
#include <random>
#include <utility>
#include <vector>

// Forward declarations.
class RNG;

class Photon
{
  public:
    Photon(
        const glm::vec3&    position,
        const glm::vec3&    inDirection,
        const float         energy,
        const Material*     mat);

    static float compute_energy(
        const float inEnergy,
        const float fr);

    const glm::vec3     position;
    const glm::vec3     inDirection;
    const float         energy;
    const Material*     mat;
};


//
// A PhotonMap is just a list of Photons.
//

class PhotonMap
{
  public:
    PhotonMap();
    ~PhotonMap();

    inline size_t kdtree_get_point_count() const {
        return map.size();
    }

    inline float kdtree_get_pt(const size_t idx, int dim) const
    {
        if(dim == 0)
            return map[idx]->position.x;
        else if(dim == 1)
            return map[idx]->position.y;
        else
            return map[idx]->position.z;
    }

    template<class BBOX>
    bool kdtree_get_bbox(BBOX&) const { return false; }

    void compute_map(
        const size_t                    samples,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const MeshGroup&                lights,
        RNG&                            rng);

    const Photon& photon(const size_t index) const;

  private:
    void trace_photon_ray(
        const Ray&                      r,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const float                     inEnergy,
        RNG&                            rng,
        const size_t                    depth = 0);

    void add_photon(const Photon* photon);

    std::vector<const Photon*>                map;
    QMutex                                    mapMutex;
    float                                     alpha;
};


//
// Kd-tree structure for a PhotonMap
//

class PhotonTree
{
  private:
    typedef nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<float, PhotonMap>,
        PhotonMap,
        3,
        size_t> PhotonTreeIndex;

  public:
    // The kd-tree is built when the constructor is called.
    PhotonTree(const PhotonMap& map);

    const PhotonMap& map;

    size_t find_in_radius(
        const glm::vec3&                        point,
        const float                             radius,
        std::vector<std::pair<size_t, float>>&  results) const;

  private:
    // Nanoflann used to search in the kd-tree.
    PhotonTreeIndex m_index;
};

#endif

