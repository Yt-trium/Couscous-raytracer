#ifndef RAY_H
#define RAY_H

// glm includes.
#include <glm/vec3.hpp>

// 3D line.
class Ray
{
  public:
    Ray();
    Ray(const glm::vec3& a, const glm::vec3& b);

    glm::vec3 origin() const;
    glm::vec3 direction() const;
    glm::vec3 pointAtParameter(float t) const;

  private:
    glm::vec3 A; // origin of the line
    glm::vec3 B; // direction of the line
};

#endif // RAY_H
