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

    virtual glm::vec3 emission() const;

    virtual float brdf() const;

    virtual float getKD() const;

    virtual float getKS() const;

    virtual float getSpecularExponent() const;

protected:
    float       m_rf;
    float       kd;
    float       ks;
    float       specularExponent;
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

    float brdf() const override;

    glm::vec3 getAlbedo() const;

  private:
    glm::vec3 m_albedo;
};

class Metal : public Material
{
  public:
    Metal(const glm::vec3& albedo, const float fuzz);

    bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        glm::vec3&          attenuation,
        Ray&                scattered) const override;

    float brdf() const override;

    glm::vec3 getAlbedo() const;

  private:
    glm::vec3   m_albedo;
    float       m_fuzz;
};

class Light : public Material
{
  public:
    Light(const glm::vec3& emission);

    bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        glm::vec3&          attenuation,
        Ray&                scattered) const override;

    float brdf() const override;

    virtual float getKD() const override;

    virtual float getKS() const override;

    virtual float getSpecularExponent() const override;

    glm::vec3 emission() const;

  private:
    glm::vec3 m_emission;
};

#endif // RENDERER_MATERIAL_H
