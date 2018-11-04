// Interface.
#include "renderer/material.h"

// couscous includes.
#include "renderer/utility.h"

using namespace glm;

namespace
{
    vec3 random_in_unit_sphere()
    {
        vec3 p;
        do
        {
            p = 2.0f * vec3(
                random<float>(),
                random<float>(),
                random<float>()
                ) - vec3(1.0f);
        } while(dot(p, p) >= 1.0f);

        return p;
    }
}

vec3 Material::emission() const
{
    return vec3(0.0f);
}

float Material::brdf() const
{
    return 0.0f;
}

Lambertian::Lambertian(
    const vec3&         albedo)
  : m_albedo(albedo)
{
    m_rf = max<float>(albedo.x, albedo.y);
    m_rf = max<float>(m_rf, albedo.z);
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

float Lambertian::brdf() const
{
    return m_rf;
}

Metal::Metal(
    const vec3&         albedo,
    const float         fuzz)
  : m_albedo(albedo)
  , m_fuzz(fuzz)
{
    m_rf = max<float>(albedo.x, albedo.y);
    m_rf = max<float>(m_rf, albedo.z);
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

float Metal::brdf() const
{
    return m_rf;
}

Light::Light(
    const vec3&         emission)
  : m_emission(emission)
{
    m_rf = 0.0f;
}

bool Light::scatter(
    const Ray&          r_in,
    const HitRecord&    rec,
    vec3&               attenuation,
    Ray&                scattered) const
{
    return false;
}

float Light::brdf() const
{
    return m_rf;
}

vec3 Light::emission() const
{
    return m_emission;
}

