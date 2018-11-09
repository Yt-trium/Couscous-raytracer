// Interface.
#include "render.h"

// couscous includes.
#include "renderer/gridaccelerator.h"
#include "renderer/material.h"
#include "renderer/samplegenerator.h"
#include "renderer/utility.h"
#include "renderer/visualobject.h"
#include "renderer/photonMapping.h"

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
    vec3 get_ray_color(
        const Ray&                      r,
        const size_t                    ray_max_depth,
        const VoxelGridAccelerator&     grid,
        const int                       current_depth)
    {
        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            Ray scattered;
            vec3 attenuation;
            vec3 emitted = rec.mat->emission();

            if (current_depth < int(ray_max_depth) && rec.mat->scatter(r, rec, attenuation, scattered))
            {
                return emitted + attenuation * get_ray_color(scattered, ray_max_depth, grid, current_depth + 1);
            }
            else
            {
                return emitted;
            }
        }
        else
        {
            return vec3(0.0f);
        }
    }

    vec3 get_ray_normal_color(const Ray &r, const VoxelGridAccelerator &grid)
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
        const Ray&                      r,
        const VoxelGridAccelerator&     grid,
        const PhotonTree&               ptree)
    {
        HitRecord rec;

        if(grid.hit(r, 0.0001f, numeric_limits<float>::max(), rec))
        {
            const auto& photon = ptree.get_closest(rec.p);

            // TODO: Clean this shit.
            // I scatter only to get the albedo. lol.
            // TODO: only get photons on this surface.
            Ray scattered;
            vec3 attenuation;
            photon.mat->scatter(r, rec, attenuation, scattered);

            // With attenuation.
            const vec3 p_to_photon = photon.position - rec.p;
            const float effector = 1.0 / pow(length(p_to_photon), 2.0);
            const vec3 color = attenuation * effector;
            return color;
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
    const size_t                    ray_max_depth,
    const Camera&                   camera,
    const VoxelGridAccelerator&     grid,
    const PhotonTree&               ptree,
    const bool                      parallel,
    const bool                      get_normal_color,
    const bool                      display_photon_map,
    QImage&                         image,
    QProgressBar&                   progressBar)
{
    progressBar.setValue(53);
    progressBar.setRange(0, int((width/64)*(height/64)));

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
                        color += get_ray_normal_color(r, grid);
                    }
                    else if (display_photon_map)
                    {
                        color += get_ray_photon_map(r, grid, ptree);
                    }
                    else
                    {
                        color += get_ray_color(r, ray_max_depth, grid, 0);
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

