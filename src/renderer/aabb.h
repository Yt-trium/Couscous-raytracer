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

    AABB(const glm::vec3& min, const glm::vec3& max);

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
};

#endif // RENDER_AABB_H
