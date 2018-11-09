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

// nanoflann inclues.
#include <nanoflann/nanoflann.hpp>

// Standard library includes
#include <map>
#include <random>

#define photonMinErnergy 0.2f
#define rangeMin 0.0f
#define rangeMax 1.0f

// Class used to modelize the russian roulette principle
class RussianRoulette
{
  public:
    RussianRoulette();

    // Generate a random number U between 0-1, 0<alpha<1
    //      if U <= alpha then return 0
    //      else return energy/(1-alpha) to compensate energy loss during process due to russian roulette
    float modifyEnergy(
        const float alpha,
        const float energy);

  private:
    std::mt19937 engine;
    std::uniform_real_distribution<> distributor;
};

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
        const MeshGroup&                rawWorld,
        const MeshGroup&                lights);

    const Photon& photon(const size_t index) const;

  private:
    void trace_photon_ray(
        const Ray&                      r,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const float                     inEnergy,
        const int                       depth = 0);

    void add_photon(const Photon* photon);

    glm::vec3 randomPointInTriangle(std::shared_ptr<Triangle> triangle);

    glm::vec3 random_in_unit_sphere() const;

    std::vector<const Photon*>                map;
    QMutex                                    mapMutex;
    RussianRoulette                           terminationSystem;
    float                                     alpha;
    std::mt19937                              engine;
    std::uniform_real_distribution<>          distributor;
};


//
// Kd-tree structure for a PhotonMap
//

class PhotonTree
{
  public:
    // The kd-tree is built when the constructor is called.
    PhotonTree(const PhotonMap& map);

    // Returns the closest photon to a given point.
    // This method isn't meant to be called in a loop
    // because it is not optimized. It allocates memory
    // to search for a photon.
    const Photon& get_closest(const glm::vec3& pos) const;

    const PhotonMap& map;

  private:
    typedef nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<float, PhotonMap>,
        PhotonMap,
        3> PhotonTreeIndex;

    // Nanoflann used to search in the kd-tree.
    PhotonTreeIndex m_index;
};

#endif

