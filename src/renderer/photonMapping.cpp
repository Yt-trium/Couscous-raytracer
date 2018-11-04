#include "photonMapping.h"


// ---------
// Russian roulette


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


// ---------
// Photon


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



// ----
// Photon map


PhotonMap::PhotonMap() :
    alpha(0.6f)
{
    std::random_device rd;
    engine = std::mt19937(rd());
    distributor = std::uniform_real_distribution<>(0.0f, 1.0f);
}


PhotonMap::PhotonMap(std::vector<Photon*> map) :
    map(map),
    alpha(0.6f)
{
    std::random_device rd;
    engine = std::mt19937(rd());
    distributor = std::uniform_real_distribution<>(0.0f, 1.0f);
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


glm::vec3 PhotonMap::randomPointInTriangle(std::shared_ptr<TriangleMesh> triangleMesh, int triangleIndice)
{
    glm::vec3 v1, v2, v3, answ;
    float r1 = distributor(engine);
    float r2 = distributor(engine);
    float m1=1 - sqrt(r1);
    float m2=sqrt(r1) * (1-r2);
    float m3=r2 * sqrt(r1);

    (*triangleMesh).getTriangleVertices(triangleIndice, v1, v2, v3);

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
        const                           bool parallel)
{
    std::shared_ptr<TriangleMesh> currentLight;
    MeshGroup lights;
    std::vector<QFuture<void>> threads;
    glm::vec3 rayOrigin, rayDirection;

    float totalEnergy = 0.0f;
    float energyForOneRay = 0.0f;
    int nbRays = std::floor(samples);
    int nbRaysPerLight = 0;
    int nbTriangleOfCurrentLight = 0;

    // Retrieve lights in scene
    for(auto it = rawWorld.begin(); it != rawWorld.end(); ++it)
    {
        currentLight = std::dynamic_pointer_cast<TriangleMesh>(*it);
        if( currentLight != nullptr && std::dynamic_pointer_cast<Light>((*currentLight).getMaterial()) != nullptr )
        {
            lights.push_back(*it);
        }
    }

    // TODO: Tell the user if there isn't any light and stop safely.
    assert(lights.size() > 0);

    nbRaysPerLight = nbRays/lights.size();
    totalEnergy = EnergyForOneLight * lights.size();
    energyForOneRay = totalEnergy/nbRays;


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

    // Compute all rays
    for(auto it=lights.begin(); it != lights.end(); ++it)
    {
        int currentTriangleIndice = 0;
        currentLight = std::dynamic_pointer_cast<TriangleMesh>(*it);
        nbTriangleOfCurrentLight = std::floor((*currentLight).getTriangleCount());

        for(int i=0; i<nbRaysPerLight + (i < nbRays%lights.size() ? 1 : 0); i++)
        {
            rayOrigin = randomPointInTriangle(currentLight, currentTriangleIndice);
            rayDirection = random_in_unit_sphere();
            if (parallel)
            {
                QFuture<void> future = QtConcurrent::run(compute, Ray(rayOrigin, rayDirection), energyForOneRay, this);
                threads.push_back(future);
            }
            else
            {
                trace_photon_ray(Ray(rayOrigin, rayDirection), ray_max_depth, grid, energyForOneRay);
            }

            currentTriangleIndice = (currentTriangleIndice + 1) % nbTriangleOfCurrentLight;
        }
    }


    for(size_t i = 0; i < threads.size(); ++i)
    {
        threads.at(i).waitForFinished();
    }

    for(auto it = map.begin(); it != map.end(); ++it)
    {
        kdtreephotonData.push_back(new kDTreeObjectContainer<Photon*>(*it, (*it)->position.x, (*it)->position.y, (*it)->position.z));
    }
}


std::vector<Photon*> PhotonMap::get_nearest_neihgboorhood(glm::vec3 photonPosition, unsigned int neighboorsNumber)
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



















