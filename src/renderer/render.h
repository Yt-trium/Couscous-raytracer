#ifndef RENDER_H
#define RENDER_H

// couscous includes.
#include "renderer/ray.h"
#include "renderer/visualobject.h"

// Math includes.
#include <glm/vec3.hpp>

class Render
{
  public:
    Render();

    glm::vec3 getRayColor(
        const Ray&          r,
        VisualObjectList    world);

    float rayHitSphere(
        const glm::vec3&    center,
        float               radius,
        const Ray&          r);
};

#endif // RENDER_H
