// Interface.
#include "render.h"

// couscous includes.
#include "renderer/gridaccelerator.h"
#include "renderer/material.h"
#include "renderer/samplegenerator.h"
#include "renderer/utility.h"
#include "renderer/visualobject.h"
#include "renderer/photonMapping.h"
#include "renderer/rng.h"
#include "renderer/utility.h"
#include "common/logger.h"

// Math includes.
#include <glm/glm.hpp>

// Qt includes.
#include <QFuture>
#include <QtConcurrentRun>
#include <QTime>

// Standard includes.
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

using namespace glm;
using namespace std;

#define indirectLightRaysNumber 16
#define COUCOUS_M_PI 3.1416f
#define COUCOUS_M_2PI 2.0f * 3.1416f
#define COUCOUS_M_INV_2PI 1.0f / (2.0f * 3.1416f)
#define COUCOUS_M_INV_PI 1.0f / 3.1416f

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
        const PhotonTree&                               ptree)
    {
        const float radius = grid.voxel_size();

        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            if (rec.mat->emission != vec3(0.0f))
                return vec3(0.0f);

            vector<pair<size_t, float>> find_result;

            size_t photons_count = ptree.find_in_radius(rec.p, radius, find_result);

            if (photons_count == 0)
                return vec3(0.0f);

            const float max_dist = sqrt(find_result[photons_count - 1].second);

            photons_count = photons_count > 20 ? 20 : photons_count;

            float weight = 0.0f;
            vec3 color(0.0f);

            for (size_t i = 0; i < photons_count; ++i)
            {
                const size_t index = find_result[i].first;
                const auto& photon = ptree.map.photon(index);

                // if (photon.mat != rec.mat) continue;

                const float pweight = photon.energy;

                weight += pweight;
                color += (photon.mat->albedo * pweight) / (3.14f * max_dist);
            }

            return (color / weight);
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_direct_diffuse(
        const Ray&                                      r,
        const size_t                                    directLightRaysCount,
        const VoxelGridAccelerator&                     grid,
        const MeshGroup&                                lights,
        RNG&                                            rng)
    {
        HitRecord rec;

        if (grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            // Display lights only by showing the emissive value.
            if(rec.mat->light)
            {
                return normalize(rec.mat->emission);
            }

            HitRecord directLightRec;
            vector<vec3> lightPoints;
            const Material* mat = rec.mat;
            vec3 diffuse(0.0f);

            // We cast n rau per lights.
            for (size_t l = 0; l < lights.size(); ++l)
            {
                const auto& light = lights[l];
                const vec3& va = light->vertice(0);
                const vec3& vb = light->vertice(1);
                const vec3& vc = light->vertice(2);

                // Compute direct lighting by sending rays to lights.
                for(size_t i = 0; i < directLightRaysCount; ++i)
                {
                    const vec3 currentPointOnLight = random_point_in_triangle(va, vb, vc, rng);
                    const vec3 currentLightDir = normalize(currentPointOnLight - rec.p);

                    bool answ = grid.hit(Ray(rec.p, currentLightDir), 0.0001f, numeric_limits<float>::max(), directLightRec);

                    // Only take into account emissive materials.
                    if (answ && directLightRec.mat->light)
                    {
                        const Material* light_mat = directLightRec.mat;

                        diffuse += mat->albedo * light_mat->emission *
                            std::max(0.0f, dot(rec.normal, currentLightDir));
                    }
                }
            }

            return (diffuse * mat->kd * COUCOUS_M_INV_PI) / static_cast<float>(lights.size() * directLightRaysCount);
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_direct_specular(
        const Ray&                                      r,
        const size_t                                    directLightRaysCount,
        const VoxelGridAccelerator&                     grid,
        const MeshGroup&                                lights,
        RNG&                                            rng)
    {
        HitRecord rec;

        if (grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            // Display lights only by showing the emissive value.
            if(rec.mat->light)
            {
                return normalize(rec.mat->emission);
            }

            HitRecord directLightRec;
            const Material* mat = rec.mat;
            const float ray_count = static_cast<float>(lights.size() * directLightRaysCount);
            const vec3 V = -r.dir;
            vec3 specular(0.0f);

            // We cast n rau per lights.
            for (size_t l = 0; l < lights.size(); ++l)
            {
                const auto& light = lights[l];
                const vec3& va = light->vertice(0);
                const vec3& vb = light->vertice(1);
                const vec3& vc = light->vertice(2);

                // Compute direct lighting by sending rays to lights.
                for(size_t i = 0; i < directLightRaysCount; ++i)
                {
                    const vec3 currentPointOnLight = random_point_in_triangle(va, vb, vc, rng);
                    const vec3 currentLightDir = normalize(currentPointOnLight - rec.p);

                    bool answ = grid.hit(Ray(rec.p, currentLightDir), 0.0001f, numeric_limits<float>::max(), directLightRec);

                    // Only take into account emissive materials.
                    if (answ && directLightRec.mat->light)
                    {
                        const Material* light_mat = directLightRec.mat;

                        vec3 R = reflect(-currentLightDir, rec.normal);
                        specular += light_mat->emission
                            * pow(std::max(0.0f, dot(R, V)), mat->specularExponent);
                    }
                }
            }

            return (specular * mat->ks * ((ray_count + 2.0f) * COUCOUS_M_INV_2PI)) / ray_count;
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_direct_phong(
        const Ray&                                      r,
        const size_t                                    directLightRaysCount,
        const VoxelGridAccelerator&                     grid,
        const MeshGroup&                                lights,
        RNG&                                            rng)
    {
        HitRecord rec;

        if (grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            // Display lights only by showing the emissive value.
            if(rec.mat->light)
            {
                return clamp(rec.mat->emission, 0.0f, 1.0f);
            }

            HitRecord directLightRec;
            const Material* mat = rec.mat;
            vec3 specular(0.0f), diffuse(0.0f);
            const vec3 V = -r.dir;
            const float ray_count = static_cast<float>(lights.size() * directLightRaysCount);

            // We cast n rau per lights.
            for (size_t l = 0; l < lights.size(); ++l)
            {
                const auto& light = lights[l];
                const vec3& va = light->vertice(0);
                const vec3& vb = light->vertice(1);
                const vec3& vc = light->vertice(2);

                // Compute direct lighting by sending rays to lights.
                for(size_t i = 0; i < directLightRaysCount; ++i)
                {
                    const vec3 currentPointOnLight = random_point_in_triangle(va, vb, vc, rng);
                    const vec3 currentLightDir = normalize(currentPointOnLight - rec.p);

                    bool answ = grid.hit(Ray(rec.p, currentLightDir), 0.0001f, numeric_limits<float>::max(), directLightRec);

                    // Only take into account emissive materials.
                    if (answ && directLightRec.mat->light)
                    {
                        const Material* light_mat = directLightRec.mat;

                        vec3 R = reflect(-currentLightDir, rec.normal);

                        specular += light_mat->emission
                            * pow(std::max(0.0f, dot(R, V)), mat->specularExponent);

                        diffuse += mat->albedo * light_mat->emission *
                            std::max(0.0f, dot(rec.normal, currentLightDir));
                    }
                }
            }

            return (
                diffuse * mat->kd * COUCOUS_M_INV_PI
                + specular * mat->ks * ((ray_count + 2.0f) * COUCOUS_M_INV_2PI))
                / ray_count;
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_final(
        const Ray&                                      r,
        const size_t                                    directLightRaysCount,
        const VoxelGridAccelerator&                     grid,
        const MeshGroup&                                lights,
        const PhotonTree&                               ptree,
        RNG&                                            rng)
    {
        HitRecord rec;

        if (grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            // Display lights only by showing the emissive value.
            if(rec.mat->light)
            {
                return clamp(rec.mat->emission, 0.0f, 1.0f);
            }

            // Compute Phong.
            float directLightIntensity = 0.0f, diffuseComp = 0.0f;
            vec3 diffuse, specular, albedo;
            HitRecord directLightRec;
            vector<vec3> lightPoints;

            vec3 indirectColor = vec3(0.0f);

            // Compute direct lighting by sending rays to lights.
            for(size_t i = 0; i < directLightRaysCount; ++i)
            {
                const vec3 currentPointOnLight = random_point_on_lights(lights, rng);
                const vec3 currentLightDir = normalize(currentPointOnLight - rec.p);

                bool answ = grid.hit(Ray(rec.p, currentLightDir), 0.00001f, numeric_limits<float>::max(), directLightRec);

                // Only take into account emissive materials.
                if (answ && (directLightRec.mat->emission != vec3(0.0f)))
                {
                    const float currentDirectLightIntensity =
                        directLightRec.mat->emission.x * 0.21f
                        + directLightRec.mat->emission.y * 0.72f
                        + directLightRec.mat->emission.z * 0.07f;

                    // Compute direct light intensity : Li * ( (kd/PI) + (ks * (n+2)/2PI) )
                    directLightIntensity += currentDirectLightIntensity / distance(rec.p, currentPointOnLight)
                                             /* * std::max(0.0f, glm::dot(glm::normalize(r.origin-rec.p), currentLightDir))*/
                                             * std::max(0.0f, dot(rec.normal, glm::normalize(r.origin - rec.p)))
                                             * std::max(0.0f, dot(rec.normal, currentLightDir)) ;

                    // Precompute diffuse element of computation : Sum( dot(rec.normal, pointOnLight ) / nbValidLights
                    diffuseComp += std::max(0.0f, dot(rec.normal, currentLightDir));

                    // Keep in memory valid lights point directions : valid light point means a visible one
                    lightPoints.push_back(currentLightDir);
                }
            }

            // Compute indirect lighting with photon mapping
            int nbSuccessfullRays = 1;
            for(int i=0; i<indirectLightRaysNumber; i++)
            {
                glm::vec3 rayDirIndirectLight = random_in_unit_sphere(rng);
                HitRecord recIndirect;
                Ray indirectLightRay;

                if(glm::dot(rayDirIndirectLight, rec.normal) < 0.0f)
                {
                    rayDirIndirectLight = -rayDirIndirectLight;
                }

                indirectLightRay = Ray(rec.p, rayDirIndirectLight);

                if(grid.hit(indirectLightRay, 0.000001f, numeric_limits<float>::max(), recIndirect))
                {
                    // Keep it just in case
                    //indirectColor += (get_ray_photon_map(indirectLightRay, grid, pfetcher)/*/(1.0f + (glm::distance(rec.p, recIndirect.p)))*/) /* * std::max(0.0f, glm::dot(rec.normal, glm::normalize(recIndirect.p - rec.p)))*/;
                    //indirectColor += (get_ray_photon_map(indirectLightRay, grid, pfetcher) * (1.0f - std::max(0.0f, glm::dot(glm::normalize(rec.normal), glm::normalize(recIndirect.p-rec.p)))));
                    indirectColor += (get_ray_photon_map(indirectLightRay, grid, ptree)
                                      /* * (0.96f / 3.14f) + (0.04f * ( (2.0f + 2) / (3.14f * 2.0f))) */)
                                    * (std::max(0.0f, glm::dot(rec.normal, rayDirIndirectLight)));
                    nbSuccessfullRays++;
                }
                else
                {
                    indirectColor += get_ray_photon_map(indirectLightRay, grid, ptree);
                }
            }


            indirectColor = (indirectColor /((float)nbSuccessfullRays));
            //directLightIntensity = (directLightIntensity / lightPoints.size()) * (rec.mat->kd / 3.14f) + (rec.mat->ks * ( (rec.mat->specularExponent + 2) / (3.14f * 2.0f)));
            directLightIntensity = (directLightIntensity /*/ lightPoints.size()*/) * (0.96f / 3.14f) + (0.04f * ( (2.0f + 2) / (3.14f * 2.0f)));

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
                //specular += rec.mat->albedo * (directLightIntensity) * std::pow(std::max(0.0f, glm::dot(glm::normalize((r.origin - rec.p)), glm::normalize(rVec))), rec.mat->specularExponent) ;
                specular += rec.mat->albedo * (directLightIntensity) * std::pow(std::max(0.0f, glm::dot(glm::normalize((r.origin - rec.p)), glm::normalize(rVec))), 2.0f) ;
            }

            if(lightPoints.size()>0)
            {
                specular = specular / (float)lightPoints.size();
            }

            // Security check
            diffuse = max(diffuse, 0.0f);

            return diffuse * 0.96f + specular * 0.04f + indirectColor;
        }
        else
        {
            return vec3(0.0f);
        }
    }
}

void Render::get_render_image(
    const size_t                    width,
    const size_t                    height,
    const size_t                    spp,
    const Camera&                   camera,
    const MeshGroup&                world,
    const size_t                    direct_light_rays_count,
    const bool                      parallel,
    const bool                      get_normal_color,
    const bool                      get_albedo_color,
    const bool                      display_photon_map,
    const bool                      direct_diffuse,
    const bool                      direct_specular,
    const bool                      direct_phong,
    QImage&                         image,
    QProgressBar&                   progressBar)
{
    // Create a random number generator.
    RNG rng;

    // Get lights from the scene.
    const MeshGroup lights = fetch_lights(world);
    Logger::log_debug(to_string(lights.size()) + " light triangles");
    Logger::log_debug(to_string(world.size() - lights.size()) + " triangles in the scene");

    // Create the grid accelerator.
    VoxelGridAccelerator grid(world);

    Logger::log_debug("fetching photons in a radius of " + to_string(grid.voxel_size()));

    // Create photon map.
    PhotonMap pmap;
    pmap.compute_map(100000, 32, grid, lights, rng);

    // Create photon tree.
    PhotonTree ptree(pmap);

    progressBar.setValue(53);
    progressBar.setRange(0, int((width/64)*(height/64)));

    // Precompute subpixel samples position
    const size_t dimension_size = static_cast<size_t>(std::max(1, static_cast<int>(sqrt(spp))));
    const size_t samples = dimension_size * dimension_size;

    SampleGenerator generator(dimension_size, rng);
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
                        color += get_ray_photon_map(r, grid, ptree);
                    }
                    else if (direct_diffuse)
                    {
                        color += get_direct_diffuse(r, direct_light_rays_count, grid, lights, rng);
                    }
                    else if (direct_specular)
                    {
                        color += get_direct_specular(r, direct_light_rays_count, grid, lights, rng);
                    }
                    else if (direct_phong)
                    {
                        color += get_direct_phong(r, direct_light_rays_count, grid, lights, rng);
                    }
                    else
                    {
                        color += get_final(r, direct_light_rays_count, grid, lights, ptree, rng);
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

    // Starts rendering.
    Logger::log_info("rendering...");

    QTime render_timer;
    render_timer.start();
    progressBar.setVisible(true);

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
    progressBar.setVisible(false);

    const int elapsed = render_timer.elapsed();

    QString message =
        QString("rendering finished in ")
        + ((elapsed > 1000)
            ? (QString::number(elapsed / 1000) + "s.")
            : (QString::number(elapsed % 1000) + "ms."));

    Logger::log_info(message.toStdString().c_str());
}

