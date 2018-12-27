#include "photonMapping.h"

// couscous includes.
#include "common/logger.h"
#include "renderer/rng.h"
#include "renderer/utility.h"

// Qt includes.
#include <QtConcurrentRun>
#include <QFuture>
#include <QObject>
#include <QTime>

// Standard includes.
#include <string>

using namespace std;
using namespace glm;

// For the creation of the photon map on a single thread.
// #define FORCE_SINGLE_THREAD

namespace
{
    // Generate a random number U between 0-1, 0<alpha<1
    //      if U <= alpha then return 0
    //      else return energy/(1-alpha) to compensate energy loss during process due to russian roulette
    float russian_roulette(
        const float alpha,
        const float energy,
        RNG&        rng)
    {
        if(rng.next() <= alpha)
            return 0.0f;

        return energy / (1.0f - alpha);
    }
}


//
// Photon implementation.
//

Photon::Photon(
    const vec3&     position,
    const vec3&     inDirection,
    const float     energy,
    const Material* mat)
  : position(position)
  , inDirection(inDirection)
  , energy(energy)
  , mat(mat)
{
}

float Photon::compute_energy(
    const float inEnergy,
    const float fr)
{
    return inEnergy * fr;
}


//
// PhotonMap class implementation.
//

PhotonMap::PhotonMap()
  : alpha(0.6f)
{
}

PhotonMap::~PhotonMap()
{
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        delete (*it);
    }

    map.clear();
}

void PhotonMap::trace_photon_ray(
    const Ray&                      r,
    const size_t                    ray_max_depth,
    const VoxelGridAccelerator&     grid,
    const float                     inEnergy,
    RNG&                            rng,
    const size_t                    depth)
{
    HitRecord rec;

    if(grid.hit(r, 0.0001f, std::numeric_limits<float>::max(), rec))
    {
        const vec3 reflection = reflect(r.dir, rec.normal);
        const Ray scattered(rec.p, rec.mat->roughness
                ? random_in_cone(reflection, rec.mat->roughness, rng)
                : reflection);

        // Check validity.
        const bool isScatterValid = dot(scattered.dir, rec.normal) > 0.0f;

        float hitPointEnergy = Photon::compute_energy(inEnergy, rec.mat->brdf());

        Photon* photon = new Photon(rec.p, r.origin, hitPointEnergy, rec.mat);
        add_photon(photon);

        // Russian roulette here to know if we stop ourselves or not
        hitPointEnergy = russian_roulette(alpha, hitPointEnergy, rng);

        // Reflect the photon if not absorbed.
        if(isScatterValid && hitPointEnergy < 0.8f && depth < ray_max_depth)
        {
            trace_photon_ray(scattered, ray_max_depth, grid, hitPointEnergy, rng, depth + 1);
        }
    }
}

void PhotonMap::add_photon(const Photon* photon)
{
    mapMutex.lock();
    map.push_back(photon);
    mapMutex.unlock();
}

void PhotonMap::compute_map(
    const size_t                    samples,
    const size_t                    ray_max_depth,
    const VoxelGridAccelerator&     grid,
    const MeshGroup&                lights,
    RNG&                            rng)
{
    if(lights.size() == 0)
    {
        Logger::log_error("No lights were found in scene. Please, add at least one light before lighting computation.");
        return;
    }

    QTime timer;
    timer.start();

    const size_t nbRaysPerLight = samples / lights.size();

    Logger::log_debug("pm rays per light: " + to_string(nbRaysPerLight) + ".");

    // Job for computing a photon path
    auto compute =
    [&](
        const Ray&                      r,
        const float                     inEnergy
       )
    {
        this->trace_photon_ray(r, ray_max_depth, grid, inEnergy, rng);
    };

    std::vector<QFuture<void>> threads;

    // Compute all rays for each light.
    for(auto it = lights.begin(); it != lights.end(); ++it)
    {
        const auto& currentLight = *it;
        const float energyForOneRay = currentLight->mat()->light_power / float(nbRaysPerLight);

        for(size_t i = 0; i < nbRaysPerLight; ++i)
        {
            // Create a ray starting from the current light
            // and going in a random direction.
            vec3 rayDir = random_in_unit_sphere(rng);

            const vec3& va = currentLight->vertice(0);
            const vec3& vb = currentLight->vertice(1);
            const vec3& vc = currentLight->vertice(2);

            // Make it point in the correct direction.
            if(dot(rayDir, cross(vc - va, vb - va)) > 0.0f)
            {
                rayDir = -rayDir;
            }

            const Ray r(random_point_in_triangle(va, vb, vc, rng), rayDir);

#ifdef FORCE_SINGLE_THREAD
            trace_photon_ray(r, ray_max_depth, grid, energyForOneRay, rng);
#else
            QFuture<void> future = QtConcurrent::run(compute, r, energyForOneRay);
            threads.push_back(future);
#endif
        }
    }

    // Waiting for tracing to end.
    for(size_t i = 0; i < threads.size(); ++i)
    {
        threads.at(i).waitForFinished();
    }

    const int pm_elapsed = timer.elapsed();

    const QString message =
        QString("generated ")
        + QString::number(map.size())
        +  QString(" photons in ")
        + ((pm_elapsed > 1000)
            ? (QString::number(pm_elapsed / 1000) + "s.")
            : (QString::number(pm_elapsed % 1000) + "ms."));

    Logger::log_info(message.toStdString().c_str());
}

const Photon& PhotonMap::photon(const size_t index) const
{
    assert(index < map.size());
    return *map[index];
}


//
// PhotonTree class implementation.
//

PhotonTree::PhotonTree(const PhotonMap& map)
  : map(map)
  , m_index(3, map, nanoflann::KDTreeSingleIndexAdaptorParams(10)) // leaf max size
{
    QTime timer;
    timer.start();

    // Build the kd-tree.
    m_index.buildIndex();

    const int kd_elapsed = timer.elapsed();

    const QString message =
        QString("added photons in a kd-tree in ")
        + ((kd_elapsed > 1000)
            ? (QString::number(kd_elapsed / 1000) + "s.")
            : (QString::number(kd_elapsed % 1000) + "ms."));

    Logger::log_info(message.toStdString().c_str());
}

size_t PhotonTree::find_in_radius(
    const vec3&                     point,
    const float                     radius,
    vector<pair<size_t, float>>&    results) const
{
    static nanoflann::SearchParams search_params(32, 0.0f, true); // true means we want photons to be sorted by dist

    return m_index.radiusSearch(glm::value_ptr(point), radius, results, search_params);
}

