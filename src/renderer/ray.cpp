// Interface.
#include "ray.h"

using namespace glm;

Ray::Ray(const vec3& origin, const vec3& direction)
  : origin(origin)
  , dir(direction)
{
}

vec3 Ray::point(const float t) const
{
    return origin + t * dir;
}

