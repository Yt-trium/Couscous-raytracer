#ifndef RAY_H
#define RAY_H

// glm includes.
#include <glm/vec3.hpp>

// 3D line.
class Ray
{
  public:
    Ray(
        const glm::vec3& origin = glm::vec3(),
        const glm::vec3& direction = glm::vec3());

    glm::vec3 point(const float t) const;

    glm::vec3 origin;
    glm::vec3 dir;
};

#endif // RAY_H
