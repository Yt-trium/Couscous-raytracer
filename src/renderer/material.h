#ifndef RENDERER_MATERIAL_H
#define RENDERER_MATERIAL_H

// glm includes.
#include <glm/glm.hpp>

class Material
{
  public:
    Material(
        const glm::vec3&    albedo,
        const float         light_power,
        const float         kd,
        const float         ks,
        const float         specularExponent,
        const float         metal,
        const float         rougness);

    float brdf() const;

    const glm::vec3     albedo;
    const float         light_power;
    const float         kd;
    const float         ks;
    const float         specularExponent;
    const float         metal;
    const float         roughness;
    const bool          metallic;
    const bool          light;
    const glm::vec3     emission;

  private:
    const float         m_rf;
};

#endif // RENDERER_MATERIAL_H
