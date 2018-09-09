#include "usualshapes.h"

UsualShapes::UsualShapes()
{

}

UsualShapes::Sphere::Sphere()
{

}

UsualShapes::Sphere::Sphere(glm::vec3 cen, float r):
    center(cen),
    radius(r)
{

}

bool UsualShapes::Sphere::hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const
{
    vec3 oc = r.origin() - center;
    float a = glm::dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float d = b*b - 4*a*c;

    if(d > 0)
    {
        float tmp = (-b-glm::sqrt(b*b-a*c))/a;
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.pointAtParameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
        tmp = (-b+glm::sqrt(b*b-a*c))/a;
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
