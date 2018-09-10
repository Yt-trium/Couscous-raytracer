#ifndef RENDER_H
#define RENDER_H

#include "ray.h"

#include <glm/glm.hpp>

class Render
{
public:
    Render();

    vec3 getRayColor(const Ray& r);
    float rayHitSphere(const vec3& center, float radius, const Ray& r);
};

#endif // RENDER_H
