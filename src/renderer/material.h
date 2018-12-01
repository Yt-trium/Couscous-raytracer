#ifndef RENDERER_MATERIAL_H
#define RENDERER_MATERIAL_H

// glm includes.
#include <glm/glm.hpp>

class Material
{
  public:
    Material(
        const glm::vec3&    albedo,
        const glm::vec3&    emission,
        const float         kd,
        const float         ks,
        const float         specularExponent);

    float brdf() const;

    const glm::vec3   albedo;
    const glm::vec3   emission;
    const float       kd;
    const float       ks;
    const float       specularExponent;
    const bool        light;

  private:
    const float       m_rf;
};

#endif // RENDERER_MATERIAL_H
