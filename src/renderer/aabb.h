#ifndef RENDER_AABB_H
#define RENDER_AABB_H

// glm includes.
#include <glm/glm.hpp>

// Standard includes.
#include <limits>

// Forward declarations.
class Ray;

// Axis-aligned bounding box.
class AABB
{
  public:
    glm::vec3 min, max;

    AABB();

    AABB(const glm::vec3& min, const glm::vec3& max);

    AABB(const glm::vec3& point);

    bool is_valid() const;

    // Detects if the given Ray intersects the bbox.
    // tmin and tmax are the start and the end of the ray if any.
    // if t_in and t_out are set, they are used to output
    // the parameters of the entry and exit point of the intersection
    // points.
    bool intersect(
        const Ray&  ray,
        const float tmin = 0.0f,
        const float tmax = std::numeric_limits<float>::max(),
        float*      t_in = nullptr,
        float*      t_out = nullptr) const;

    // Test if a point is inside the bbox.
    bool contains(const glm::vec3& point) const;

    // Enlarge the bbox so that it contains the given point.
    void add_point(const glm::vec3& point);

    // Returns the axis index
    // of the maximum extent.
    size_t max_extent() const;
};


//
// Artihmetic.
//

AABB operator+(const AABB& lhs, const AABB& rhs);
AABB& operator+=(AABB& lhs, const AABB& rhs);

#endif // RENDER_AABB_H
