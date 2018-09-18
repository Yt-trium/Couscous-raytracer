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
}

vec3 Material::emission() const
{
    return vec3(0.0f);
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
    vec3 reflected = reflect(normalize(r_in.dir), rec.normal);
    scattered = Ray(rec.p, reflected + m_fuzz * random_in_unit_sphere());
    attenuation = m_albedo;
    return (dot(scattered.dir, rec.normal) > 0.0f);
}

Light::Light(
    const vec3&         emission)
  : m_emission(emission)
{
}

bool Light::scatter(
    const Ray&          r_in,
    const HitRecord&    rec,
    vec3&               attenuation,
    Ray&                scattered) const
{
    return false;
}

vec3 Light::emission() const
{
    return m_emission;
}

