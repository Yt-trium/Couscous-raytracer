// Interface.
#include "renderer/visualobject.h"
#include "renderer/material.h"

using namespace glm;
using namespace std;

//
// 3D Object data structures implementation.
//

VisualObjectList::VisualObjectList()
{
}

void VisualObjectList::add(VisualObject* object)
{
    m_object_list.emplace_back(object);
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

    for (i = 0; i < m_object_list.size(); i++)
    {
        const VisualObject* object = m_object_list[i].get();

        if (object->hit(r, tmin, closest_so_far, tmp_rec))
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
    const glm::vec3&                center,
    float                           radius,
    const shared_ptr<Material>&     mat)
  : center(center)
  , radius(radius)
  , m_mat(mat)
{
}

bool Sphere::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    vec3 oc = r.origin - center;

    float a = dot(r.dir, r.dir);
    float b = 2.0f * dot(oc, r.dir);
    float c = dot(oc, oc) - radius * radius;
    float d = b * b - 4.0f * a * c;

    if(d > 0)
    {
        float tmp = (-b-sqrt(d))/(2*a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.point(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat = m_mat.get();
            return true;
        }
        tmp = (-b+sqrt(d))/(2*a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.point(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat = m_mat.get();
            return true;
        }
    }

    return false;
}


Triangle::Triangle(
    const glm::vec3&                v0,
    const glm::vec3&                v1,
    const glm::vec3&                v2,
    const shared_ptr<Material>&     mat)
  : v0(v0)
  , v1(v1)
  , v2(v2)
  , m_mat(mat)
{
}

bool Triangle::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    static const float epsilon = 0.001f;

    // Compute triangle's normal.
    const vec3 v0v1 = v1 - v0;
    const vec3 v0v2 = v2 - v0;
    rec.normal = cross(v0v1, v0v2);

    // 1.
    // Find the intersection point with the ray and the plane lying on
    // the triangle.

    // Parallel test between the plane and the ray.
    const float n_dot_ray_dir = dot(rec.normal, r.dir);

    // Near 0.
    if (fabs(n_dot_ray_dir) < epsilon)
        return false;

    // Compute the parameter d that gives the direction to P.
    const float d = dot(rec.normal, v0);

    // Compute the parameter t that satisfies
    // origin + t * dir = P
    rec.t = - (dot(rec.normal, r.origin) - d) / n_dot_ray_dir;

    // Is the triangle behind the ray ?
    if (rec.t < 0.0f || rec.t > tmax || rec.t < tmin)
        return false;

    // Calculate Ray/Plane intersection point.
    rec.p = r.origin + rec.t * r.dir;

    // 2.
    // Check if the point P is inside the triangle.
    vec3 c;

    // First edge.
    const vec3& edge0 = v0v1;
    const vec3 vp0 = rec.p - v0;
    c = cross(edge0, vp0);
    if (dot(rec.normal, c) < 0.0f)
        return false;

    // Second ege.
    const vec3 edge1 = v2 - v1;
    const vec3 vp1 = rec.p - v1;
    c = cross(edge1, vp1);
    if (dot(rec.normal, c) < 0.0f)
        return false;

    // Third ege.
    const vec3 edge2 = v0 - v2;
    const vec3 vp2 = rec.p - v2;
    c = cross(edge2, vp2);
    if (dot(rec.normal, c) < 0.0f)
        return false;

    rec.normal = normalize(rec.normal);
    rec.mat = m_mat.get();

    return true;
}
