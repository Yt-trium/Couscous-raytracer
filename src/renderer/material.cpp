// Interface.
#include "material.h"

// Standard includes.
#include <cmath>

using namespace glm;
using namespace std;

Material::Material(
    const vec3&    albedo,
    const vec3&    emission,
    const float    kd,
    const float    ks,
    const float    specularExponent)
  : albedo(albedo)
  , emission(emission)
  , kd(kd)
  , ks(ks)
  , specularExponent(specularExponent)
  , m_rf(max(albedo.x, max(albedo.y, albedo.z)))
{
}

float Material::brdf() const
{
    return m_rf;
}

