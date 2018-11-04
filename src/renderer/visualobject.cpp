// Interface.
#include "renderer/material.h"
#include "renderer/visualobject.h"

// Standard includes.
#include <algorithm>
#include <cassert>
#include <cmath>
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
    const float         tmin,
    const float         tmax,
    HitRecord&          rec)
{
    bool hit_something = false;

    rec.t = tmax;

    for (size_t i = 0; i < world.size(); i++)
    {
        const VisualObject* object = world[i].get();
        hit_something |= object->hit(r, tmin, rec.t, rec);
    }

    return hit_something;
}

MeshGroup fetch_lights(const MeshGroup& world)
{
    MeshGroup lights;

    auto filter_light = [&](const shared_ptr<VisualObject>& mesh)
    {
        // Is it a light ?
        auto mat = mesh->mat();

        if (mat->emission() != vec3(0.0f))
            lights.push_back(mesh);
    };

    for_each(world.begin(), world.end(), filter_light);

    return lights;
}


//
// Usual shapes implementation.
//

Sphere::Sphere(
    const glm::vec3&                center,
    float                           radius,
    const shared_ptr<Material>&     mat)
  : m_center(center)
  , m_radius(radius)
  , m_bbox(center - radius, center + radius)
  , m_mat(mat)
{
}

bool Sphere::hit(
    const Ray&                      r,
    const float                     tmin,
    const float                     tmax,
    HitRecord&                      rec) const
{
    vec3 oc = r.origin - m_center;

    float a = dot(r.dir, r.dir);
    float b = 2.0f * dot(oc, r.dir);
    float c = dot(oc, oc) - m_radius * m_radius;
    float d = b * b - 4.0f * a * c;

    if(d > 0)
    {
        // Solution 1.
        float tmp = (-b - sqrt(d)) / (2.0f * a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.point(rec.t);
            rec.normal = (rec.p - m_center) / m_radius;
            rec.mat = m_mat.get();
            return true;
        }

        // Solution 2.
        tmp = (-b + sqrt(d)) / (2.0f * a);
        if(tmp < tmax && tmp > tmin)
        {
            rec.t = tmp;
            rec.p = r.point(rec.t);
            rec.normal = (rec.p - m_center) / m_radius;
            rec.mat = m_mat.get();
            return true;
        }
    }

    return false;
}

const AABB& Sphere::bbox() const
{
    return m_bbox;
}

const std::shared_ptr<Material>& Sphere::mat() const
{
    return m_mat;
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

std::shared_ptr<Material> TriangleMesh::getMaterial()
{
    return m_mat;
}

void TriangleMesh::getTriangleVertices(int triangleindice, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
{
    v1 = m_vertices[m_indices[triangleindice]];
    v2 = m_vertices[m_indices[triangleindice+1]];
    v3 = m_vertices[m_indices[triangleindice+2]];
}

size_t TriangleMesh::getTriangleCount()
{
    return m_triangle_count;
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

    const vec3& v0 = m_mesh->m_vertices[*m_indices];
    const vec3& v1 = m_mesh->m_vertices[*(m_indices + 1)];
    const vec3& v2 = m_mesh->m_vertices[*(m_indices + 2)];

    m_bbox = AABB(v0);
    m_bbox.add_point(v1);
    m_bbox.add_point(v2);
}

bool Triangle::hit(
    const Ray&                      r,
    const float                     tmin,
    const float                     tmax,
    HitRecord&                      rec) const
{
    static const float epsilon = 0.0001f;

    const vec3& v0 = m_mesh->m_vertices[*m_indices];
    const vec3& v1 = m_mesh->m_vertices[*(m_indices + 1)];
    const vec3& v2 = m_mesh->m_vertices[*(m_indices + 2)];

#ifdef COMPUTE_NORMALS_ON_FLY
    const vec3 v0v1 = v1 - v0;
    const vec3 v0v2 = v2 - v0;
    const vec3 normal = cross(v0v1, v0v2);
#else
    const vec3& normal = m_normal;
#endif

    // 1.
    // Find the intersection point with the ray and the plane lying on
    // the triangle.

    // Parallel test between the plane and the ray.
    const float n_dot_ray_dir = dot(normal, r.dir);

    // Near 0.
    if (fabs(n_dot_ray_dir) < epsilon)
        return false;

    // Compute the parameter d that gives the direction to P.
    const float d = dot(normal, v0);

    // Compute the parameter t that satisfies
    // origin + t * dir = P
    const float t = - (dot(normal, r.origin) - d) / n_dot_ray_dir;

    // Is the triangle behind the ray ?
    if (t < 0.0f || t >= tmax || t < tmin)
        return false;

    // Calculate Ray/Plane intersection point.
    const vec3 p = r.origin + t * r.dir;

    // 2.
    // Check if the point P is inside the triangle.
    vec3 c;

    // First edge.
    const vec3& edge0 = v1 - v0;
    const vec3 vp0 = p - v0;
    c = cross(edge0, vp0);
    if (dot(normal, c) < 0.0f)
        return false;

    // Second ege.
    const vec3 edge1 = v2 - v1;
    const vec3 vp1 = p - v1;
    c = cross(edge1, vp1);
    if (dot(normal, c) < 0.0f)
        return false;

    // Third ege.
    const vec3 edge2 = v0 - v2;
    const vec3 vp2 = p - v2;
    c = cross(edge2, vp2);
    if (dot(normal, c) < 0.0f)
        return false;

#ifdef COMPUTE_NORMALS_ON_FLY
    rec.normal = normalize(normal);
#else
    rec.normal = normal;
#endif

    rec.mat = m_mesh->m_mat.get();
    rec.p = p;
    rec.t = t;

    return true;
}

const AABB& Triangle::bbox() const
{
    return m_bbox;
}

const std::shared_ptr<Material>& Triangle::mat() const
{
    return m_mesh->m_mat;
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

#include <iostream>
// Procedurally generate a sphere.
// Poles are not considered as subidivisions.
void create_cylinder(
    MeshGroup&                      world,
    const shared_ptr<Material>&     material,
    const size_t                    subidivisions,
    const float                     height,
    const float                     width,
    const bool                      caps,
    const mat4&                     transform)
{
    assert(subidivisions >= 3);
    assert(height >= 0.0f && width >= 0.0f);

    const float radius = width * 0.5f;
    const float hheight = height * 0.5f;

    vector<vec3> vertices, normals;
    vector<size_t> indices;
    size_t face_count = 0;

    const float angle_step = (-2.0f * M_PI) / static_cast<float>(subidivisions);

    // Initial points.
    vertices.emplace_back(radius, -hheight, 0.0f);
    vertices.emplace_back(radius, hheight, 0.0f);

    // Create vertices and indices.
    for (size_t i = 1; i < subidivisions; ++i)
    {
        const float angle = static_cast<float>(i) * angle_step;

        // X and Z depends on the angle.
        const float x = radius * cos(angle);
        const float z = radius * sin(angle);

        // Create points;
        vertices.emplace_back(x, -hheight, z);
        vertices.emplace_back(x, hheight, z);

        // Create the face.
        const size_t index = i * 2;
        indices.push_back(index - 2);
        indices.push_back(index);
        indices.push_back(index + 1);
        indices.push_back(index - 2);
        indices.push_back(index + 1);
        indices.push_back(index - 1);
        face_count += 2;

        // Create normals.
        const vec3 ab = vertices[index] - vertices[index - 2];
        const vec3 ac = vertices[index - 1] - vertices[index - 2];
        const vec3 normal = normalize(cross(ab, ac));
        normals.emplace_back(normal);
        normals.emplace_back(normal);
    }

    // Create the welding face.
    const size_t index = subidivisions * 2 - 2;
    indices.push_back(index);
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(index);
    indices.push_back(1);
    indices.push_back(index + 1);
    const vec3 ab = vertices[0] - vertices[index];
    const vec3 ac = vertices[index + 1] - vertices[index];
    const vec3 normal = normalize(cross(ab, ac));
    normals.emplace_back(normal);
    normals.emplace_back(normal);
    face_count += 2;

    if (caps)
    {
        // Poke faces with one vertice for each cap.
        vertices.emplace_back(0.0f, hheight, 0.0f);
        vertices.emplace_back(0.0f, -hheight, 0.0f);

        const size_t top_index = vertices.size() - 2;
        const size_t bottom_index = vertices.size() - 1;

        const vec3 top_normal(0.0f, 1.0f, 0.0f);
        const vec3 bottom_normal(0.0f, -1.0f, 0.0f);

        // Turn around the cylinder.
        for (size_t i = 0; i < subidivisions; ++i)
        {
            // Bottom triangle.
            indices.emplace_back(bottom_index);
            indices.emplace_back(i * 2 + 2);
            indices.emplace_back(i * 2);
            normals.push_back(bottom_normal);

            // Top triangle.
            indices.emplace_back(top_index);
            indices.emplace_back(i * 2 + 1);
            indices.emplace_back(i * 2 + 3);
            normals.push_back(top_normal);

            face_count += 2;
        }

        // Welding triangles.
        indices.emplace_back(bottom_index);
        indices.emplace_back(0);
        indices.emplace_back(index);
        indices.emplace_back(top_index);
        indices.emplace_back(index + 1);
        indices.emplace_back(1);
        normals.push_back(bottom_normal);
        normals.push_back(top_normal);
        face_count += 2;
    }

    create_triangle_mesh(
        world,
        face_count,
        vertices.size(),
        indices.data(),
        vertices.data(),
        normals.data(),
        material,
        transform);
}

