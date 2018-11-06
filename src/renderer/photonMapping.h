#ifndef RENDERER_PHOTONMAPPING_H
#define RENDERER_PHOTONMAPPING_H

// QT includes

#include <QFuture>
#include <QtConcurrentRun>
#include <QObject>
#include <QMutex>

// couscous includes.
#include "renderer/material.h"
#include "renderer/gridaccelerator.h"
#include "renderer/utility.h"
#include "renderer/visualobject.h"
#include "renderer/kDTree.hpp"

// glm includes.
#include <glm/glm.hpp>

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
    float modifyEnergy(float alpha,
                       float energy);


private:
    std::mt19937 engine;
    std::uniform_real_distribution<> distributor;
};


class Photon
{
public:

    Photon();

    Photon(glm::vec3 position,
           glm::vec3 inDirection,
           float energy,
           Material* mat);

    static float compute_energy(float inEnergy,
                                 float fr);

    glm::vec3 position;

    glm::vec3 inDirection;

    float energy;

    Material* mat;
};


class PhotonMap
{
  public:
    PhotonMap();
    PhotonMap(std::vector<Photon*> map);
    ~PhotonMap();

    void compute_map(
        const size_t                    samples,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const MeshGroup&                rawWorld,
        const MeshGroup&                lights);

    std::vector<Photon*> get_nearest_neihgboorhood(glm::vec3 photonPosition, unsigned int neighboorsNumber);

  private:
    void trace_photon_ray(
            const Ray&                      r,
            const size_t                    ray_max_depth,
            const VoxelGridAccelerator&     grid,
            const float                     inEnergy,
            const int                       depth = 0);

    void add_photon(Photon* photon);

    glm::vec3 randomPointInTriangle(std::shared_ptr<Triangle> triangle);

    glm::vec3 random_in_unit_sphere() const;

    std::vector<Photon*>                            map;
    std::vector<kDTreeObjectContainer<Photon*>*>    kdtreephotonData;
    QMutex                                          mapMutex;
    RussianRoulette                                 terminationSystem;
    float                                           alpha;
    std::mt19937                                    engine;
    std::uniform_real_distribution<>                distributor;
};

#endif

