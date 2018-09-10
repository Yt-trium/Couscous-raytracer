#ifndef USUALSHAPES_H
#define USUALSHAPES_H

#include "visualobject.h"

#include <glm/glm.hpp>

namespace UsualShapes
{
    class Sphere:public VisualObject
    {
    public:
        Sphere();
        Sphere(vec3 cen, float r);

        virtual bool hit(const Ray& r, float tmin, float tmax, HitRecord& rec) const;

        vec3 center;
        float radius;
    };
};

#endif // USUALSHAPES_H
