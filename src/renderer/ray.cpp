#include "ray.h"

using namespace glm;

Ray::Ray()
{
    A = vec3();
    B = vec3();
}

Ray::Ray(const vec3& a, const vec3& b)
{
    A = a;
    B = b;
}

vec3 Ray::origin() const
{
    return A;
}

vec3 Ray::direction() const
{
    return B;
}

vec3 Ray::pointAtParameter(float t) const
{
    return A + t*B;
}
