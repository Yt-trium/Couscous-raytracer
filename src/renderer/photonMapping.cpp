#include "photonMapping.h"

// couscous includes.
#include "common/logger.h"

// Standard includes.
#include <string>

using namespace std;

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

float RussianRoulette::modifyEnergy(float alpha,
                   float energy)
{
    if(distributor(engine) <= alpha)
    {
        return 0.0f;
    }

    return energy/(1.0f-alpha);
}


//
// Photon implementation.
//

Photon::Photon()
{
    this->position = glm::vec3(0.0f, 0.0f, 0.0f);
    this->energy = 0.0f;
}

Photon::Photon(glm::vec3 position,
               glm::vec3 inDirection,
               float energy,
               Material* mat)
{
    this->position = position;
    this->inDirection = inDirection;
    this->energy = energy;
    this->mat = mat;
}

float compute_energy(float inEnergy, float fr)
{
    return inEnergy * fr;
}


//
// Photon map implementation.
//

PhotonMap::PhotonMap()
  : alpha(0.6f)
  , engine(random_device()())
  , distributor(0.0f, 1.0f)
{
}

PhotonMap::PhotonMap(std::vector<Photon*> map)
  : map(map)
  , alpha(0.6f)
  , engine(random_device()())
  , distributor(0.0f, 1.0f)
{
}

PhotonMap::~PhotonMap()
{

    for(auto it =  this->map.begin() ; it != this->map.end(); ++it)
    {
        delete(*it);
    }
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
        Photon* hitPoint = nullptr;
        Ray scattered;
        glm::vec3 attenuation;
        bool isScatterValid = rec.mat->scatter(r, rec, attenuation, scattered);


        float hitPointEnergy = compute_energy(inEnergy, rec.mat->brdf());

        // Russian roulette here to know if we stop ourselves or not
        hitPointEnergy = terminationSystem.modifyEnergy(alpha, hitPointEnergy);
        if(hitPointEnergy >= photonMinErnergy)
        {
            hitPoint = new Photon(rec.p, r.origin, hitPointEnergy, rec.mat);
            add_photon(hitPoint);

            // scatterd ray (cast here should not cause any problem, because ray depth will not be greater thant MAX_INT in practice)
            if(depth < static_cast<int>(ray_max_depth) && isScatterValid)
            {
                trace_photon_ray(scattered, ray_max_depth, grid, hitPointEnergy, depth + 1);
            }
        }
    }
}

void PhotonMap::add_photon(Photon *photon)
{
    mapMutex.lock();
    this->map.push_back(photon);
    mapMutex.unlock();
}

glm::vec3 PhotonMap::randomPointInTriangle(std::shared_ptr<Triangle> triangle)
{
    glm::vec3 v1, v2, v3, answ;
    float r1 = distributor(engine);
    float r2 = distributor(engine);
    float m1=1 - sqrt(r1);
    float m2=sqrt(r1) * (1-r2);
    float m3=r2 * sqrt(r1);

    (*triangle).getVertices(v1, v2, v3);

    answ = m1*v1 + m2*v2 + m3*v3;

    return answ;
}

glm::vec3 PhotonMap::random_in_unit_sphere() const
{
    glm::vec3 p;
    do
    {
        p = 2.0f * glm::vec3(
            random<float>(),
            random<float>(),
            random<float>()) - glm::vec3(1, 1, 1);
    } while(glm::dot(p, p) >= 1);
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

    Logger::log_info("compute photon map...");

    const size_t nbRaysPerLight = samples / lights.size();

    Logger::log_debug("pm rays per light: " + to_string(nbRaysPerLight) + ".");

    Logger::log_debug("pm total initial rays: " + to_string(samples) + ".");

    const float totalEnergy = EnergyForOneLight * static_cast<float>(lights.size());
    // TODO: Use the light emmissive value to define the energy of each ray.
    const float energyForOneRay = totalEnergy/ static_cast<float>(samples);
    Logger::log_debug("pm initial rays energy: " + to_string(energyForOneRay) + " W.");

    // Job for computing a photon path
    auto compute =
    [&](
        const Ray&                      r,
        const float                     inEnergy,
        PhotonMap*                      map
       )
    {
        map->trace_photon_ray(r, ray_max_depth, grid, inEnergy);
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
            QFuture<void> future = QtConcurrent::run(compute, r, energyForOneRay, this);
            threads.push_back(future);
#endif
        }
    }

    // Waiting for tracing to end.
    for(size_t i = 0; i < threads.size(); ++i)
    {
        threads.at(i).waitForFinished();
    }

    // Create a Kd-tree for storing all photons.
    Logger::log_info("adding photons in a kd-tree...");

    for(auto it = map.begin(); it != map.end(); ++it)
    {
        kdtreephotonData.push_back(new kDTreeObjectContainer<Photon*>(*it, (*it)->position.x, (*it)->position.y, (*it)->position.z));
    }
}

std::vector<Photon*> PhotonMap::get_nearest_neihgboorhood(glm::vec3 photonPosition, unsigned int neighboorsNumber) const
{
    kDTree<Photon*> tree;
    kDTreeObjectContainer<Photon*>* refPosition = new kDTreeObjectContainer<Photon*> (nullptr, photonPosition.x, photonPosition.y, photonPosition.z);
    std::vector<kDTreeObjectContainer<Photon*>*> answ;
    std::vector<Photon*> answAsPhotonVector;

    tree.init(kdtreephotonData);

    answ = tree.findKNN(refPosition, neighboorsNumber);

    for(auto it = answ.begin(); it != answ.end(); ++it)
    {
        answAsPhotonVector.push_back((*it)->object);
    }

    return answAsPhotonVector;
}

