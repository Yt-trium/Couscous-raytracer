#ifndef RENDERER_UTILITY_H
#define RENDERER_UTILITY_H

// couscous includes.
#include "renderer/visualobject.h"

// GLM includes.
#include <glm/glm.hpp>

// Forward declarations.
class RNG;

glm::vec3 random_point_in_triangle(
    const glm::vec3&    va,
    const glm::vec3&    vb,
    const glm::vec3&    vc,
    RNG&                rng);

glm::vec3 random_in_unit_sphere(RNG& rng);

glm::vec3 random_point_on_lights(const MeshGroup& lights, RNG& rng);

#endif // RENDERER_UTILITY_H
