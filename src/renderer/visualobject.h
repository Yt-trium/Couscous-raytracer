#ifndef RENDERER_VISUALOBJECT_H
#define RENDERER_VISUALOBJECT_H

// couscous includes.
#include "renderer/ray.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <memory>
#include <vector>

// Forward declarations.
class Material;

//
// Ray intersections data structures.
//

// Store informations about an intersection between a ray and an object.
typedef struct HitRecord
{
    float       t;
    glm::vec3   p;
    glm::vec3   normal;
    Material*   mat;
} HitRecord;


//
// 3D Object abstract data structures.
//

// A 3D object is something that a ray can intersects.
class VisualObject
{
  public:
    virtual bool hit(
        const Ray&  r,
        float       tmin,
        float       tmax,
        HitRecord&  rec) const = 0;
};

// A list of 3D intersectable objects.
class VisualObjectList : public VisualObject
{
  public:
    VisualObjectList();

    void add(VisualObject* object);

    bool hit(
        const Ray&  r,
        float       tmin,
        float       tmax,
        HitRecord&  rec) const override;

  private:
    std::vector<std::shared_ptr<VisualObject>> m_object_list;
};


//
// Usual shapes.
//

class Sphere : public VisualObject
{
  public:
    Sphere(
        const glm::vec3&                    center,
        float                               radius,
        const std::shared_ptr<Material>&    mat);

    bool hit(
        const Ray&                          r,
        float                               tmin,
        float                               tmax,
        HitRecord&                          rec) const override;

    glm::vec3   center;
    float       radius;

  private:
    std::shared_ptr<Material> m_mat;
};

class Triangle : public VisualObject
{
  public:
    Triangle(
        const glm::vec3&                    v0,
        const glm::vec3&                    v1,
        const glm::vec3&                    v2,
        const std::shared_ptr<Material>&    mat);

    // Möller-Trumbore algorithm.
    bool hit(
        const Ray&                          r,
        float                               tmin,
        float                               tmax,
        HitRecord&                          rec) const override;

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

  private:
    std::shared_ptr<Material> m_mat;
};

class Quad : public VisualObject
{
  public:
    Quad(
        const glm::vec3&                    v0,
        const glm::vec3&                    v1,
        const glm::vec3&                    v2,
        const glm::vec3&                    v3,
        const std::shared_ptr<Material>&    mat);

    bool hit(
        const Ray&                          r,
        float                               tmin,
        float                               tmax,
        HitRecord&                          rec) const override;

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;

  private:
    std::shared_ptr<Material> m_mat;
};

class Box : public VisualObject
{
  public:
    Box(
        const glm::vec3&                    bottom_center,
        const float                         half_width,
        const float                         half_height,
        const float                         half_depth,
        const std::shared_ptr<Material>&    mat);

    bool hit(
        const Ray&                          r,
        float                               tmin,
        float                               tmax,
        HitRecord&                          rec) const override;

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
    glm::vec3 v4;
    glm::vec3 v5;
    glm::vec3 v6;
    glm::vec3 v7;

  private:
    std::shared_ptr<Material> m_mat;
};

#endif // VISUALOBJECT_H
