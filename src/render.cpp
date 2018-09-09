#include "render.h"

Render::Render()
{

}

glm::vec3 Render::getRayColor(const Ray &r)
{
    if(rayHitSphere(vec3(0,0,-1), 0.5, r))
        return vec3(1.0, 0.0, 0.0);

    vec3 unit_direction = (r.direction()) / float(r.direction().length());
    float t = 0.5 * (unit_direction.y + 1.0);
    return  float(1.0-t) * vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
}

bool Render::rayHitSphere(const glm::vec3 &center, float radius, const Ray &r)
{
    vec3 oc = r.origin() - center;
    float a = glm::dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float d = b*b - 4*a*c;
    return (d > 0);
}
