// Interface.
#include "render.h"

// couscous includes.
#include "renderer/material.h"
#include "renderer/samplegenerator.h"

// Math includes.
#include <glm/glm.hpp>

// Standard includes.
#include <algorithm>
#include <limits>

using namespace glm;
using namespace std;

Render::Render()
{
}

vec3 Render::get_ray_color(
    const Ray&              r,
    const size_t            ray_max_depth,
    const VisualObjectList& world,
    const int               depth) const
{
    HitRecord rec;

    if(world.hit(r, 0.001f, numeric_limits<float>::max(), rec))
    {
        Ray scattered;
        vec3 attenuation;
        vec3 emitted = rec.mat->emission();
        if (depth < int(ray_max_depth) && rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return emitted + attenuation * get_ray_color(scattered, ray_max_depth, world, depth + 1);
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

void Render::get_render_image(
    const size_t            width,
    const size_t            height,
    const size_t            spp,
    const size_t            ray_max_depth,
    const Camera&           camera,
    const VisualObjectList& world,
    QImage&                 image) const
{
    QProgressDialog progress("Rendering...", "Abort", 0, int(width*height), nullptr);
    progress.setWindowModality(Qt::WindowModal);

    progress.show();

    // Precompute subpixel samples position
    const size_t dimension_size =
        std::max(
            size_t(1),
            static_cast<size_t>(sqrt(spp)));
    const size_t samples = dimension_size * dimension_size;

    SampleGenerator generator(dimension_size);

    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            if (progress.wasCanceled())
            {
                QMessageBox::critical(nullptr, "Rendering aborted", "Rendering aborted");
                return;
            }

            const vec2 pt(x, y);
            const vec2 frame(width, height);

            progress.setValue(int(y * width + x));

            vec3 color(0.0f, 0.0f, 0.0f);
            for (size_t i = 0; i < samples; ++i)
            {
                const vec2 subpixel_pos = generator.next();
                const vec2 uv(
                    (pt.x + subpixel_pos.x) / frame.x,
                    (pt.y + subpixel_pos.y) / frame.y);

                color += get_ray_color(
                    camera.get_ray(uv.x, uv.y), ray_max_depth, world);
            }

            color /= static_cast<float>(samples);
            color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));

            int ir = int(255.0f * color[0]);
            int ig = int(255.0f * color[1]);
            int ib = int(255.0f * color[2]);

            image.setPixel(int(width-1-x), int(height-1-y), QColor(ir, ig, ib).rgb());
        }
    }

    progress.setValue(int(width*height));
}

void Render::get_render_image_thread(
        const size_t width,
        const size_t height,
        const size_t spp,
        const size_t ray_max_depth,
        const Camera &camera,
        const VisualObjectList &world,
        QImage &image)
{
    QProgressDialog progress("Rendering...", "", 0, int((width/64)*(height/64)), nullptr);
    progress.setWindowModality(Qt::WindowModal);

    progress.show();
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

    auto compute =
    [&](size_t x1, size_t x2, size_t y1, size_t y2)
    {
        for (size_t y = y1; y < y2; ++y)
        {
            for (size_t x = x1; x < x2; ++x)
            {
                const vec2 pt(x, y);
                const vec2 frame(width, height);

                vec3 color(0.0f, 0.0f, 0.0f);
                for (size_t i = 0; i < samples; ++i)
                {
                    const vec2 subpixel_pos = generator.next();
                    const vec2 uv(
                        (pt.x + subpixel_pos.x) / frame.x,
                        (pt.y + subpixel_pos.y) / frame.y);

                    color += get_ray_color(
                        camera.get_ray(uv.x, uv.y), ray_max_depth, world);
                }

                color /= static_cast<float>(samples);
                color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));

                int ir = int(255.0f * color[0]);
                int ig = int(255.0f * color[1]);
                int ib = int(255.0f * color[2]);
                image.setPixel(int(width-1-x), int(height-1-y), QColor(ir, ig, ib).rgb());
            }
        }
    };

    for (y0 = 0; y0 < height; y0+=64)
    {
        for (x0 = 0; x0 < width; x0+=64)
        {
            x1 = x0;
            y1 = y0;
            x2 = std::min(x0+64, width);
            y2 = std::min(y0+64, height);

            QFuture<void> future = QtConcurrent::run(compute, x1, x2, y1, y2);
            threads.push_back(future);
        }
    }

    for(size_t i = 0; i < threads.size(); ++i)
    {
        threads.at(i).waitForFinished();
        progress.setValue(int(i));
    }

    progress.setValue(int((width/64)*(height/64)));
}

vec3 Render::random_in_unit_sphere() const
{
    vec3 p;
    do
    {
        p = 2.0f * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
    } while(dot(p, p) >= 1);
    return p;
}
