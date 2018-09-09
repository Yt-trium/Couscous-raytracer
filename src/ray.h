#ifndef RAY_H
#define RAY_H

#include<glm/vec3.hpp>

using glm::vec3;

class Ray
{
public:
    Ray();
    Ray(const vec3& a, const vec3& b);

    vec3 origin() const;
    vec3 direction() const;
    vec3 pointAtParameter(float t) const;

private:
    vec3 A;
    vec3 B;

};

#endif // RAY_H
