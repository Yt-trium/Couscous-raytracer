// Interface.
#include "renderer/visualobject.h"

using namespace glm;
using namespace std;

//
// 3D Object data structures implementation.
//

VisualObjectList::VisualObjectList()
{
}

VisualObjectList::VisualObjectList(vector<VisualObject*> l)
  : object_list(l)
{
}

bool VisualObjectList::hit(
    const Ray&  r,
    float       tmin,
    float       tmax,
    HitRecord&  rec) const
{
    HitRecord tmp_rec;
    bool hit_something = false;
    float closest_so_far = tmax;
    size_t i;

    for (i = 0; i < object_list.size(); i++)
    {
        if (object_list[i]->hit(r, tmin, closest_so_far, tmp_rec))
        {
            hit_something = true;
            closest_so_far = tmp_rec.t;
            rec = tmp_rec;
        }
    }

    return hit_something;
}


//
// Usual shapes implementation.
//

Sphere::Sphere(
    const glm::vec3&    center,
    float               radius)
  : center(center)
  , radius(radius)
{
}

bool Sphere::hit(
    const Ray&          r,
    float               tmin,
    float               tmax,
    HitRecord&          rec) const
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


Triangle::Triangle(
    const float v1,
    const float v2,
    const float v3)
  : v1(v1)
  , v2(v2)
  , v3(v3)
{
}

bool Triangle::hit(
    const Ray&          r,
    float               tmin,
    float               tmax,
    HitRecord&          rec) const
{
    return false;
}
