#ifndef RENDERER_MATERIAL_H
#define RENDERER_MATERIAL_H

// couscous includes.
#include "renderer/ray.h"
#include "renderer/visualobject.h"

// glm includes.
#include <glm/glm.hpp>

class Material
{
  public:
    virtual bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        glm::vec3&          attenuation,
        Ray&                scattered) const = 0;
};

class Lambertian : public Material
{
  public:
    Lambertian(const glm::vec3& albedo);

    bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        glm::vec3&          attenuation,
        Ray&                scattered) const override;

  private:
    glm::vec3 m_albedo;
};

class Metal : public Material
{
  public:
    Metal(const glm::vec3& albedo);

    bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        glm::vec3&          attenuation,
        Ray&                scattered) const override;

  private:
    glm::vec3 m_albedo;
};

#endif // RENDERER_MATERIAL_H
