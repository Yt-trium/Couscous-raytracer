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

#define COUCOUS_M_PI 3.1416f

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

    auto filter_light = [&](const Shape& mesh)
    {
        // Is it a light ?
        auto mat = mesh->mat();

        if (mat->emission != vec3(0.0f))
            lights.push_back(mesh);
    };

    for_each(world.begin(), world.end(), filter_light);

    return lights;
}


//
// Usual shapes implementation.
//

TriangleMesh::TriangleMesh(
    const size_t                    triangle_count,
    const size_t                    vertices_count,
    const size_t*                   indices,
    const vec3*                     vertices,
    const vec3*                     normals,
    const shared_ptr<Material>&     material,
    const mat4&                     transform,
    const bool                      smooth_shading)
  : m_triangle_count(triangle_count)
  , m_vertices_count(vertices_count)
  , m_mat(material)
  , m_smooth_shading(smooth_shading)
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
    const size_t normal_count = smooth_shading ? m_vertices_count : m_triangle_count;
    m_normals.reset(new vec3[normal_count]);
    for (size_t i = 0; i < normal_count; ++i)
    {
        m_normals[i] = normalize(normals_transform * normals[i]);
    }
}

std::shared_ptr<Material> TriangleMesh::getMaterial()
{
    return m_mat;
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
  , m_triangle_indice(indice)
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
    const vec3& v0 = m_mesh->m_vertices[*m_indices];
    const vec3& v1 = m_mesh->m_vertices[*(m_indices + 1)];
    const vec3& v2 = m_mesh->m_vertices[*(m_indices + 2)];
    const vec3 v0v1 = v1 - v0;
    const vec3 v0v2 = v2 - v0;

    const vec3 h = cross(r.dir, v0v2);
    const float a = dot(v0v1, h);

    // Parallel or behind ?
    if (a <= 0.0f)
        return false;

    const float f = 1.0f / a;
    const vec3 s = r.origin - v0;
    const float u = f * dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    const vec3 q = cross(s, v0v1);
    const float v = f * dot(r.dir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    const float t = f * dot(v0v2, q);

    if (t <= 0.0f || t >= tmax || t < tmin)
        return false;

    if (!m_mesh->m_smooth_shading)
    {
        rec.normal = m_mesh->m_normals[m_triangle_indice];
    }
    else
    {
        // Interpolate normals.
        const vec3& n0 = m_mesh->m_normals[*m_indices];
        const vec3& n1 = m_mesh->m_normals[*(m_indices + 1)];
        const vec3& n2 = m_mesh->m_normals[*(m_indices + 2)];
        const float w = 1.0f - u - v;
        rec.normal = u * n1 + v * n2 + w * n0;
    }

    //const vec3 normal = normalize(cross(v0v1, v0v2));
    //const vec3& normal = m_normal;
    //const vec3& v0 = m_mesh->m_vertices[*m_indices];
    rec.mat = m_mesh->m_mat.get();
    rec.p = r.origin + r.dir * t;
    rec.t = t;
    rec.triangle = this;

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

const vec3& Triangle::vertice(const size_t indice) const
{
    assert(indice == 0 || indice == 1 || indice == 2);
    return m_mesh->m_vertices[*(m_indices + indice)];
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
    const mat4&                 transform,
    const bool                  smooth_shading)
{
    shared_ptr<TriangleMesh> mesh = make_shared<TriangleMesh>(
        triangle_count,
        vertices_count,
        indices,
        vertices,
        normals,
        material,
        transform,
        smooth_shading);

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

    const float angle_step = (-2.0f * COUCOUS_M_PI) / static_cast<float>(subidivisions);

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

