// Interface.
#include "usualshapes.h"

// Math includes.
#include <glm/glm.hpp>

using namespace glm;

UsualShapes::Sphere::Sphere()
{
}

UsualShapes::Sphere::Sphere(vec3 cen, float r)
  : center(cen)
  , radius(r)
{
}

bool UsualShapes::Sphere::hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const
{
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float d = b*b - 4*a*c;

    if(d > 0)
    {
        float tmp = (-b-sqrt(d))/(2*a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.pointAtParameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
        tmp = (-b+sqrt(d))/(2*a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.pointAtParameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
    }
    return false;
}

