#ifndef RENDERER_VISUALOBJECT_H
#define RENDERER_VISUALOBJECT_H

// couscous includes.
#include "renderer/aabb.h"
#include "renderer/ray.h"

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <memory>
#include <vector>

// Forward declarations.
class Material;

//
// Ray intersections data structures.
//

// Store informations about an intersection between a ray and an object.
typedef struct HitRecord
{
    float       t;
    glm::vec3   p;
    glm::vec3   normal;
    Material*   mat;
} HitRecord;


//
// 3D Object abstract data structures.
//

// A 3D object is something that a ray can intersects.
class VisualObject
{
  public:
    virtual bool hit(
        const Ray&  r,
        const float tmin,
        const float tmax,
        HitRecord&  rec) const = 0;

    virtual const AABB& bbox() const = 0;
};

// A list of 3D intersectable objects.
typedef std::vector<std::shared_ptr<VisualObject>> MeshGroup;

// Test the intersection in the whole world.
bool hit_world(
    const MeshGroup&    world,
    const Ray&          r,
    const float         tmin,
    const float         tmax,
    HitRecord&          rec);


//
// Usual shapes.
//

class Sphere : public VisualObject
{
  public:
    Sphere(
        const glm::vec3&                    center,
        float                               radius,
        const std::shared_ptr<Material>&    mat);

    bool hit(
        const Ray&                          r,
        const float                         tmin,
        const float                         tmax,
        HitRecord&                          rec) const override;

    const AABB& bbox() const override;


  private:
    const glm::vec3             m_center;
    const float                 m_radius;
    const AABB                  m_bbox;
    std::shared_ptr<Material>   m_mat;
};

class Triangle;

// 3D mesh.
// It stores only 1 normal per triangle.
class TriangleMesh
{
    friend class Triangle;

  public:
    TriangleMesh(
        const size_t                        triangle_count,
        const size_t                        vertices_count,
        const size_t*                       indices,
        const glm::vec3*                    vertices,
        const glm::vec3*                    normals,
        const std::shared_ptr<Material>&    material,
        const glm::mat4&                    transform = glm::mat4(1.0f));

  private:
    size_t                          m_triangle_count; // number of triangles
    size_t                          m_vertices_count; // number of vertices
    std::vector<size_t>             m_indices; // each triangle vertex indices
    std::unique_ptr<glm::vec3[]>    m_vertices;
    std::unique_ptr<glm::vec3[]>    m_normals;
    std::shared_ptr<Material>       m_mat;
};

// A triangle linked directly to 1 triangle mesh.
class Triangle : public VisualObject
{
  public:
    Triangle(
        const std::shared_ptr<TriangleMesh>&    mesh,
        const size_t                            indice); // indice of the triangle first vertex in the mesh

    // Möller-Trumbore algorithm.
    bool hit(
        const Ray&                          r,
        const float                         tmin,
        const float                         tmax,
        HitRecord&                          rec) const override;

    const AABB& bbox() const override;

  private:
    std::shared_ptr<TriangleMesh>       m_mesh;
    size_t*                             m_indices;
    const glm::vec3&                    m_normal;
    AABB                                m_bbox;
};


//
// Mesh generation.
//

void create_triangle_mesh(
    MeshGroup&                          world,
    const size_t                        triangle_count,
    const size_t                        vertices_count,
    const size_t*                       indices,
    const glm::vec3*                    vertices,
    const glm::vec3*                    normals,
    const std::shared_ptr<Material>&    material,
    const glm::mat4&                    transform);

void create_cube(
    MeshGroup&                          world,
    const std::shared_ptr<Material>&    material,
    const glm::mat4&                    transform = glm::mat4(1.0f));

void create_plane(
    MeshGroup&                          world,
    const std::shared_ptr<Material>&    material,
    const glm::mat4&                    transform = glm::mat4(1.0f));


#endif // VISUALOBJECT_H
