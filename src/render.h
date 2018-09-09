#ifndef RENDER_H
#define RENDER_H

#include "ray.h"

#include <glm/glm.hpp>

using glm::vec3;

class Render
{
public:
    Render();

    vec3 getRayColor(const Ray& r);
    bool rayHitSphere(const vec3& center, float radius, const Ray& r);
};

#endif // RENDER_H
