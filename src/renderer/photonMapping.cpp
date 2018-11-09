#include "photonMapping.h"

// couscous includes.
#include "common/logger.h"

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

//
// Russian roulette.
//

RussianRoulette::RussianRoulette()
{
    std::random_device rd;
    engine = std::mt19937(rd());
    distributor = std::uniform_real_distribution<>(rangeMin, rangeMax);
}

float RussianRoulette::modifyEnergy(
    const float alpha,
    const float energy)
{
    if(distributor(engine) <= alpha)
        return 0.0f;

    return energy / (1.0f - alpha);
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
  , engine(random_device()())
  , distributor(0.0f, 1.0f)
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
    const int                       depth)
{
    HitRecord rec;

    if(grid.hit(r, 0.0001f, std::numeric_limits<float>::max(), rec))
    {
        Ray scattered;
        vec3 attenuation;
        bool isScatterValid = rec.mat->scatter(r, rec, attenuation, scattered);


        float hitPointEnergy = Photon::compute_energy(inEnergy, rec.mat->brdf());

        // Russian roulette here to know if we stop ourselves or not
        hitPointEnergy = terminationSystem.modifyEnergy(alpha, hitPointEnergy);
        if(hitPointEnergy >= photonMinErnergy)
        {
            Photon* photon = new Photon(rec.p, r.origin, hitPointEnergy, rec.mat);
            add_photon(photon);

            // scatterd ray (cast here should not cause any problem, because ray depth will not be greater thant MAX_INT in practice)
            if(depth < static_cast<int>(ray_max_depth) && isScatterValid)
            {
                trace_photon_ray(scattered, ray_max_depth, grid, hitPointEnergy, depth + 1);
            }
        }
    }
}

void PhotonMap::add_photon(const Photon* photon)
{
    mapMutex.lock();
    map.push_back(photon);
    mapMutex.unlock();
}

vec3 PhotonMap::randomPointInTriangle(std::shared_ptr<Triangle> triangle)
{
    vec3 v1, v2, v3, answ;
    float r1 = distributor(engine);
    float r2 = distributor(engine);
    float m1=1 - sqrt(r1);
    float m2=sqrt(r1) * (1-r2);
    float m3=r2 * sqrt(r1);

    (*triangle).getVertices(v1, v2, v3);

    answ = m1*v1 + m2*v2 + m3*v3;

    return answ;
}

vec3 PhotonMap::random_in_unit_sphere() const
{
    vec3 p;
    do
    {
        p = 2.0f * vec3(
            random<float>(),
            random<float>(),
            random<float>()) - vec3(1, 1, 1);
    } while(dot(p, p) >= 1);
    return p;
}

void PhotonMap::compute_map(
    const size_t                    samples,
    const size_t                    ray_max_depth,
    const VoxelGridAccelerator&     grid,
    const MeshGroup&                rawWorld,
    const MeshGroup&                lights)
{
    static float EnergyForOneLight = 150.0f;

    // TODO: Tell the user if there isn't any light and stop safely.
    assert(lights.size() > 0);

    QTime timer;
    timer.start();

    const size_t nbRaysPerLight = samples / lights.size();
    const float totalEnergy = EnergyForOneLight * static_cast<float>(lights.size());
    // TODO: Use the light emmissive value to define the energy of each ray.
    const float energyForOneRay = totalEnergy/ static_cast<float>(samples);

    Logger::log_debug("pm rays per light: " + to_string(nbRaysPerLight) + ".");
    Logger::log_debug("pm total initial rays: " + to_string(samples) + ".");
    Logger::log_debug("pm initial rays energy: " + to_string(energyForOneRay) + " W.");

    // Job for computing a photon path
    auto compute =
    [&](
        const Ray&                      r,
        const float                     inEnergy
       )
    {
        this->trace_photon_ray(r, ray_max_depth, grid, inEnergy);
    };

    std::vector<QFuture<void>> threads;

    // Compute all rays for each light.
    for(auto it = lights.begin(); it != lights.end(); ++it)
    {
        const auto& currentLight = *it;

        for(size_t i = 0; i < nbRaysPerLight; ++i)
        {
            // Create a ray starting from the current light
            // and going in a random direction.
            const Ray r(randomPointInTriangle(currentLight), random_in_unit_sphere());

#ifdef FORCE_SINGLE_THREAD
            trace_photon_ray(r, ray_max_depth, grid, energyForOneRay);
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

    if (map.size() < samples)
        Logger::log_warning("too few photons was generated.");
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
  , m_index(3, map, nanoflann::KDTreeSingleIndexAdaptorParams(10))
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

