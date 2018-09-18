#include "renderer/material.h"

using namespace glm;

namespace
{
    vec3 random_in_unit_sphere()
    {
        vec3 p;
        do
        {
            p = 2.0f * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
        } while(dot(p, p) >= 1);
        return p;
    }

    vec3 reflect(
        const vec3& v,
        const vec3& n)
    {
        return v - 2 * dot(v, n) * n;
    }
}

Lambertian::Lambertian(
    const vec3&         albedo)
  : m_albedo(albedo)
{
}

bool Lambertian::scatter(
    const Ray&          r_in,
    const HitRecord&    rec,
    vec3&               attenuation,
    Ray&                scattered) const
{
    vec3 target = rec.normal + random_in_unit_sphere();
    scattered = Ray(rec.p, target);
    attenuation = m_albedo;
    return true;
}

Metal::Metal(
    const vec3&         albedo,
    const float         fuzz)
  : m_albedo(albedo)
  , m_fuzz(fuzz)
{
}

bool Metal::scatter(
    const Ray&          r_in,
    const HitRecord&    rec,
    vec3&               attenuation,
    Ray&                scattered) const
{
    vec3 reflected = reflect(normalize(r_in.B), rec.normal);
    scattered = Ray(rec.p, reflected + m_fuzz * random_in_unit_sphere());
    attenuation = m_albedo;
    return (dot(scattered.B, rec.normal) > 0.0f);
}

