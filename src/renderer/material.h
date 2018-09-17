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
    Material(
        glm::vec3           ambient = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3           diffuse = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3           specular = glm::vec3(0.0f, 0.0f, 0.0f),
        float               shininess = 0.0f);

    bool scatter(
        const Ray&          r_in,
        const HitRecord&    rec,
        Ray&                scattered);

    glm::vec3 reflect(
        const glm::vec3&    v,
        const glm::vec3&    n);

    static Material lambertian();
    static Material metal();

  private:
    glm::vec3   ambient;
    glm::vec3   diffuse;
    glm::vec3   specular;
    float       shininess;

    static glm::vec3 random_in_unit_sphere();
};

#endif // RENDERER_MATERIAL_H
