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
class Triangle;

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
    const Triangle*   triangle;
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

    virtual const std::shared_ptr<Material>& mat() const = 0;
};

class Triangle;

// A list of 3D intersectable objects.
typedef std::shared_ptr<Triangle> Shape;
typedef std::vector<Shape> MeshGroup;

// Test the intersection in the whole world.
bool hit_world(
    const MeshGroup&    world,
    const Ray&          r,
    const float         tmin,
    const float         tmax,
    HitRecord&          rec);

// Returns all light objects from the given world.
MeshGroup fetch_lights(const MeshGroup& world);


//
// Usual shapes.
//

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

    std::shared_ptr<Material> getMaterial();

    size_t getTriangleCount();

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

    const std::shared_ptr<Material>& mat() const override;

    void getVertices(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

    const glm::vec3& normal() const;

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

void create_cylinder(
    MeshGroup&                          world,
    const std::shared_ptr<Material>&    material,
    const size_t                        subidivisions = 6,
    const float                         height = 1.0f,
    const float                         width = 1.0f,
    const bool                          caps = true,
    const glm::mat4&                    transform = glm::mat4(1.0f));

#endif // VISUALOBJECT_H
