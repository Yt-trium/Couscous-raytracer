// Interface.
#include "material.h"

// Standard includes.
#include <cmath>

using namespace glm;
using namespace std;

Material::Material(
    const vec3&    albedo,
    const float    light_power,
    const float    kd,
    const float    ks,
    const float    specularExponent,
    const float    metal,
    const float    roughness)
  : albedo(albedo)
  , light_power(light_power)
  , kd(kd)
  , ks(ks)
  , specularExponent(specularExponent)
  , metal(metal)
  , roughness(roughness)
  , metallic(metal > 0.0f)
  , light(light_power > 0.0f)
  , emission(light_power * albedo)
  , m_rf(max(albedo.x, max(albedo.y, albedo.z)))
{
}

float Material::brdf() const
{
    return m_rf;
}

