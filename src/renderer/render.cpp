// Interface.
#include "render.h"

// couscous includes.
#include "renderer/gridaccelerator.h"
#include "renderer/material.h"
#include "renderer/samplegenerator.h"
#include "renderer/utility.h"
#include "renderer/visualobject.h"
#include "renderer/photonMapping.h"
#include "common/logger.h"

// Math includes.
#include <glm/glm.hpp>

// Qt includes.
#include <QFuture>
#include <QtConcurrentRun>

// Standard includes.
#include <algorithm>
#include <cmath>
#include <limits>


using namespace glm;
using namespace std;

namespace
{

    vec3 get_albedo(
        const Ray&                      r,
        const VoxelGridAccelerator&     grid)
    {
        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            return rec.mat->albedo;
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_normal(
        const Ray&                      r,
        const VoxelGridAccelerator&     grid)
    {
        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            return 0.5f * vec3(rec.normal.x + 1.0f, rec.normal.y + 1.0f, rec.normal.z + 1.0f);
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_ray_photon_map(
        const Ray&                                      r,
        const VoxelGridAccelerator&                     grid,
        PhotonTree::Fetcher<PHOTON_FETCH_SIZE>&         pfetcher)
    {
        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            const size_t count = pfetcher.find_closest(rec.p);

            float weight = 0.0f;
            vec3 color(0.0f);

            for (size_t i = 0; i < count; ++i)
            {
                const auto& photon = pfetcher.photon(i);

                const float dist = pfetcher.squared_dist(count-1);
                const float pweight = photon.energy;

                weight += pweight;
                color += (photon.mat->albedo * pweight) / (3.14f * (float)std::pow(dist, 2));
            }

            return (color / weight);
        }
        else
        {
            return vec3(0.0f);
        }
    }
}


vec3 Render::randomPointInTriangle(std::shared_ptr<Triangle> triangle)
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


vec3 Render::getRandomPointOnLights(const MeshGroup& lights)
{
    int lightIndice = std::rand()/((RAND_MAX + 1u)/lights.size());

    return randomPointInTriangle(lights[lightIndice]);
}


vec3 Render::random_in_unit_sphere() const
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


glm::vec3 Render::get_ray_color_phong(
    const Ray&                                      r,
    const VoxelGridAccelerator&                     grid,
    const MeshGroup&                                lights,
    PhotonTree::Fetcher<PHOTON_FETCH_SIZE>&         pfetcher)
{
    HitRecord rec;

    if (grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
    {
        float directLightIntensity = 0.0f, diffuseComp = 0.0f;
        glm::vec3 diffuse, specular, currentPointOnLight, albedo, currentLightDir;
        HitRecord directLightRec;
        std::vector<glm::vec3> lightPoints;

        glm::vec3 indirectColor = glm::vec3(0.0f);


        // Compute direct lighting by sending rays to lights
        for(int i=0; i<directlightRaysNumber; i++)
        {
            currentPointOnLight = getRandomPointOnLights(lights);
            currentLightDir = glm::normalize(currentPointOnLight - rec.p);

            bool answ = grid.hit(Ray(rec.p, currentLightDir), 0.00001f, numeric_limits<float>::max(), directLightRec);
            if( answ && (directLightRec.mat->emission != vec3(0.0f, 0.0f, 0.0f)) )
            {
                float currentDirectLightIntensity = directLightRec.mat->emission.x * 0.21f + directLightRec.mat->emission.y * 0.72f  + directLightRec.mat->emission.z * 0.07;

                // Compute direct light intensity : Li * ( (kd/PI) + (ks * (n+2)/2PI) )
                directLightIntensity += ((currentDirectLightIntensity/(glm::distance(rec.p, currentPointOnLight)))) /* * std::max(0.0f, glm::dot(glm::normalize(r.origin-rec.p), glm::normalize(rec.p - currentPointOnLight))) */;

                // Precompute diffuse element of computation : Sum( dot(rec.normal, pointOnLight ) / nbValidLights
                diffuseComp += std::max(0.0f, glm::dot(glm::normalize(rec.normal), currentLightDir));

                // Keep in memory valid lights point directions : valid light point means a visible one
                lightPoints.push_back(currentLightDir);
            }
        }


        // Compute indirect lighting with photon mapping
        int nbSuccessfullRays = 1;
        for(int i=0; i<indirectLightRaysNumber; i++)
        {
            glm::vec3 rayDirIndirectLight = random_in_unit_sphere();
            HitRecord recIndirect;
            Ray indirectLightRay = Ray(rec.p, rayDirIndirectLight);

            if(glm::dot(rayDirIndirectLight, rec.normal) < 0.0f)
            {
                rayDirIndirectLight = -rayDirIndirectLight;
            }

            if(grid.hit(indirectLightRay, 0.0001f, numeric_limits<float>::max(), recIndirect))
            {
                // Keep it just in case
                //indirectColor += (get_ray_photon_map(indirectLightRay, grid, pfetcher)/*/(1.0f + (glm::distance(rec.p, recIndirect.p)))*/) /* * std::max(0.0f, glm::dot(rec.normal, glm::normalize(recIndirect.p - rec.p)))*/;
                //indirectColor += (get_ray_photon_map(indirectLightRay, grid, pfetcher) * (1.0f - std::max(0.0f, glm::dot(glm::normalize(rec.normal), glm::normalize(recIndirect.p-rec.p)))));
                indirectColor += (get_ray_photon_map(indirectLightRay, grid, pfetcher) * (0.96f / 3.14f) + (0.04f * ( (2.0f + 2) / (3.14f * 2.0f))));
                nbSuccessfullRays++;
            }
            else
            {
                indirectColor += get_ray_photon_map(indirectLightRay, grid, pfetcher);
            }
        }


        indirectColor = (indirectColor /((float)nbSuccessfullRays));
        directLightIntensity = (directLightIntensity / lightPoints.size()) * (rec.mat->kd / 3.14f) + (rec.mat->ks * ( (rec.mat->specularExponent + 2) / (3.14f * 2.0f)));
        //directLightIntensity = (directLightIntensity / lightPoints.size()) * (0.96f / 3.14f) + (0.04f * ( (2.0f + 2) / (3.14f * 2.0f)));

        // Compute diffuse part
        albedo = rec.mat->albedo;

        diffuse = glm::vec3(0.0f);

        if(lightPoints.size() > 0)
        {
            diffuse = directLightIntensity * albedo  * (diffuseComp / lightPoints.size());
        }

        // Compute specular part
        specular = glm::vec3(0.0f);

        for(unsigned int i=0; i<lightPoints.size(); i++)
        {
            glm::vec3 rVec = (2*(glm::dot(glm::normalize(rec.normal), lightPoints[0])) * glm::normalize(rec.normal)) - lightPoints[0];
            // Try inverse here
            specular += rec.mat->albedo * (directLightIntensity) * std::pow(std::max(0.0f, glm::dot(glm::normalize((r.origin - rec.p)), glm::normalize(rVec))), rec.mat->specularExponent) ;
            //specular += rec.mat->albedo * (directLightIntensity) * std::pow(std::max(0.0f, glm::dot(glm::normalize((r.origin - rec.p)), glm::normalize(rVec))), 2.0f) ;
        }

        if(lightPoints.size()>0)
        {
            specular = specular / (float)lightPoints.size();
        }

        // Security check
        if(diffuse.x < 0.0f)
        {
            diffuse.x = 0.0f;
        }
        if(diffuse.y < 0.0f)
        {
            diffuse.y = 0.0f;
        }
        if(diffuse.z < 0.0f)
        {
            diffuse.z = 0.0f;
        }

        return diffuse * 0.96f + specular * 0.04f + indirectColor;
    }
    else
    {
        return vec3(0.0f);
    }
}


void Render::get_render_image(
    const size_t                    width,
    const size_t                    height,
    const size_t                    spp,
    const size_t                    ray_max_depth,
    const Camera&                   camera,
    const VoxelGridAccelerator&     grid,
    const PhotonTree&               ptree,
    const MeshGroup&                lights,
    const bool                      parallel,
    const bool                      get_normal_color,
    const bool                      get_albedo_color,
    const bool                      display_photon_map,
    QImage&                         image,
    QProgressBar&                   progressBar)
{
    progressBar.setValue(53);
    progressBar.setRange(0, int((width/64)*(height/64)));

    // Init random generator
    std::random_device rd;
    engine = std::mt19937(rd());
    distributor = std::uniform_real_distribution<>(0.0f, 1.0f);

    // Precompute subpixel samples position
    const size_t dimension_size =
        std::max(
            size_t(1),
            static_cast<size_t>(sqrt(spp)));
    const size_t samples = dimension_size * dimension_size;

    SampleGenerator generator(dimension_size);
    size_t x0, x1, x2, y0, y1, y2;

    // Thread handles
    vector<QFuture<void>> threads;

    // Job for rendering a given tile.
    auto compute =
    [&](
        size_t x1,
        size_t x2,
        size_t y1,
        size_t y2)
    {
        // Create a photon fetcher.
        PhotonTree::Fetcher<PHOTON_FETCH_SIZE> photon_fetcher = ptree.create_fetcher<PHOTON_FETCH_SIZE>();

        for (size_t y = y1; y < y2; ++y)
        {
            for (size_t x = x1; x < x2; ++x)
            {
                // In Qt, y is going from top to bottom.
                const vec2 pt(x, height - y - 1);
                const vec2 frame(width, height);

                vec3 color(0.0f, 0.0f, 0.0f);
                for (size_t i = 0; i < samples; ++i)
                {
                    const vec2 subpixel_pos = generator.next();
                    const vec2 uv(
                        (pt.x + subpixel_pos.x) / frame.x,
                        (pt.y + subpixel_pos.y) / frame.y);
                    const Ray r = camera.get_ray(uv.x, uv.y);

                    if(get_normal_color)
                    {
                        color += get_normal(r, grid);
                    }
                    else if(get_albedo_color)
                    {
                        color += get_albedo(r, grid);
                    }
                    else if (display_photon_map)
                    {
                        color += get_ray_photon_map(r, grid, photon_fetcher);
                    }
                    else
                    {
                        color += get_ray_color_phong(r, grid, lights, photon_fetcher);
                    }
                }

                color /= static_cast<float>(samples);
                color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));
                color.x = std::min(color.x, 1.0f);
                color.y = std::min(color.y, 1.0f);
                color.z = std::min(color.z, 1.0f);

                const QRgb rgb_color = qRgb(
                    static_cast<int>(255.0f * color[0]),
                    static_cast<int>(255.0f * color[1]),
                    static_cast<int>(255.0f * color[2]));

                image.setPixel(x, y, rgb_color);
            }
        }
    };

    // Create tile jobs or render tiles.
    for (y0 = 0; y0 < height; y0+=64)
    {
        for (x0 = 0; x0 < width; x0+=64)
        {
            x1 = x0;
            y1 = y0;
            x2 = std::min(x0+64, width);
            y2 = std::min(y0+64, height);

            if (parallel)
            {
                QFuture<void> future = QtConcurrent::run(compute, x1, x2, y1, y2);
                threads.push_back(future);
            }
            else
            {
                emit on_tile_begin(x1, y1, x2, y2);
                compute(x1, x2, y1, y2);
                progressBar.setValue(int((y0/64)*(width/64)+(x0/64)));
                emit on_tile_end(x1, y1, x2, y2, image);
            }
        }
    }

    y0 = 0;
    x0 = 0;

    for(size_t i = 0; i < threads.size(); ++i)
    {
        // Compute thread's tile position.
        x2 = std::min(x0 + 64, width);
        y2 = std::min(y0 + 64, height);

        emit on_tile_begin(x0, y0, x2, y2);
        threads.at(i).waitForFinished();
        emit on_tile_end(x0, y0, x2, y2, image);
        progressBar.setValue(int(i));

        x0 += 64;

        if (x0 >= width)
        {
            x0 = 0;
            y0 += 64;
        }
    }

    progressBar.setValue(progressBar.maximum());
}

