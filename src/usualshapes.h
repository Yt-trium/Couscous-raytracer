#ifndef USUALSHAPES_H
#define USUALSHAPES_H

// couscous includes.
#include "visualobject.h"

// Math includes.
#include <glm/vec3.hpp>

namespace UsualShapes
{

class Sphere:public VisualObject
{
  public:
    Sphere();
    Sphere(glm::vec3 cen, float r);

    virtual bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const;

    glm::vec3 center;
    float radius;
};

};

#endif // USUALSHAPES_H
