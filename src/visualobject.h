#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

// couscous includes.
#include "renderer/ray.h"

// Math includes.
#include <glm/vec3.hpp>

class HitRecord
{
  public:
    float       t;
    glm::vec3   p;
    glm::vec3   normal;
};

class VisualObject
{
  public:
    virtual bool hit(
        const Ray& r,
        float tmin,
        float tmax,
        HitRecord& rec) const = 0;
};

#endif // VISUALOBJECT_H
