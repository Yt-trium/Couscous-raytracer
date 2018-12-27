
// Interface.
#include "utility.h"

// couscous includes.
#include "renderer/rng.h"

// Standard includes.
#include <cmath>
#include <cstddef>

using namespace std;
using namespace glm;

vec3 random_point_in_triangle(
    const vec3&     va,
    const vec3&     vb,
    const vec3&     vc,
    RNG&            rng)
{
    const float r1 = rng.next();
    const float r2 = rng.next();
    const float m1 = 1.0f - sqrt(r1);
    const float m2 = sqrt(r1) * (1.0f - r2);
    const float m3 = r2 * sqrt(r1);

    return m1*va + m2*vb + m3*vc;
}

vec3 random_in_unit_sphere(RNG& rng)
{
    vec3 p;

    do
    {
        p = 2.0f * vec3(
            rng.next(),
            rng.next(),
            rng.next()) - vec3(1.0f);
    } while(dot(p, p) >= 1.0f);

    return p;
}

vec3 random_point_on_lights(const MeshGroup& lights, RNG& rng)
{
    const size_t indice = static_cast<size_t>(rng.next() * (lights.size() - 1));

    const shared_ptr<Triangle>& light = lights[indice];
    const vec3& va = light->vertice(0);
    const vec3& vb = light->vertice(1);
    const vec3& vc = light->vertice(2);

    return random_point_in_triangle(va, vb, vc, rng);
}

vec3 random_in_cone(const vec3& direction, const float roughness, RNG& rng)
{
    return direction + roughness * random_in_unit_sphere(rng);
}

