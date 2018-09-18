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
    const VisualObjectList& world,
    const int               depth) const
{
    HitRecord rec;

    if(world.hit(r, 0.001f, numeric_limits<float>::max(), rec))
    {
        Ray scattered;
        vec3 attenuation;
        vec3 emitted = rec.mat->emission();
        if (depth < 10 && rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return emitted + attenuation * get_ray_color(scattered, world, depth + 1);
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

float Render::ray_hit_sphere(
    const vec3&             center,
    float                   radius,
    const Ray&              r)
{
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0f * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float d = b*b - 4*a*c;

    if(d < 0)
        return -1.0;
    else
        return (-b-sqrt(d))/(2.0f*a);
}


void Render::get_render_image(
    const size_t            width,
    const size_t            height,
    const size_t            spp,
    const Camera&           camera,
    const VisualObjectList& world,
    QImage&                 image) const
{
    QProgressDialog progress("Rendering...", "Abort", 0, int(width*height), 0);
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

            progress.setValue(y * width + x);

            vec3 color(0.0f, 0.0f, 0.0f);
            for (size_t i = 0; i < samples; ++i)
            {
                const vec2 subpixel_pos = generator.next();
                const vec2 uv(
                    (pt.x + subpixel_pos.x) / frame.x,
                    (pt.y + subpixel_pos.y) / frame.y);

                color += get_ray_color(
                    camera.get_ray(uv.x, uv.y), world);
            }

            color /= static_cast<float>(samples);
            color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));

            int ir = int(255.0f * color[0]);
            int ig = int(255.0f * color[1]);
            int ib = int(255.0f * color[2]);

            image.setPixel(width-1-x, height-1-y, QColor(ir, ig, ib).rgb());
        }
    }

    progress.setValue(width*height);
}

void Render::get_render_image_thread(const size_t width, const size_t height, const size_t spp, const Camera &camera, const VisualObjectList &world, QImage &image)
{
    QProgressDialog progress("Rendering...", "Abort", 0, int(width*height), 0);
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
    std::vector<QFuture<void>> threads;

    auto compute =
    [&]()
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
                        camera.get_ray(uv.x, uv.y), world);
                }

                color /= static_cast<float>(samples);
                color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2]));

                int ir = int(255.0f * color[0]);
                int ig = int(255.0f * color[1]);
                int ib = int(255.0f * color[2]);
                image.setPixel(width-1-x, height-1-y, QColor(ir, ig, ib).rgb());
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

            /* [world, x1, x2, y1, y2, &image, samples,
                     width, height, camera, this]*/


            QFuture<void> future = QtConcurrent::run(compute);
            threads.push_back(future);
            threads.back().waitForFinished();
        }
    }

    for(std::size_t i = 0; i < threads.size(); ++i)
    {
        threads.at(i).waitForFinished();
        qDebug() << i;
    }

    progress.setValue(width*height);
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
