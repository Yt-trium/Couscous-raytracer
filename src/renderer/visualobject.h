#ifndef RENDERER_VISUALOBJECT_H
#define RENDERER_VISUALOBJECT_H

// couscous includes.
#include "ray.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <vector>

//
// Ray intersections data structures.
//

// Store informations about an intersection between a ray and an object.
typedef struct HitRecord
{
    float       t;
    glm::vec3   p;
    glm::vec3   normal;
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
    VisualObjectList(std::vector<VisualObject*> l);

    bool hit(
        const Ray&  r,
        float       tmin,
        float       tmax,
        HitRecord&  rec) const override;

    std::vector<VisualObject*> object_list;
};


//
// Usual shapes.
//

class Sphere : public VisualObject
{
  public:
    Sphere(
        const glm::vec3&    center = glm::vec3(0.0f),
        float               radius = 1.0f);

    bool hit(
        const Ray&          r,
        float               tmin,
        float               tmax,
        HitRecord&          rec) const override;

    glm::vec3   center;
    float       radius;
};

class Triangle : public VisualObject
{
  public:
    Triangle(
        const glm::vec3&    v0,
        const glm::vec3&    v1,
        const glm::vec3&    v2);

    // Möller-Trumbore algorithm.
    bool hit(
        const Ray&          r,
        float               tmin,
        float               tmax,
        HitRecord&          rec) const override;

    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

#endif // VISUALOBJECT_H
