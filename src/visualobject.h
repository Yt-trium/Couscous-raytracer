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
    virtual ~VisualObject() {}
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const = 0;
};

#endif // VISUALOBJECT_H
