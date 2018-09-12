// Interface.
#include "render.h"

// Math includes.
#include <glm/glm.hpp>

// Standard includes.
#include <limits>

using namespace glm;
using namespace std;

Render::Render()
{
}

vec3 Render::get_ray_color(
    const Ray&          r,
    VisualObjectList    world) const
{
    HitRecord rec;
    float t;

    if(world.hit(r, 0.0f, numeric_limits<float>::max(), rec))
    {
        return 0.5f * vec3(rec.normal.x+1, rec.normal.y+1, rec.normal.z+1);
    }
    /*
    float t;
    t = rayHitSphere(vec3(0,0,-1), 0.5, r);
    if(t > 0.0)
    {
        vec3 N = glm::normalize(r.pointAtParameter(t) - vec3(0, 0, -1));
        return float(0.5) * vec3(N.x+1, N.y+1, N.z+1);
    }
    */

    vec3 unit_direction = normalize(r.direction());
    t = 0.5f * (unit_direction.y + 1.0f);
    return  float(1.0f-t) * vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
}

float Render::ray_hit_sphere(
    const vec3& center,
    float       radius,
    const Ray&  r)
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

QImage Render::get_render_image(
    size_t &width,
    size_t &height,
    size_t &samples,
    Camera &camera,
    VisualObjectList &world) const
{
    size_t x, y, s;
    QImage image(int(width), int(height), QImage::Format_RGB32);

    for(x=0;x<width;x++)
    {
        for(y=0;y<height;y++)
        {
            vec3 color(0.0f, 0.0f, 0.0f);
            for(s=0;s<samples;s++)
            {
                float u = float(x+drand48()) / float(width);
                float v = float(y+drand48()) / float(height);
                color += get_ray_color(camera.get_ray(u, v), world);
            }
            color /= float(samples);

            // color += render.getRayColor(camera.get_ray(u, v), world);

            int ir = int(255.0f * color[0]);
            int ig = int(255.0f * color[1]);
            int ib = int(255.0f * color[2]);
            image.setPixel(width-1-x, height-1-y, QColor(ir, ig, ib).rgb());
        }
    }

    return image;
}
