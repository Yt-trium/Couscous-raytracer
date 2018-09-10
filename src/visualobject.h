#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

#include "ray.h"

class HitRecord
{
public:
    float t;
    vec3 p;
    vec3 normal;
};

class VisualObject
{
public:
    virtual bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const = 0;
};

#endif // VISUALOBJECT_H
