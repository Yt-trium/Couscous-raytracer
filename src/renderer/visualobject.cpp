// Interface.
#include "renderer/visualobject.h"
#include "renderer/material.h"

// Standard includes.
#include <algorithm>
#include <cassert>
#include <iterator>

using namespace glm;
using namespace std;

// Uncomment this to compute shapes normal at intersection.
// #define COMPUTE_NORMALS_ON_FLY

//
// 3D Object data structures implementation.
//

bool hit_world(
    const MeshGroup&    world,
    const Ray&          r,
    float               tmin,
    float               tmax,
    HitRecord&          rec)
{
    HitRecord tmp_rec;
    bool hit_something = false;
    float closest_so_far = tmax;
    size_t i;

    for (i = 0; i < world.size(); i++)
    {
        const VisualObject* object = world[i].get();

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

TriangleMesh::TriangleMesh(
    const size_t                    triangle_count,
    const size_t                    vertices_count,
    const size_t*                   indices,
    const vec3*                     vertices,
    const vec3*                     normals,
    const shared_ptr<Material>&     material,
    const mat4&                     transform)
  : m_triangle_count(triangle_count)
  , m_vertices_count(vertices_count)
  , m_mat(material)
{
    assert(indices);
    assert(vertices);
    assert(normals);

    // Copy indices.
    m_indices.reserve(triangle_count * 3);
    copy(indices, &indices[m_triangle_count * 3], back_inserter(m_indices));

    // Copy vertices.
    m_vertices.reset(new vec3[m_vertices_count]);
    for (size_t i = 0; i < vertices_count; ++i)
    {
        m_vertices[i] = transform * vec4(vertices[i], 1.0f);
    }

    const mat3 normals_transform = transpose(inverse(transform));

    // Copy normals.
    m_normals.reset(new vec3[m_triangle_count]);
    for (size_t i = 0; i < m_triangle_count; ++i)
    {
        m_normals[i] = normalize(normals_transform * normals[i]);
    }
}

Triangle::Triangle(
    const shared_ptr<TriangleMesh>&     mesh,
    const size_t                        indice)
  : m_mesh(mesh)
  , m_indices(&mesh->m_indices[3 * indice])
  , m_normal(mesh->m_normals[indice])
{
    assert(*m_indices == m_mesh->m_indices[3 * indice]);
    assert(*(m_indices + 1) == m_mesh->m_indices[3 * indice + 1]);
    assert(*(m_indices + 2) == m_mesh->m_indices[3 * indice + 2]);
}

bool Triangle::hit(
    const Ray&                      r,
    float                           tmin,
    float                           tmax,
    HitRecord&                      rec) const
{
    static const float epsilon = 0.0001f;

    const vec3& v0 = m_mesh->m_vertices[*m_indices];
    const vec3& v1 = m_mesh->m_vertices[*(m_indices + 1)];
    const vec3& v2 = m_mesh->m_vertices[*(m_indices + 2)];

#ifdef COMPUTE_NORMALS_ON_FLY
    const vec3 v0v1 = v1 - v0;
    const vec3 v0v2 = v2 - v0;
    rec.normal = cross(v0v1, v0v2);
#else
    rec.normal = m_normal;
#endif

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
    const vec3& edge0 = v1 - v0;
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

#ifdef COMPUTE_NORMALS_ON_FLY
    rec.normal = normalize(rec.normal);
#endif

    rec.mat = m_mesh->m_mat.get();

    return true;
}


//
// Mesh generation implementation.
//

void create_triangle_mesh(
    MeshGroup&                  world,
    const size_t                triangle_count,
    const size_t                vertices_count,
    const size_t*               indices,
    const vec3*                 vertices,
    const vec3*                 normals,
    const shared_ptr<Material>& material,
    const mat4&                 transform)
{
    shared_ptr<TriangleMesh> mesh = make_shared<TriangleMesh>(
        triangle_count,
        vertices_count,
        indices,
        vertices,
        normals,
        material,
        transform);

    for (size_t i = 0; i < triangle_count; ++i)
    {
        world.push_back(make_shared<Triangle>(mesh, i));
    }
}

// Create a cube.
//
//     7----------6
//    /|         /|
//   / |        / |
//  /  |       /  |
//  /  |      /   |
// 3---------2    |
// |   |     |    |
// |   4-----|----5
// |  /      |   /
// | /       |  /
// |/        | /
// 0 ------- 1
//
//
void create_cube(
    MeshGroup&                      world,
    const shared_ptr<Material>&     material,
    const mat4&                     transform)
{
    // Create vertices.
    const vector<vec3> vertices = {
        vec3(-0.5f, -0.5f, 0.5f),
        vec3(0.5f, -0.5f, 0.5f),
        vec3(0.5f, 0.5f, 0.5f),
        vec3(-0.5f, 0.5f, 0.5f),
        vec3(-0.5f, -0.5f, -0.5f),
        vec3(0.5f, -0.5f, -0.5f),
        vec3(0.5f, 0.5f, -0.5f),
        vec3(-0.5f, 0.5f, -0.5f)
    };

    // Create indices.
    const vector<size_t> indices = {
        // Bottom.
        0, 4, 1,
        1, 4, 5,
        // Front.
        0, 1, 2,
        0, 2, 3,
        // Back.
        4, 7, 6,
        4, 6, 5,
        // Right.
        1, 6, 2,
        1, 5, 6,
        // Left.
        0, 3, 4,
        4, 3, 7,
        // Top.
        3, 2, 6,
        3, 6, 7
    };

    // Create normals.
    const vector<vec3> normals = {
        vec3(0.0f, -1.0f, 0.0f),
        vec3(0.0f, -1.0f, 0.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f, -1.0f),
        vec3(0.0f, 0.0f, -1.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(-1.0f, 0.0f, 0.0f),
        vec3(-1.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    create_triangle_mesh(
        world,
        12,
        8,
        indices.data(),
        vertices.data(),
        normals.data(),
        material,
        transform);
}

// Create a plane.
void create_plane(
    MeshGroup&                      world,
    const shared_ptr<Material>&     material,
    const mat4&                     transform)
{
    // Create vertices.
    const vector<vec3> vertices = {
        vec3(-0.5f, 0.0f, 0.5f),
        vec3(0.5f, 0.0f, 0.5f),
        vec3(0.5f, 0.0f, -0.5f),
        vec3(-0.5f, 0.0f, -0.5f)
    };

    // Create indices.
    const vector<size_t> indices = {
        0, 1, 2,
        0, 2, 3,
    };

    // Create normals.
    const vector<vec3> normals = {
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    create_triangle_mesh(
        world,
        2,
        4,
        indices.data(),
        vertices.data(),
        normals.data(),
        material,
        transform);
}
