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

namespace
{
    bool ray_hit_triangle(
        const vec3&                     v0,
        const vec3&                     v1,
        const vec3&                     v2,
        const shared_ptr<Material>&     mat,
        const Ray&                      r,
        float                           tmin,
        float                           tmax,
        HitRecord&                      rec)
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
        rec.mat = mat.get();

        return true;
    }

    bool ray_hit_quad(
        const vec3&                     v0,
        const vec3&                     v1,
        const vec3&                     v2,
        const vec3&                     v3,
        const shared_ptr<Material>&     mat,
        const Ray&                      r,
        float                           tmin,
        float                           tmax,
        HitRecord&                      rec)
    {
        return ray_hit_triangle(v0, v1, v2, mat, r, tmin, tmax, rec)
            || ray_hit_triangle(v0, v2, v3, mat, r, tmin, tmax, rec);
    }
}

bool Triangle::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    return ray_hit_triangle(v0, v1, v2, m_mat, r, tmin, tmax, rec);
}

Quad::Quad(
    const glm::vec3&                v0,
    const glm::vec3&                v1,
    const glm::vec3&                v2,
    const glm::vec3&                v3,
    const shared_ptr<Material>&     mat)
  : v0(v0)
  , v1(v1)
  , v2(v2)
  , v3(v3)
  , m_mat(mat)
{
}

bool Quad::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    return ray_hit_quad(v0, v1, v2, v3, m_mat, r, tmin, tmax, rec);
}

Box::Box(
    const vec3&                     bottom_center,
    const float                     half_width,
    const float                     half_height,
    const float                     half_depth,
    const shared_ptr<Material>&     mat)
  : v0(bottom_center)
  , v1(bottom_center)
  , v2(bottom_center)
  , v3(bottom_center)
  , v4(bottom_center)
  , v5(bottom_center)
  , v6(bottom_center)
  , v7(bottom_center)
  , m_mat(mat)
{
    // Bottom.
    v0.x -= half_width; // front left
    v0.z += half_depth;
    v1.x -= half_width; // back left
    v1.z -= half_depth;
    v2.x += half_width; // back right
    v2.z -= half_depth;
    v3.x += half_width; // front right
    v3.z += half_depth;

    // Top.
    v4.y += 2.0f * half_height;
    v5.y += 2.0f * half_height;
    v6.y += 2.0f * half_height;
    v7.y += 2.0f * half_height;
    v4.x -= half_width; // front left
    v4.z += half_depth;
    v5.x -= half_width; // back left
    v5.z -= half_depth;
    v6.x += half_width; // back right
    v6.z -= half_depth;
    v7.x += half_width; // front right
    v7.z += half_depth;
}

bool Box::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    // Take the closest hit.
    float closest_so_far = tmax;

    HitRecord tmp_rec;

    // Front.
    if (ray_hit_quad(v0, v3, v7, v4, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    // Left.
    if (ray_hit_quad(v0, v1, v5, v4, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    // Right.
    if (ray_hit_quad(v3, v2, v6, v7, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    // Back.
    if (ray_hit_quad(v1, v2, v6, v5, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    // Bottom.
    if (ray_hit_quad(v0, v3, v2, v1, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    // Top.
    if (ray_hit_quad(v4, v7, v6, v5, m_mat, r, tmin, tmax, tmp_rec)
        && tmp_rec.t < closest_so_far)
    {
        rec = tmp_rec;
        closest_so_far = tmp_rec.t;
    }

    return closest_so_far != tmax;
}

